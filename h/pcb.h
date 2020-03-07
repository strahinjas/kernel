#ifndef _PCB_H_
#define _PCB_H_

#include "kernel.h"

const StackSize maxStackSize = 65536;

// kernel's thread implementation

class PCB {
public:
	PCB();
	PCB(void (*body)(), StackSize stackSize, Time timeSlice);
	PCB(Thread* thread, StackSize stackSize, Time timeSlice);

	~PCB() { delete[] stack; }

	static volatile PCB* running;

	void start();
	void waitToComplete();

	static void sleep(Time timeToSleep);

	void exitThread();

	volatile unsigned sp;
	volatile unsigned ss;
	volatile unsigned bp;
	volatile unsigned lck;

	enum State { IDLE, READY, ASLEEP, BLOCKED, FINISHED };

	State state;
	unsigned int quantum;
	volatile unsigned int sleepTime;
private:
	unsigned* stack;
	Queue<PCB> waiting;
};

// kernel's semaphore implementation

class KernelSem {
public:
	KernelSem(int init) : value(init) {}
	~KernelSem();

	int val() const { return value; }

	int wait(int toBlock);
	void signal();
private:
	int value;
	Queue<PCB> blocked;
};

// kernel's event implementation

class KernelEv {
public:
	KernelEv(PCB* pcb, IVTNo ivtNo);
	~KernelEv();

	void wait();
	void signal();
private:
	PCB* owner;
	IVTNo entry;
	volatile int value;
};

#endif