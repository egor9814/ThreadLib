//
// Created by egor9814 on 02.01.18.
//

#include <lib_threads.hpp>

struct MutexObject {
	pthread_mutex_t mutex;

	void lock(){
		pthread_mutex_lock(&mutex);
	}

	void unlock(){
		pthread_mutex_unlock(&mutex);
	}

	void init(){
		pthread_mutex_init(&mutex, nullptr);
	}

	void destroy(){
		pthread_mutex_destroy(&mutex);
	}
};

threads::concurrent::Mutex::Mutex() {
	lockable = (void*)new MutexObject;
	((MutexObject*)lockable)->init();
}

threads::concurrent::Mutex::~Mutex() {
	((MutexObject*)lockable)->destroy();
	delete (MutexObject*)lockable;
	lockable = nullptr;
}

void threads::concurrent::Mutex::lock() {
	((MutexObject*)lockable)->lock();
}

void threads::concurrent::Mutex::unlock() {
	((MutexObject*)lockable)->unlock();
}
