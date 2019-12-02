#include "pcb.h"

extern void tick();

pInterrupt oldRoutine;

ID uniqueThreadID = -1;
ID uniqueSemaphoreID = -1;
ID uniqueEventID = -1;

volatile unsigned tsp;
volatile unsigned tss;
volatile unsigned tbp;

Queue<PCB> sleeping;
Vector<PCB> threads;
Vector<KernelSem> semaphores;
Vector<KernelEv> events;

PCB *idle, *initial, *runningKernelThread;

volatile unsigned int lockFlag = 0;
volatile unsigned char kernelMode = 0;

volatile int timeLeft = defaultTimeSlice;

void idleRun() { while (1); }

void kernelRun() {
	while (1) {
		// retrieving system call parameters
		CallParams* params;
		unsigned segment, offs;
#ifndef BCC_BLOCK_IGNORE
		asm mov segment, cx
		asm mov offs, dx
		params = (CallParams*) MK_FP(segment, offs);
#endif
		switch (params->callID) {

		// threads' subroutines
		case 1:
			threads.push_back(new PCB(params->thread, params->stackSize, params->time));
			break;
		case 2:
			if (!threads[params->objectID]) break; // error: item not found!
			delete threads[params->objectID];
			break;
		case 3:
			if (!threads[params->objectID]) break; // error: item not found!
			threads[params->objectID]->start();
			break;
		case 4:
			if (!threads[params->objectID]) break; // error: item not found!
			threads[params->objectID]->waitToComplete();
			break;
		case 5:
			PCB::sleep(params->time);
			break;
		case 6:
			if (!threads[params->objectID]) break; // error: item not found!
			threads[params->objectID]->exitThread();
			break;
		case 7:
			break;

		// semaphores' subroutines
		case 8:
			semaphores.push_back(new KernelSem(params->objectID));
			break;
		case 9:
			if (!semaphores[params->objectID]) break; // error: item not found!
			delete semaphores[params->objectID];
			break;
		case 10:
			if (!semaphores[params->objectID]) break; // error: item not found!
			params->objectID = semaphores[params->objectID]->val();
			break;
		case 11:
			if (!semaphores[params->time]) break; // error: item not found!
			params->objectID = semaphores[params->time]->wait(params->objectID);
			break;
		case 12:
			if (!semaphores[params->objectID]) break; // error: item not found!
			semaphores[params->objectID]->signal();
			break;

		// events' subroutines
		case 13:
			events.push_back(new KernelEv((PCB*)PCB::running, params->time));
			break;
		case 14:
			if (!events[params->objectID]) break; // error: item not found!
			delete events[params->objectID];
			break;
		case 15:
			if (!events[params->objectID]) break; // error: item not found!
			events[params->objectID]->wait();
			break;
		case 16:
			if (!events[params->objectID]) break; // error: item not found!
			events[params->objectID]->signal();

		}
		sys_dispatch();
	}
}

void transfer(CallParams* params) {
#ifndef BCC_BLOCK_IGNORE
	unsigned segment = FP_SEG(params), offs = FP_OFF(params);
	asm {
		push cx
		push dx
		mov cx, segment
		mov dx, offs
		int 63h
		pop dx
		pop cx
	}
#endif
}

void lock() {
#ifndef BCC_BLOCK_IGNORE
	asm pushf
	asm cli
	++lockFlag;
	asm popf
#endif
}

void unlock() {
#ifndef BCC_BLOCK_IGNORE
	asm pushf
	asm cli
	--lockFlag;
	asm popf
#endif
}

// system initialization
void init() {
#ifndef BCC_BLOCK_IGNORE
	lock();
	PCB::running = initial = new PCB();
	idle = new PCB(idleRun, defaultStackSize, 1);
	runningKernelThread = new PCB(kernelRun, maxStackSize, 0);
	oldRoutine = getvect(0x8);
	setvect(0x8, timer);
	setvect(0x60, oldRoutine);
	setvect(0x63, sys_call);
	unlock();
#endif
}

// restoring system's previous state
void restore() {
#ifndef BCC_BLOCK_IGNORE
	lock();
	delete runningKernelThread;
	delete idle;
	delete initial;
	setvect(0x8, oldRoutine);
	unlock();
#endif
}

void interrupt timer(...) {
	if (--timeLeft < 0) timeLeft = 0;

	// awakening sleeping threads
	if (!sleeping.empty()) {
		Queue<PCB>::Element* current = sleeping.head;
		while (current) {
			if (--current->data->sleepTime == 0) {
				Queue<PCB>::Element* awakened = current;
				(current->prev ? current->prev->next : sleeping.head) = current->next;
				(current->next ? current->next->prev : sleeping.tail) = current->prev;
				current = current->next;
				awakened->data->state = PCB::READY;
				Scheduler::put(awakened->data);
				delete awakened;
			}
			else current = current->next;
		}
	}

	// calling system timer routine
	tick();
	asm int 60h;

	// context switch if allowed
	if (!timeLeft && !lockFlag && !kernelMode && PCB::running->quantum) dispatch();
}

void interrupt sys_call(...) {
	// saving userThread's context
#ifndef BCC_BLOCK_IGNORE
	asm {
		mov tsp, sp
		mov tss, ss
		mov tbp, bp
	}
#endif

	PCB::running->sp = tsp;
	PCB::running->ss = tss;
	PCB::running->bp = tbp;
	PCB::running->lck = lockFlag;

	// switching to kernel mode
	kernelMode = 1;

	// restoring kernelThread's context
	tsp = runningKernelThread->sp;
	tss = runningKernelThread->ss;
	tbp = runningKernelThread->bp;

	lockFlag = runningKernelThread->lck;
	timeLeft = runningKernelThread->quantum;

#ifndef BCC_BLOCK_IGNORE
	asm {
		mov sp, tsp
		mov ss, tss
		mov bp, tbp
		mov [bp + 10], dx
		mov [bp + 12], cx
	}
#endif
}

void interrupt sys_dispatch(...) {
	// saving kernelThread's context
#ifndef BCC_BLOCK_IGNORE
	asm {
		mov tsp, sp
		mov tss, ss
		mov tbp, bp
	}
#endif

	runningKernelThread->sp = tsp;
	runningKernelThread->ss = tss;
	runningKernelThread->bp = tbp;
	runningKernelThread->lck = lockFlag;

	// returning to user mode
	kernelMode = 0;

	switch (PCB::running->state) {
	case PCB::ASLEEP:
		sleeping.insert((PCB*)PCB::running);
		break;
	case PCB::READY:
		Scheduler::put((PCB*)PCB::running);
	}

	PCB::running = Scheduler::get();
	if (!PCB::running) PCB::running = idle;

	// restoring userThread's context
	tsp = PCB::running->sp;
	tss = PCB::running->ss;
	tbp = PCB::running->bp;

	lockFlag = PCB::running->lck;
	timeLeft = PCB::running->quantum;

#ifndef BCC_BLOCK_IGNORE
	asm {
		mov sp, tsp
		mov ss, tss
		mov bp, tbp
	}
#endif
}