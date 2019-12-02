#include "pcb.h"

volatile PCB* PCB::running = 0;

// for mainThread only
PCB::PCB() {
	state = READY; stack = 0;
	sleepTime = lck = 0;
	quantum = defaultTimeSlice;
}

// for idleThread and kernelThread only
PCB::PCB(void (*body)(), StackSize stackSize, Time timeSlice) {
	state = IDLE;
	sleepTime = lck = 0;
	quantum = timeSlice;
	stack = new unsigned[stackSize /= sizeof(unsigned)];
#ifndef BCC_BLOCK_IGNORE
	stack[stackSize - 1] = 0x200;
	stack[stackSize - 2] = FP_SEG(body);
	stack[stackSize - 3] = FP_OFF(body);
	sp = FP_OFF(stack + stackSize - 12);
	ss = FP_SEG(stack + stackSize - 12);
	bp = FP_OFF(stack + stackSize - 12);
#endif
}

PCB::PCB(Thread* thread, StackSize stackSize, Time timeSlice) {
	state = IDLE;
	sleepTime = lck = 0;
	quantum = timeSlice;
	if (stackSize > maxStackSize) stackSize = maxStackSize;
	stack = new unsigned[stackSize /= sizeof(unsigned)];
#ifndef BCC_BLOCK_IGNORE
	stack[stackSize - 1] = FP_SEG(thread);
	stack[stackSize - 2] = FP_OFF(thread);
	stack[stackSize - 5] = 0x200;
	stack[stackSize - 6] = FP_SEG(Thread::wrapper);
	stack[stackSize - 7] = FP_OFF(Thread::wrapper);
	sp = FP_OFF(stack + stackSize - 16);
	ss = FP_SEG(stack + stackSize - 16);
	bp = FP_OFF(stack + stackSize - 16);
#endif
}

void PCB::start() {
	state = READY;
	Scheduler::put(this);
}

void PCB::waitToComplete() {
	if (state != FINISHED) {
		PCB::running->state = BLOCKED;
		waiting.insert((PCB*)PCB::running);
	}
}

void PCB::sleep(Time timeToSleep) {
	if (timeToSleep > 0) {
		PCB::running->state = ASLEEP;
		PCB::running->sleepTime = timeToSleep;
	}
}

void PCB::exitThread() {
	while (!waiting.empty()) {
		PCB* pcb = waiting.remove();
		pcb->state = READY;
		Scheduler::put(pcb);
	}
	state = FINISHED;
}