//
// Created by egor9814 on 01.01.18.
//

#include <lib_threads.hpp>
#include "public.hpp"

using _public_::get_key;

/*threads::Thread* threads::currentThread::get() {
	return __threads_internal__::find_thread(_public_::get_key(), __threads_internal__::get_thread_id());
}

const char* threads::currentThread::getName() {
	if(getId() == __threads_internal__::get_main_thread_id()){
		return "system";
	}
	auto thread = get();
	if(thread == nullptr){
		return "unknown";
	} else {
		return thread->getName();
	}
}*/

std::string threads::currentThread::getName() {
	const unsigned long max_len = 128;
	char buffer[max_len];
	auto id = getId();
	if(id != 0){
		if(id == __threads_internal__::get_main_thread_id(get_key()))
			return "main";
		else if(pthread_getname_np(id, buffer, max_len))
			return "unknown";
	}
	return std::string(buffer);
}

unsigned long threads::currentThread::getId() {
	return __threads_internal__::get_thread_id(get_key());
}

bool threads::currentThread::isMainThread() {
	return __threads_internal__::get_main_thread_id(get_key()) == getId();
}

static inline void sleep_impl(unsigned long time, unsigned long (*time_getter)()){
	auto start = time_getter();
	for(auto now = time_getter(); now - start < time; now = time_getter());
}

void threads::currentThread::sleep(unsigned long millis) {
	sleep_impl(millis, threads::util::currentTimeMillis);
}

void threads::currentThread::sleep_nano(unsigned long nanos) {
	sleep_impl(nanos, threads::util::currentTimeNanos);
}

void threads::currentThread::yield() {
	sched_yield();
}