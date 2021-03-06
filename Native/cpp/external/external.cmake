set(VCPKG_INSTALLED_PATH "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
set(VCPKG_INSTALLED_PATH_DEBUG "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug")

# Thread
find_package(Threads REQUIRED)

# BVE
find_package(CLI11 CONFIG REQUIRED)
find_package(cppfs CONFIG REQUIRED)
find_package(doctest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(foundational CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(unofficial-abseil CONFIG REQUIRED)

# EASTL
find_library(EASTL_NAME EASTL PATHS ${VCPKG_INSTALLED_PATH} PATH_SUFFIXES lib NO_DEFAULT_PATH)
find_library(EASTL_DEBUG_NAME EASTL PATHS ${VCPKG_INSTALLED_PATH_DEBUG} PATH_SUFFIXES lib NO_DEFAULT_PATH)
add_library(eastl::lib INTERFACE IMPORTED)
set_property(TARGET eastl::lib
			 PROPERTY INTERFACE_LINK_LIBRARIES $<$<CONFIG:Debug>:${EASTL_DEBUG_NAME}> $<$<NOT:$<CONFIG:Debug>>:${EASTL_NAME}>
)
find_package(bvestl CONFIG REQUIRED)

set_property(TARGET glm APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS GLM_ENABLE_EXPERIMENTAL)

add_library(gsl::gsl INTERFACE IMPORTED)
set_property(TARGET gsl::gsl APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS GSL_THROW_ON_CONTRACT_VIOLATION)

find_package(SWIG)
if(SWIG_FOUND)
	message(STATUS "SWIG v${SWIG_VERSION} found at ${SWIG_EXECUTABLE}")
	cmake_policy(SET CMP0078 NEW)
	cmake_policy(SET CMP0086 NEW)
	include(UseSWIG)

	find_package(Python3 COMPONENTS Development)
	message(STATUS "Python3 Include: ${Python3_INCLUDE_DIRS}")
	message(STATUS "Python3 Library: ${Python3_LIBRARIES}")
	message(STATUS "Python3 Library: ${Python3_LIBRARY_DEBUG}")
	message(STATUS "Python3 Library: ${Python3_LIBRARY_RELEASE}")
else()
	message(STATUS "SWIG not found")
endif()

if(BVE_VTUNE)
	find_package(VTune)
endif()
