cmake_minimum_required(VERSION 3.9)
project(ThreadLib)

macro(use_pthreads module)
	find_package(Threads REQUIRED)
	target_link_libraries(${module} ${CMAKE_THREAD_LIBS_INIT})
	message("pthreads linked to ${module}")
endmacro()

set(CMAKE_CXX_STANDARD 17)

add_executable(ThreadLibTest main.cpp)
add_executable(ArrayBench array_bench.cpp)

include_directories(include)
add_library(threads SHARED
		include/lib_threads.hpp
		util.cpp
		thread.cpp
		public.cpp
		internal.cpp
		current.cpp
		count_down_latch.cpp
		mutex.cpp

		include/lib_async.hpp
		async_internal.cpp
		)
use_pthreads(threads)

target_link_libraries(ThreadLibTest threads)
target_link_libraries(ArrayBench threads)