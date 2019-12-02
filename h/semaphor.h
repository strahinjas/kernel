#ifndef _SEMAPHOR_H_
#define _SEMAPHOR_H_

typedef int ID;

class Semaphore {
public:
	Semaphore(int init = 1);
	virtual ~Semaphore();

	int val() const;

	virtual int wait(int toBlock);
	virtual void signal();
private:
	ID id;
};

#endif