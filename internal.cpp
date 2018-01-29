//
// Created by egor9814 on 01.01.18.
//

#include "public.hpp"
#include <lib_threads.hpp>
#include <map>

using _public_::get_key;

inline void check(unsigned long key){
	if(get_key() != key)
		throw "using private threads functions";
}

struct ThreadsInfo {
	unsigned long currentThreadName = 0;
	unsigned long mainThread = threads::__threads_internal__::get_thread_id(get_key());
	//std::map<unsigned long, threads::Thread*> threads;
};
static ThreadsInfo threadsInfo;

/*void threads::__threads_internal__::add_thread(unsigned long key, Thread * t) {
	check(key);
	if(t != nullptr && t->getId() != 0)
		threadsInfo.threads[t->getId()] = t;
}

void threads::__threads_internal__::remove_thread(unsigned long key, Thread * t) {
	check(key);
	if(t != nullptr && threadsInfo.threads.count(t->getId()) > 0){
		threadsInfo.threads.erase(t->getId());
	}
}

threads::Thread* threads::__threads_internal__::find_thread(unsigned long key, unsigned long id) {
	check(key);
	if(threadsInfo.threads.count(id)){
		return threadsInfo.threads[id];
	}
	return nullptr;
}*/

unsigned long threads::__threads_internal__::next_thread_name(unsigned long key) {
	check(key);
	return ++threadsInfo.currentThreadName;
}

unsigned long threads::__threads_internal__::get_thread_id(unsigned long key) {
	check(key);
	return pthread_self();
}

unsigned long threads::__threads_internal__::get_main_thread_id(unsigned long key) {
	check(key);
	return threadsInfo.mainThread;
}


/*void threads::__threads_internal__::wait(unsigned long key) {
	check(key);
	while (!threadsInfo.threads.empty());
}*/
