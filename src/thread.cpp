#include "kernel.h"

Thread::Thread(StackSize stackSize, Time timeSlice) {
	lock();
	id = ++uniqueThreadID;
	CallParams params;
	params.callID = 1;
	params.thread = this;
	params.stackSize = stackSize;
	params.time = timeSlice;
	transfer(&params);
	unlock();
}

Thread::~Thread() {
	lock();
	waitToComplete();
	CallParams params;
	params.callID = 2;
	params.objectID = id;
	transfer(&params);
	unlock();
}

void Thread::start() {
	lock();
	CallParams params;
	params.callID = 3;
	params.objectID = id;
	transfer(&params);
	unlock();
}

void Thread::waitToComplete() {
	lock();
	CallParams params;
	params.callID = 4;
	params.objectID = id;
	transfer(&params);
	unlock();
}

void Thread::sleep(Time timeToSleep) {
	lock();
	CallParams params;
	params.callID = 5;
	params.time = timeToSleep;
	transfer(&params);
	unlock();
}

void Thread::wrapper(Thread* running) {
	running->run();

	// system call: exitThread

	lock();
	CallParams params;
	params.callID = 6;
	params.objectID = running->id;
	transfer(&params);
	unlock();
}

void dispatch() {
	lock();
	CallParams params;
	params.callID = 7;
	transfer(&params);
	unlock();
}