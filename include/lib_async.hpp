//
// Created by egor9814 on 01.01.18.
//

#ifndef __egor9814__lib_threads_async_hpp__
#define __egor9814__lib_threads_async_hpp__

#include <lib_threads.hpp>
#include <string>
#include <vector>

/** Main Async namespace */
namespace async {

	/** Internal functions for Async */
	namespace __async_internal__ {

		threads::Thread* newThread(threads::util::Runnable*);
		//std::string newAsyncTaskName();

	}

	/** Async task
	 * onPreExecute() - calling in current thread
	 * onCancelled() - calling in new thread, if task is cancelled
	 * doInBackground(Param&) - calling in new thread */
	template <typename ParamType>
	class AsyncTask {
	public:
		typedef ParamType Param;

	protected:
		/** Calling in current thread */
		virtual void onPreExecute(){}
		/** Calling in new thread, if task is cancelled */
		virtual void onCancelled(){}

		/** Calling in new thread */
		virtual void doInBackground(Param& param) = 0;

	private:
		enum Status {
			Pending, Running, Finished
		};
		Status status;
		bool cancelled;

		void onPost(){
			status = Finished;
		}

		void onCancel(){
			status = Finished;
			cancelled = true;
			onCancelled();
		}


		//typedef AsyncTask<Result, Param...>* Instance;
		threads::Thread* thread;

		/** Special Runnable for thread */
		class Worker : public threads::util::Runnable {
			struct CancelSignal {};

			AsyncTask* instance;
			ParamType& param;
			//threads::Thread* capturedThread;

		public:
			Worker(AsyncTask* instance, Param& param)
					: instance(instance), param(param){}

			~Worker() {
				instance = nullptr;
				//capturedThread = nullptr;
			}

			void run() override {
				//capturedThread = threads::currentThread::get();
				try {
					instance->doInBackground(param);
					instance->onPost();
				} catch (CancelSignal&){
					instance->onCancel();
				}
			}

			void cancel(){
				/*auto capturedThread = instance->thread;
				if(capturedThread != nullptr && capturedThread->isAlive()){
					capturedThread->interrupt();
				}*/
				throw CancelSignal{};
			}
		};
		Worker* worker;


	public:
		AsyncTask()
				: status(Pending), cancelled(false), thread(nullptr){}

		~AsyncTask(){
			if(thread != nullptr){
				delete thread;
				thread = nullptr;
			}
			if(worker != nullptr){
				delete worker;
				thread = nullptr;
			}
		}

		AsyncTask(const AsyncTask&) = delete;
		AsyncTask&operator=(const AsyncTask&) = delete;


		bool isPending(){
			return status == Pending;
		}

		bool isRunning(){
			return status == Running;
		}

		bool isFinished(){
			return status == Finished;
		}

		bool isCancelled(){
			return cancelled;
		}


		void cancel(){
			if(isRunning()){
				worker->cancel();
			}
		}

		/** Run async task */
		void execute(Param && param){
			if(isPending()){
				worker = new Worker(this, param);
				thread = __async_internal__::newThread(worker);
				status = Running;
				onPreExecute();
				thread->start();
			}
		}

		void await(){
			thread->join();
		}
	};


	/** Call function in new thread and wait for finishing */
	template <typename _Callable, typename ... _Args>
	threads::AThread execute(_Callable&& function, _Args&&... args){
		auto t = new threads::Thread(
				static_cast<_Callable&&>(function),
				static_cast<_Args&&>(args)...
		);
		t->start();
		return threads::AThread(t);
	};

}

#endif //__egor9814__lib_threads_async_hpp__
