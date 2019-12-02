#ifndef _THREAD_H_
#define _THREAD_H_

typedef unsigned long StackSize;
const StackSize defaultStackSize = 4096;
typedef unsigned int Time;
const Time defaultTimeSlice = 2;
typedef int ID;

class PCB;

class Thread {
public:
	virtual ~Thread();

	void start();
	void waitToComplete();

	static void sleep(Time timeToSleep);
protected:
	Thread(StackSize stackSize = defaultStackSize, Time timeSlice = defaultTimeSlice);

	friend class PCB;

	virtual void run() {}
private:
	ID id;

	static void wrapper(Thread* running);
};

void dispatch();

#endif