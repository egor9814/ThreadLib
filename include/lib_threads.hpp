//
// Created by egor9814 on 01.01.18.
//

#ifndef __egor9814__lib_threads_hpp__
#define __egor9814__lib_threads_hpp__

#include <functional>
#include <list>

/** Main ThreadLib namespace */
namespace threads {

	/** Different functions and classes */
	namespace util {

		/** Get current system time in nanoseconds */
		unsigned long currentTimeNanos();
		/** Get current system time in milliseconds */
		unsigned long currentTimeMillis();


		/** Runnable */
		class Runnable {
		public:
			virtual void run() = 0;

			void operator()(){
				run();
			}

		};


		/** Auto-remove pointer */
		template <typename T>
		class Pointer {
			T* ptr;

		public:
			explicit Pointer(T* pointer = nullptr) : ptr(pointer){}
			Pointer(Pointer&& pointer) noexcept {
				ptr = pointer.ptr;
				pointer.ptr = nullptr;
			}

			~Pointer(){
				free();
			}

			void free(){
				if(ptr != nullptr){
					delete ptr;
					ptr = nullptr;
				}
			}

			explicit operator bool(){
				return ptr != nullptr;
			}

			T* get(){
				return ptr;
			}

			T*operator->(){
				return ptr;
			}

			T&operator*(){
				return *ptr;
			}


			Pointer&operator=(T* pointer){
				free();
				ptr = pointer;
				return *this;
			}

			Pointer&operator=(Pointer&& pointer) noexcept {
				if(this != &pointer){
					free();
					ptr = pointer.ptr;
					pointer.ptr = nullptr;
				}
				return *this;
			}

			Pointer&operator=(const Pointer& pointer) {
				if(this != &pointer){
					free();
					ptr = pointer.ptr;
				}
				return *this;
			}
		};


		/** Awaitable (Example, for hold thread) */
		template <typename _Awaitable>
		class Awaitable {
			_Awaitable& awaitable;
			bool await = true;

		public:
			explicit Awaitable(_Awaitable& awaitable)
					: awaitable(awaitable){}

			~Awaitable(){
				if(await){
					awaitable.await();
				}
			}

			_Awaitable& get(){
				return awaitable;
			}

			_Awaitable*operator->(){
				return &awaitable;
			}

			void release(){
				await = false;
			}
		};

		/** AwaitablePointer (Example, for hold thread) */
		template <typename _Awaitable>
		class AwaitablePointer {
			_Awaitable* awaitable;
			bool await = true;
			bool remove;

		public:
			explicit AwaitablePointer(_Awaitable* awaitable = nullptr, bool removeAfterWait = true)
					: awaitable(awaitable), remove(removeAfterWait){}

			AwaitablePointer(AwaitablePointer&) = delete;

			AwaitablePointer(AwaitablePointer&& pointer) noexcept {
				awaitable = pointer.awaitable;
				await = pointer.await;
				remove = pointer.remove;

				pointer.awaitable = nullptr;
				pointer.await = false;
				pointer.remove = false;
			}

			~AwaitablePointer(){
				if(awaitable != nullptr){
					if(await){
						awaitable->await();
					}
					if(remove){
						delete awaitable;
						awaitable = nullptr;
					}
				}
			}

			_Awaitable* get(){
				return awaitable;
			}

			_Awaitable*operator->(){
				return awaitable;
			}

			void release(){
				await = false;
			}

			AwaitablePointer&operator=(AwaitablePointer&) = delete;

			AwaitablePointer&operator=(AwaitablePointer&& pointer) noexcept {
				if(this != &pointer){
					if(awaitable != nullptr && remove){
						delete awaitable;
					}
					awaitable = pointer.awaitable;
					await = pointer.await;
					remove = pointer.remove;

					pointer.awaitable = nullptr;
					pointer.await = false;
					pointer.remove = false;
				}
				return *this;
			}

			AwaitablePointer&operator=(_Awaitable* awaitable){
				if(this->awaitable != nullptr && remove){
					delete this->awaitable;
				}
				this->awaitable = awaitable;
				return *this;
			}
		};
	}

	/** Concurrency utils */
	namespace concurrent {

