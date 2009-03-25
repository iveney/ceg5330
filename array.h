#ifndef __ARRAY_H
#define __ARRAY_H

#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "igl.h"

#ifdef DEBUG
// check array bound during dereference
#define ARRAY_CHECK_BOUND
// initialize the newly allocated space to all zeros
#define ARRAY_INIT_ZERO
#endif

// the default capacity of newly allocated array
#define ARRAY_DEFAULT_CAPACITY 3

//====================================================
// macro for efficient access of array
//====================================================
// array - an Array object
// idx - the index
#ifdef ARRAY_CHECK_BOUND
#define array_fetch(array, idx) ( \
		((idx) >= 0 && (idx) < (array).size) ? \
		0 : (array).abort(0, idx), \
		((array).space[(idx)]))
#else
#define array_fetch(array, idx) ((array).space[(idx)])
#endif

template <class T>
class Array {
	int capacity; // capacity of the array

	void resize(int newsize);

	public:
	T *space;
	int size; // occupied size
	
	Array();	
	Array(int cap); // allocate a new array of capacity cap
	Array(Array<T>& fromArray); // copy a new array from fromArray
	~Array();

	// FIXME: T& ==> cannot insert constant value
	void insert(T& item); // insert item at the end
	void insert(int idx, T& item); // insert item after the index i

	T& remove(); // remove the item at the end
	T& remove(int idx); // remove the item at index i
	
	void sort(int (*compare)(const void *, const void *) = cmp); // qsort using compare as compare function
	int find(T& item, int (*compare)(const void *, const void *) = cmp); // return the index of item
	void append(Array<T>& anotherArray); // append anotherArray to the end of this array

	void clear(); // clear the content of the array, but retain the memory allocated

	T& operator[] (int idx){
#ifdef ARRAY_CHECK_BOUND
		if (!(idx >= 0 && idx < size)) abort(0, idx);
#endif
		return space[idx];
	}

	int abort(int error, int idx); // show error message and exit

	static int cmp(const void *a_, const void *b_) {
		T *a = (T *) a_;
		T *b = (T *) b_;
		if (*a > *b) return 1;
		else if (*a == *b) return 0;
		else return -1;
	}
};

// constructors-----------------------------------------
template <class T>
inline Array<T>::Array() {
	space = (T *) malloc(sizeof(T) * ARRAY_DEFAULT_CAPACITY);
	if (space == NULL) abort(1, 0);
	
	capacity = ARRAY_DEFAULT_CAPACITY;
	size = 0;
#ifdef ARRAY_INIT_ZERO
	memset(space, 0, sizeof(T) * ARRAY_DEFAULT_CAPACITY);
#endif
};

template <class T>
inline Array<T>::Array(int cap) {
	space = (T *) malloc(sizeof(T) * cap);
	if (space == NULL) abort(1, 0);
	
	capacity = cap;
	size = 0;
#ifdef ARRAY_INIT_ZERO
	memset(space, 0, sizeof(T) * cap);
#endif
};

template <class T>
inline Array<T>::Array(Array<T> &fromArray) {
	space = (T *) malloc(sizeof(T) * fromArray.size);
	if (space == NULL) abort(1, 0);
	
	capacity = fromArray.size;
	for (int i = 0; i < fromArray.size; i++) {
		space[i] = fromArray[i];
	}
	size = fromArray.size;
};

template <class T>
inline Array<T>::~Array() {
	free(space);
};

// member functions-------------------------------------
template <class T>
inline void Array<T>::resize(int newsize) {
#ifdef ARRAY_INIT_ZERO
	int old_size = capacity;
	T *pos;
#endif

	capacity = MAX(capacity * 2, newsize);
	space = (T *) realloc(space, capacity * sizeof(T));
	if (space == NULL) abort(1, 0);

#ifdef ARRAY_INIT_ZERO
	pos = space + old_size;
	memset(pos, 0, (capacity - old_size) * sizeof(T));
#endif
};

template <class T>
inline void Array<T>::insert(T& item) {
	if (size >= capacity){
		resize(capacity + 1);
	}

	space[size] = item;
	size += 1;
};

template <class T>
inline void Array<T>::insert(int idx, T& item) {
#ifdef ARRAY_CHECK_BOUND
	if (!(idx >= 0 && idx < size)) abort(0, idx);
#endif

	if (size >= capacity){
		resize(capacity + 1);
	}

	memmove(space + idx + 1, space + idx, sizeof(T) * size - idx);
	space[idx] = item;
	size += 1;
};

template <class T>
inline T& Array<T>::remove() {
#ifdef ARRAY_CHECK_BOUND
	if (size == 0) abort(0, 0);
#endif
	size -= 1;
	return space[size];
}; // remove the item at index i

template <class T>
inline T& Array<T>::remove(int idx) {
#ifdef ARRAY_CHECK_BOUND
	if (!(idx >= 0 && idx < size)) abort(0, idx);
#endif

	T tmp = space[idx];
	memmove(space + idx, space + idx + 1, sizeof(T) * size - idx - 1);
	size -= 1;
	space[size] = tmp;

	return space[size];
}; // remove the item at index i

template <class T>
inline void Array<T>::sort(int (*compare)(const void *, const void *)){
	qsort(space, size, sizeof(T), compare);
}; // qsort using compare as compare function

template <class T>
inline int Array<T>::find(T& item, int (*compare)(const void *, const void *)) {
	T *itmptr = (T *) bsearch(&item, space, size, sizeof(T), compare);

	if (itmptr != NULL) {
		return itmptr - space;
	}

	return -1;
}; // return the index of item

template <class T>
inline void Array<T>::append(Array<T>& anotherArray) {
	if ((capacity - size) < anotherArray.size) {
		resize(size + anotherArray.size);
	}
	memcpy(space + size, anotherArray.space, anotherArray.size * sizeof(T));
	size += anotherArray.size;
}; // append another Array to the end of this array

template <class T>
inline void Array<T>::clear() {
#ifdef ARRAY_INIT_ZERO
	memset(space, 0, capacity * sizeof(T));
#endif
	size = 0;
}; // clear the whole array

template <class T>
int Array<T>::abort(int error, int idx) {
	fprintf(stderr,"Array: Error: ");
	switch (error) {
		case 0: // index out of bound
			fprintf(stderr,"array index %d out of bound [0, %d]\n", idx, size - 1);
			break;

		case 1: // not enough memory
			fprintf(stderr,"memory not enough\n");
			break;

		default:
			fprintf(stderr, "unknown error\n");
			break;
	}
	
	std::abort();
	
	return 0;
};

#endif
