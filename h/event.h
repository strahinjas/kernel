#ifndef _EVENT_H_
#define _EVENT_H_

typedef int ID;
typedef unsigned char IVTNo;
typedef void interrupt (*pInterrupt)(...);

#define PREPAREENTRY(ivtNo, callOld)\
void interrupt intr##ivtNo(...) {\
	IVTEntry::signal(ivtNo);\
	if (callOld) IVTEntry::oldOne(ivtNo)();\
}\
IVTEntry ivte##ivtNo(intr##ivtNo, (IVTNo) ivtNo);

class KernelEv;

class Event {
public:
	Event(IVTNo ivtNo);
	~Event();

	void wait();
	void signal();
private:
	ID id;
};

class IVTEntry {
public:
	IVTEntry(pInterrupt routine, IVTNo ivtNo);
	~IVTEntry();

	static IVTEntry* myIVT[256];

	static void signal(IVTNo ivtNo);
	static pInterrupt oldOne(IVTNo ivtNo);

private:
	IVTNo entry;
	KernelEv* event;
	pInterrupt oldRoutine, newRoutine;

	friend class KernelEv;
};

#endif