		/** Mutex */
		class Mutex {
			void* lockable;

		public:
			Mutex();
			~Mutex();

			void lock();
			void unlock();
		};


		/** Count Down Latch */
		class CountDownLatch {
			class Sync {
				unsigned long state;
				threads::concurrent::Mutex lock;

			public:
				explicit Sync(unsigned long);
				unsigned long getState() const;
				bool releaseShared();
				void acquireShared();
				bool acquireShared(unsigned long timeoutMillis);
			};

			Sync sync;

		public:
			explicit CountDownLatch(unsigned long count = 0);

			void countDown();

			void await();

			unsigned long getCount() const ;

			bool await(unsigned long timeoutMillis);
		};

	}


	/** Thread */
	class Thread;

	/** Internal functions for threads */
	namespace __threads_internal__ {

		/*void add_thread(unsigned long, Thread*);
		void remove_thread(unsigned long, Thread*);
		Thread* find_thread(unsigned long, unsigned long);*/

		unsigned long next_thread_name(unsigned long);

		unsigned long get_thread_id(unsigned long);
		unsigned long get_main_thread_id(unsigned long);

		//void wait(unsigned long);
	}

	class Thread {

		/** Class for hold name and id of thread */
		class ID {
			friend class Thread;

			std::string _name;
			unsigned long _id;

		public:
			ID();

			ID(std::string&& _name, unsigned long _id);
			~ID();

			ID(ID&&) noexcept ;
			ID&operator=(ID&&) noexcept ;

			void setName(std::string&& name);
		};


		/** Function binder to Runnable */
		template <typename Func, typename ... Args>
		class BindFunc : public util::Runnable {
			Func func;
			std::tuple<Args...> args;

		public:
			BindFunc(Func&& func, Args&&... args)
					: func(func), args(args...){}

			void run() override {
				func(std::get<Args>(args)...);
			}
		};


		ID id;

		typedef util::Pointer<util::Runnable> target_t;
		target_t target;
		static void* startImpl(void *);

		bool is_interrupted = false;

	public:
		Thread() noexcept
				: id(), target(){}

		explicit Thread(std::string&& name) noexcept
				: id(static_cast<std::string &&>(name), 0), target(){}

		Thread(Thread&) = delete;

		Thread(Thread&& t) noexcept {
			swap(t);
		}

		Thread&operator=(Thread&& t) noexcept {
			swap(t);
			return *this;
		}

		template <typename _Callable, typename ... _Args>
		explicit Thread(_Callable&& f, _Args&&... args)
				: id(){
			target = target_t(new
									  BindFunc<_Callable, _Args...>(
					static_cast<_Callable&&>(f),
					static_cast<_Args&&>(args)...
							  )
			);
		}

		template <typename _Callable, typename ... _Args>
		Thread(std::string&& name, _Callable&& f, _Args&&... args)
				: id(static_cast<std::string &&>(name), 0){
			target = target_t(new
									  BindFunc<_Callable, _Args...>(
					static_cast<_Callable&&>(f),
					static_cast<_Args&&>(args)...
							  )
			);
		}

		virtual ~Thread(){
			interrupt();
		}

		virtual void run();

		bool start();
		bool interrupt();

		bool isAlive();
		bool isInterrupted();

		bool isJoinable();
		bool join();

		bool detach();

		void setName(std::string&& name);
		std::string getName() const ;

		unsigned long getId() const ;

		void swap(Thread&) noexcept ;


		// wrap for "Awaitable"
		void await(){
			if(isJoinable())
				join();
		}

	};

	/** Special type for threads
	 * Use: AThread thread = new Thread(...);
	 * */
	typedef threads::util::AwaitablePointer<threads::Thread> AThread;


	/** Get current thread */
	namespace currentThread {
		std::string getName();
		unsigned long getId();

		bool isMainThread();

		void sleep(unsigned long millis);
		void sleep_nano(unsigned long nanos);

		void yield();

		void interrupt();
	}

}

#define synchronized(mutex, unit){\
    (mutex).lock();\
	unit;\
    (mutex).unlock();\
}

#endif //__egor9814__lib_threads_hpp__
