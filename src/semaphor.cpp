#include "pcb.h"
#include "semaphor.h"

KernelSem::~KernelSem() {
	while (!blocked.empty()) {
		PCB* pcb = blocked.remove();
		pcb->state = PCB::READY;
		Scheduler::put(pcb);
	}
}

int KernelSem::wait(int toBlock) {
	int result = 0;
	if (!toBlock && value <= 0) result = -1;
	else if (--value < 0) {
		result = 1;
		PCB::running->state = PCB::BLOCKED;
		blocked.insert((PCB*)PCB::running);
	}
	return result;
}

void KernelSem::signal() {
	if (value++ < 0) {
		PCB* pcb = blocked.remove();
		pcb->state = PCB::READY;
		Scheduler::put(pcb);
	}
}

Semaphore::Semaphore(int init) {
	lock();
	id = ++uniqueSemaphoreID;
	CallParams params;
	params.callID = 8;
	params.objectID = init;
	transfer(&params);
	unlock();
}

Semaphore::~Semaphore() {
	lock();
	CallParams params;
	params.callID = 9;
	params.objectID = id;
	transfer(&params);
	unlock();
}

int Semaphore::val() const {
	lock();
	CallParams params;
	params.callID = 10;
	params.objectID = id;
	transfer(&params);
	int result = params.objectID;
	unlock();
	return result;
}

int Semaphore::wait(int toBlock) {
	lock();
	CallParams params;
	params.callID = 11;
	params.time = id;
	params.objectID = toBlock;
	transfer(&params);
	int result = params.objectID;
	unlock();
	return result;
}

void Semaphore::signal() {
	lock();
	CallParams params;
	params.callID = 12;
	params.objectID = id;
	transfer(&params);
	unlock();
}