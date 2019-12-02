#ifndef _VECTOR_H_
#define _VECTOR_H_

const unsigned int defaultCapacity = 10;

// kernel's internal generic dynamic array

template<class T> class Vector {
public:
	Vector(int capacity = defaultCapacity) : sz(0), cap(capacity) {
		if (cap < 1) cap = defaultCapacity;
		array = new T*[cap];
	}

	~Vector() { delete[] array; }

	unsigned int size() const { return sz; }
	unsigned int capacity() const { return cap; }

	void push_back(T*);
	T* pop_back();

	T* operator[](int i) {
		if (i < 0 || i >= sz) return 0;
		return array[i];
	}

	void shrink_to_fit();
private:
	T** array;
	unsigned int sz, cap;
};

template<class T> void Vector<T>::push_back(T* data) {
	if (sz == cap) {
		T** newArray = new T*[cap += defaultCapacity];
		for (int i = 0; i < sz; i++) newArray[i] = array[i];
		delete[] array; array = newArray;
	}
	array[sz++] = data;
}

template<class T> T* Vector<T>::pop_back() {
	if (sz == 0) return 0;
	T* data = array[--sz]; array[sz] = 0;
	if (cap - sz > (2 * defaultCapacity)) {
		T** newArray = new T*[cap -= defaultCapacity];
		for (int i = 0; i < sz; i++) newArray[i] = array[i];
		delete[] array; array = newArray;
	}
	return data;
}

template<class T> void Vector<T>::shrink_to_fit() {
	if (sz == cap) return;
	T** newArray = new T*[cap = sz];
	for (int i = 0; i < sz; i++) newArray[i] = array[i];
	delete[] array; array = newArray;
}

#endif