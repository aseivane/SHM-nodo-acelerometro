#ifndef CIRCULAR_ORDERED_ARRAY_H
#define CIRCULAR_ORDERED_ARRAY_H

#include <stdint.h>
#include <stdlib.h>

/*
* This library provide a list wich orders itself as new entries are added.
*/

struct CircularOrderedArray { 
	int next;
	int size;

	void (*cpy)(void*, void*); 
	double (*cmp)(void*, void*);

	size_t dataSize;
	int maxSize;
	int* order; // array representing the order in which the data was stored, used to discard old entries.
	void** data; // array of data.
};

typedef struct CircularOrderedArray CircularOrderedArray;

/*
* Allocates resources for the new list. Parameters are :
* 	- maxSize -> the maximum size the list will take before discarding old values.
*	- dataSize -> the size of the datatype that it will store.
*	- cpy -> a 'copy(soruce, target)' function that is able to copy the data type that will be stored
*	- cmp -> a 'compare(first, second)' function that is able to compare the data type that will be stored where:
*		- < 0 <-> first < second.
*		- = 0 <-> first = second.
*		- > 0 <-> first > second.
*/
CircularOrderedArray * initCircularOrderedArray(int maxSize, size_t dataSize, void (*cpy)(void*, void*), double (*cmp)(void*, void*));

// Discards all current entries of the array
void resetCircularOrderedArray(CircularOrderedArray *);

// Frees alocated resources
void freeCircularOrderedArray(CircularOrderedArray *);

// Inserts a new entry in the array
void insertOrdered(CircularOrderedArray *, void *);

// Executes the function 'f' foreach element of the array in order.
void foreach(CircularOrderedArray *, void (*f)(void*));

#endif