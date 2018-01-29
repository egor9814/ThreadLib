//
// Created by egor9814 on 01.01.18.
//

#include <lib_threads.hpp>

threads::concurrent::CountDownLatch::Sync::Sync(unsigned long i)
		: state(i), lock(){}

unsigned long threads::concurrent::CountDownLatch::Sync::getState() const {
	const_cast<Mutex&>(lock).lock();
	auto res = state;
	const_cast<Mutex&>(lock).unlock();
	return res;
}


template <typename T>
bool compareAndSwap(threads::concurrent::Mutex* mutex, T& what, T with, T replacement){
	mutex->lock();
	if(what == with){
		what = replacement;
		mutex->unlock();
		return true;
	}
	mutex->unlock();
	return false;
}

bool threads::concurrent::CountDownLatch::Sync::releaseShared() {
	for(;;){
		auto s = getState();
		if(s == 0)
			return false;
		auto next = s-1;
		if(compareAndSwap(&lock, state, s, next))
			return next == 0;
	}
}

void threads::concurrent::CountDownLatch::Sync::acquireShared() {
	while (state > 0);
}

bool threads::concurrent::CountDownLatch::Sync::acquireShared(unsigned long timeoutMillis) {
	using namespace threads::util;
	auto start = currentTimeMillis();
	auto current = currentTimeMillis();
	for(; state > 0 && current - start < timeoutMillis; current = currentTimeMillis());
	return state == 0 && current - start < timeoutMillis;
}


threads::concurrent::CountDownLatch::CountDownLatch(unsigned long count)
		: sync(count){}

void threads::concurrent::CountDownLatch::countDown() {
	sync.releaseShared();
}

unsigned long threads::concurrent::CountDownLatch::getCount() const {
	return sync.getState();
}

void threads::concurrent::CountDownLatch::await() {
	sync.acquireShared();
}

bool threads::concurrent::CountDownLatch::await(unsigned long timeoutMillis) {
	return sync.acquireShared(timeoutMillis);
}
