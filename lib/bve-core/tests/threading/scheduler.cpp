#include <core/threading/scheduler.hpp>
#include <doctest.h>

TEST_SUITE_BEGIN("libcore - threading");

using bve::core::threading::TaskScheduler;

TEST_CASE("libcore - threading - scheduler - sum") {
	TaskScheduler ts;

	auto dependent = ts.prepareDependentTask([](TaskScheduler& ts) {
		ts.stop(); // Separate for debugging
	});

	constexpr std::size_t count = 100;

	std::array<std::size_t, count> partials;

	for (std::size_t i = 0; i < count; ++i) {
		ts.addTask(
		    [i, &partials](TaskScheduler&) {
			    partials[i] = i; // Separate for debugging
		    },
		    dependent);
	}

	ts.addDependentTask(std::move(dependent));

	ts.run(2, 0);

	for (std::size_t i = 0; i < count; ++i) {
		CHECK_EQ(partials[i], i);
	}
}

TEST_CASE("libcore - threading - scheduler - nested sum") {
	TaskScheduler ts;

	constexpr std::size_t count = 100;

	std::atomic<std::size_t> counter = 0;
	std::array<std::size_t, count * 2> partials;

	// First task increments does 0->99
	// Inner task does 100->199
	// When counter increment returns 99 all tasks are done.
	for (std::size_t i = 0; i < count; ++i) {
		ts.addTask([i, count, &partials, &counter](TaskScheduler& ts) {
			partials[i] = i; // Separate for debugging
			ts.addTask([i, count, &partials, &counter](TaskScheduler& ts) {
				partials[i + count] = i + count;
				if (counter.fetch_add(1, std::memory_order_acq_rel) == count - 1) {
					ts.stop();
				}
			});
		});
	}

	ts.run(2, 0);

	for (std::size_t i = 0; i < count * 2; ++i) {
		CHECK_EQ(partials[i], i);
	}
}

TEST_CASE("libcore - threading - scheduler - double dependants") {
	TaskScheduler ts;

	std::size_t counter = 0;

	auto dep2 = ts.prepareDependentTask([&counter](TaskScheduler& ts) {
		counter *= 2;
		ts.stop();
	});
	auto dep1 = ts.prepareDependentTask(
	    [&counter](auto&) {
		    counter = 4; // for debugging
	    },
	    dep2);
	ts.addTask([&counter](auto&) { counter = 1; }, dep1);

	ts.addDependentTask(std::move(dep1));
	ts.addDependentTask(std::move(dep2));

	ts.run(2, 0);

	CHECK_EQ(counter, 8);
}

TEST_SUITE_END();
