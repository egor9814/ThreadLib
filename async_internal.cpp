//
// Created by egor9814 on 01.01.18.
//

#include <lib_async.hpp>

struct AsyncInfo {
	unsigned long currentTaskName = 0;
};
static AsyncInfo asyncInfo;

struct Callable {
	threads::util::Runnable* runnable;

	void operator()(){
		runnable->run();
	}
};

std::string newAsyncTaskName() {
	return "AsyncTask #" + std::to_string(++asyncInfo.currentTaskName);
}

threads::Thread* async::__async_internal__::newThread(threads::util::Runnable * r) {
	return new threads::Thread(
			newAsyncTaskName(),
			Callable{r}
	);
}
