#include "pcb.h"
#include "event.h"

KernelEv::KernelEv(PCB* pcb, IVTNo ivtNo) {
	if (!IVTEntry::myIVT[ivtNo]->event) {
		owner = pcb;
		value = 0;
		entry = ivtNo;
		IVTEntry::myIVT[ivtNo]->event = this;
	}
}

KernelEv::~KernelEv() {
	if (IVTEntry::myIVT[entry])
		IVTEntry::myIVT[entry]->event = 0;
}

void KernelEv::wait() {
	if (owner == PCB::running) {
		--value;
		owner->state = PCB::BLOCKED;
	}
}

void KernelEv::signal() {
	if (value++ < 0) {
		owner->state = PCB::READY;
		Scheduler::put(owner);
	}
	else value = 0;
}

Event::Event(IVTNo ivtNo) {
	lock();
	id = ++uniqueEventID;
	CallParams params;
	params.callID = 13;
	params.time = ivtNo;
	transfer(&params);
	unlock();
}

Event::~Event() {
	lock();
	CallParams params;
	params.callID = 14;
	params.objectID = id;
	transfer(&params);
	unlock();
}

void Event::wait() {
	lock();
	CallParams params;
	params.callID = 15;
	params.objectID = id;
	transfer(&params);
	unlock();
}

void Event::signal() {
	lock();
	CallParams params;
	params.callID = 16;
	params.objectID = id;
	transfer(&params);
	unlock();
}

IVTEntry* IVTEntry::myIVT[256];

IVTEntry::IVTEntry(pInterrupt routine, IVTNo ivtNo) : event(0), oldRoutine(0), newRoutine(routine), entry(ivtNo) {
	lock();
#ifndef BCC_BLOCK_IGNORE
	if (myIVT[ivtNo]) event = myIVT[ivtNo]->event;
	oldRoutine = getvect(ivtNo);
	setvect(ivtNo, newRoutine);
	myIVT[ivtNo] = this;
#endif
	unlock();
}

IVTEntry::~IVTEntry() {
	lock();
#ifndef BCC_BLOCK_IGNORE
	setvect(entry, oldRoutine);
	myIVT[entry] = 0;
#endif
	unlock();
}

void IVTEntry::signal(IVTNo ivtNo) {
	if (myIVT[ivtNo]->event)
		myIVT[ivtNo]->event->signal();
}

pInterrupt IVTEntry::oldOne(IVTNo ivtNo) {
	return myIVT[ivtNo]->oldRoutine;
}