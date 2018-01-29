//
// Created by egor9814 on 26.12.17.
//

#include <iostream>
#include <lib_async.hpp>
#include <utility>

void r1(int arg){
	std::cout << arg << ": hello from " << threads::currentThread::getName() << std::endl;
}

class MyTask : public async::AsyncTask<std::string>{
	threads::concurrent::CountDownLatch* signal;

public:
	MyTask(threads::concurrent::CountDownLatch& doneSignal)
			: async::AsyncTask<std::string>(), signal(&doneSignal){}

	MyTask()
			: async::AsyncTask<std::string>(), signal(nullptr){}

protected:
	void onPreExecute() override {
		std::cout << "run in " << threads::currentThread::getName() << std::endl;
	}

	void doInBackground(Param &param) override {
		std::cout << "working in " << threads::currentThread::getName() << std::endl;
		auto result = std::stoi(param, nullptr, 16);
		if(result % 2)
			cancel();
		std::cout << "Result in " << threads::currentThread::getName() << ": " << result << std::endl;
		if(signal != nullptr)
			signal->countDown();
	}

	void onCancelled() override {
		std::cout << "cancelled" << std::endl;
	}
};

static int global = 0;

void plus(threads::concurrent::Mutex* mutex){
	synchronized(*mutex, {
		auto local = global;
		std::cout << threads::currentThread::getName() << ": plus " << global << std::endl;
		local++;
		global = local;
	})
}

void minus(threads::concurrent::Mutex* mutex){
	synchronized(*mutex, {
		auto local = global;
		std::cout << threads::currentThread::getName() << ": minus " << global << std::endl;
		local--;
		global = local;
	})
}


int main(){
	using namespace threads;
	using namespace concurrent;
	using namespace util;

	/*Thread t(r1);
	r1();
	t.setName("My Thread");
	t.start();
	t.join();*/

	//util::concurrent::CountDownLatch done(1);
	/*MyTask task;
	Awaitable<MyTask> awaitable(task);
	//awaitable.release();

	std::string arg("a1");
	task.execute("a3");*/

	//done.await();

	/*async::execute(r1, 1);
	Thread t(r1, 2);
	Awaitable<Thread> awaitable(t);
	t.start();*/

	const unsigned long count = 100;
	AThread threads[count];
	unsigned long i;
	Mutex mutex;
	/*for(i = 0; i < count; i++){
		auto j = i / 4;
		threads[i] = Thread(
				(j == 1 || j == 2) ? minus : plus,
				&mutex);
		threads[i].start();
	}*/
	for(i = 0; i < count/2; i++){
		threads[i] = new Thread(minus, &mutex);
		threads[i]->start();
	}
	for(; i < count; i++){
		threads[i] = new Thread(plus, &mutex);
		threads[i]->start();
	}

	MyTask task;
	task.execute("fe");
	async::execute(r1, 23);

	for(i = 0; i < count; i++){
		threads[i]->await();
	}
	task.await();

	std::cout << threads::currentThread::getName() << ": global " << global << std::endl;


	CountDownLatch done(10);
	Thread t([&](){
		while(done.getCount() > 0){
			std::cout << threads::currentThread::getName() << ": " << (done.getCount() - 1) << std::endl;
			threads::currentThread::sleep(500);
			done.countDown();
		}
	});
	t.start();
	done.await();

	return 0;
}