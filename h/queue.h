#ifndef _QUEUE_H_
#define _QUEUE_H_

// kernel's internal generic queue

template<class T> class Queue {
public:
	Queue() { sz = 0; head = tail = 0; }
	~Queue();

	unsigned int size() const { return sz; }

	unsigned int empty() const { return head == 0; }

	void insert(T*);
	T* remove();

	struct Element {
		T* data; Element *prev, *next;
		Element(T* t, Element* p = 0, Element* n = 0) : data(t), prev(p), next(n) {}
	};

	Element *head, *tail;
private:
	unsigned int sz;
};

template<class T> Queue<T>::~Queue() {
	while (head) {
		Element* old = head;
		head = head->next;
		delete old;
	}
	sz = 0; tail = 0;
}

template<class T> void Queue<T>::insert(T* data) {
	Element* e = new Element(data);
	if (!head) head = e;
	else {
		e->prev = tail;
		tail->next = e;
	}
	tail = e; ++sz;
}

template<class T> T* Queue<T>::remove() {
	if (!head) return 0;
	Element* e = head;
	T* data = e->data;
	head = head->next;
	if (!head) tail = 0;
	else head->prev = 0;
	delete e; --sz;
	return data;
}

#endif