//
// Created by egor9814 on 26.12.17.
//

#include <ctime>
#include <lib_threads.hpp>

unsigned long threads::util::currentTimeNanos() {
	return (unsigned long)(clock() * (clock_t)1000000000 / CLOCKS_PER_SEC);
}

unsigned long threads::util::currentTimeMillis() {
	return (unsigned long)(clock() * (clock_t)1000 / CLOCKS_PER_SEC);
}
