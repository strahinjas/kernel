#ifndef _KERNEL_H_
#define _KERNEL_H_

#include <dos.h>

#include "queue.h"
#include "vector.h"
#include "thread.h"
#include "schedule.h"

typedef unsigned char IVTNo;
typedef void interrupt (*pInterrupt)(...);

extern ID uniqueThreadID;
extern ID uniqueSemaphoreID;
extern ID uniqueEventID;

// structure for parameters of system calls
struct CallParams {
	ID callID, objectID;
	Thread* thread;
	StackSize stackSize;
	Time time;
	CallParams() { thread = 0; callID = objectID = stackSize = time = 0; }
};

// helper function - system call parameters into registers
void transfer(CallParams*);

void lock();
void unlock();

void init();
void restore();

// interrupt routines
void interrupt timer(...);
void interrupt sys_call(...);
void interrupt sys_dispatch(...);

#endif