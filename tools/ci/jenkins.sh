#!/bin/bash

set -e -x

# Coverage
curl -so codecov.bash https://codecov.io/bash
echo "End Clone"

# Docs
./tools/docs/linux.sh build-docs
mkdir -p "/mnt/data/docs.bvereborn.com/$GIT_BRANCH/"
bash -c 'rsync -a --omit-dir-times --delete build-docs/html/* /mnt/data/docs.bvereborn.com/$GIT_BRANCH/'
echo "End Docs"

# Clang Build
mkdir build-clang
cd build-clang
cmake ..\
	-DCMAKE_BUILD_TYPE=Debug\
	-DSANITIZE_{ADDRESS,UNDEFINED}=On\
	-DCMAKE_C_COMPILER=/usr/lib/ccache/clang-8 -DCMAKE_CXX_COMPILER=/usr/lib/ccache/clang++-8\
	-DCMAKE_{EXE,SHARED}_LINKER_FLAGS="-fuse-ld=lld"\
	-DCMAKE_TOOLCHAIN_FILE=${VCPKG_CMAKE}\
	-DCMAKE_EXPORT_COMPILE_COMMANDS=On\
	-GNinja
ninja -j8
echo "End Clang-Build"
bin/tests
echo "End Clang-Tests"
cd ..

# GCC Build
mkdir build-gcc
cd build-gcc
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug\
    -DBVEREBORN_CODE_COVERAGE=On\
    -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc-7 -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++-7\
    -DCMAKE_{EXE_SHARED}_LINKER_FLAGS="-fuse-ld=gold"\
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_CMAKE}\
    -GNinja
ninja -j8
lcov -c -i -d . -o empty-coverage.info --gcov-tool gcov-7
echo "End GCC-Build"
bin/tests
echo "End GCC-Tests"
cd ..

# Formatting
cd build-clang
ninja format
cd ..
echo "End formatting"

if [ `git status --porcelain lib src | wc -c` -eq 0 ]; then
    echo "No formatting errors found."
else
    echo "Formatting errors found."
    git diff -numstat lib src
    exit 1
fi

cd build-gcc
lcov -c -d . -o live-coverage.info --gcov-tool gcov-7
lcov -a empty-coverage.info -a live-coverage.info -o coverage.info --gcov-tool gcov-7
lcov --remove coverage.info '/usr/*' '$WORKSPACE/extern/*' '*/tests/*' '*/tests_old/*' --output-file coverage.info
lcov --list coverage.info
bash ../codecov.bash -f coverage.info -t '2bfac9df-f504-4c34-bc53-3803bb9c9b27' || echo "Codecov did not collect coverage reports"
echo "End Coverage"
cd ..