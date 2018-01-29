//
// Created by egor9814 on 01.01.18.
//

#include <string>
#include <cstring>
#include <lib_threads.hpp>
#include "public.hpp"

threads::Thread::ID::ID() : ID((std::string("Thread-") +
			std::to_string(threads::__threads_internal__::next_thread_name(_public_::get_key()))),
			0){}

threads::Thread::ID::ID(std::string&& _name, unsigned long _id) {
	/*if(_name == nullptr){
		this->_name = nullptr;
	} else {
		this->_name = new char[strlen(_name)+1];
		strcpy((char*)this->_name, _name);
	}*/
	this->_id = _id;
	setName(static_cast<std::string &&>(_name));
}

void threads::Thread::ID::setName(std::string &&name) {
	_name = std::string(name);
	if(!_name.empty() && _id != 0){
		pthread_setname_np(_id, this->_name.c_str());
	}
}

threads::Thread::ID::~ID() {
	/*if(_name != nullptr){
		delete [] _name;
		_name = nullptr;
	}*/
	_id = 0;
}

threads::Thread::ID::ID(ID && _id_) noexcept {
	_id = _id_._id;
	_name = std::move(_id_._name);

	_id_._id = 0;
	//_id_._name = nullptr;
}

threads::Thread::ID& threads::Thread::ID::operator=(ID && _id_) noexcept {
	if(this != &_id_){
		_id = _id_._id;
		/*if(_name != nullptr){
			delete [] _name;
		}*/
		_name = std::move(_id_._name);

		_id_._id = 0;
		_id_._name = nullptr;
	}
	return *this;
}


void* threads::Thread::startImpl(void *t) {
	auto thread = (Thread*)t;
	while (thread->id._id == 0);
	if(!thread->id._name.empty())
		pthread_setname_np(thread->id._id, thread->id._name.c_str());
	//__threads_internal__::add_thread(_public_::get_key(), thread);
	thread->run();
	//__threads_internal__::remove_thread(_public_::get_key(), thread);
	thread->id._id = 0;
	return nullptr;
}

void threads::Thread::run() {
	if(target){
		target->run();
	}
}

bool threads::Thread::start() {
	if(isAlive())
		return false;
	is_interrupted = false;
	unsigned long t_id = 0;
	if(pthread_create(&t_id, nullptr, Thread::startImpl, this)){
		return false;
	}
	id._id = t_id;
	return true;
}

bool threads::Thread::interrupt() {
	if(!is_interrupted){
		try {
			detach();
		} catch (...){}
		is_interrupted = true;
		bool res = pthread_cancel(id._id) != 0;
		//__threads_internal__::remove_thread(_public_::get_key(), this);
		id._id = 0;
		return res;
	}
	return false;
}

bool threads::Thread::isAlive() {
	return id._id != 0;
}

bool threads::Thread::isInterrupted() {
	return is_interrupted;
}

bool threads::Thread::isJoinable() {
	return isAlive();
}

bool threads::Thread::join() {
	return pthread_join(id._id, nullptr) != 0;
}

bool threads::Thread::detach() {
	return pthread_detach(id._id) != 0;
}

void threads::Thread::setName(std::string&& name) {
	id.setName(static_cast<std::string &&>(name));
}

std::string threads::Thread::getName() const {
	return id._name;
}

unsigned long threads::Thread::getId() const {
	return id._id;
}

void threads::Thread::swap(Thread &t) noexcept {
	std::swap(id, t.id);
	std::swap(target, t.target);
	std::swap(is_interrupted, t.is_interrupted);
}


/*
void threads::waitAllThreads() {
	threads::__threads_internal__::wait(_public_::get_key());
}*/
