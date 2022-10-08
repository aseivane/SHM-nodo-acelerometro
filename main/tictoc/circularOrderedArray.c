#include "circularOrderedArray.h"
#include <stdio.h>
#include <inttypes.h>

//#define TICTOC_CIRCULAR_ARRAY_DEBUG

CircularOrderedArray * initCircularOrderedArray(int maxSize, size_t dataSize, void (*cpy)(void*, void*), double (*cmp)(void*, void*)){
	CircularOrderedArray* array = malloc(sizeof(CircularOrderedArray));
	array->next = 0;
	array->size = 0;

	array->maxSize = maxSize;
	array->cpy = cpy;
	array->cmp = cmp;
	array->order = malloc(sizeof(int)*maxSize);
	array->data = malloc(sizeof(void*)*maxSize);

	for(int i = 0; i < maxSize; i ++) {
		array->data[i]=malloc(dataSize);
	}

	return array;
}

void resetCircularOrderedArray(CircularOrderedArray* array){
	array->next = 0;
	array->size = 0;
}


void freeCircularOrderedArray(CircularOrderedArray* array){
	for(int i = 0; i < array->maxSize; i ++) {
		free(array->data[i]);
	}
	free(array->data);
	free(array->order);
	free(array);
}

void swap(CircularOrderedArray * array, int first, int second){
	// aux = first;
	int orderAux = array->order[first];
	void* dataAux = array->data[first];

	// first = second;
	array->order[first] = array->order[second];
	array->data[first] = array->data[second];

	// second = aux;
	array->order[second] = orderAux;
	array->data[second] = dataAux;
}

int orderRight(CircularOrderedArray * array, int position){
	#ifdef TICTOC_CIRCULAR_ARRAY_DEBUG
	printf("--- Ordering Right ---\n");
	#endif
	int nextPosition = position;

	while((nextPosition + 1 < array->size) && (((*array->cmp)(array->data[nextPosition], array->data[nextPosition+1])) > 0)){ //hasNext && bigger than next
		swap(array, nextPosition, nextPosition+1);
		#ifdef TICTOC_CIRCULAR_ARRAY_DEBUG
		printf("Swapped %d > %d\n", nextPosition, nextPosition+1);	
		#endif

		nextPosition++;
	}
	return position!=nextPosition;
}

int orderLeft(CircularOrderedArray * array, int position){
	#ifdef TICTOC_CIRCULAR_ARRAY_DEBUG
	printf("--- Ordering Left ---\n");
	#endif
	int nextPosition = position;

	while((nextPosition > 0) && (((*array->cmp)(array->data[nextPosition-1], array->data[nextPosition])) > 0)){ //hasPrevious && smaller than previous
		swap(array, nextPosition-1, nextPosition);
		#ifdef TICTOC_CIRCULAR_ARRAY_DEBUG
		printf("Swapped %d < %d\n", nextPosition-1, nextPosition);	
		#endif
		nextPosition--;
	}
	return position!=nextPosition;
}

void orderPosition(CircularOrderedArray * array, int position){
	#ifdef TICTOC_CIRCULAR_ARRAY_DEBUG
	printf("--- Ordering ---\n");
	#endif
	if(!orderRight(array, position)){
		orderLeft(array, position);
	}	
}

int findPosition(int* array, int size, int order) {
	for(int i = 0; i < size; i++){
		if(array[i] == order) return i;
	}

	//should never arrive here
	return -1;
}

void insertOrdered(CircularOrderedArray* array, void* node){
	int insertPosition;
	if(array->size < array->maxSize){
		insertPosition = array->size;
		array->size++;
	} else {
		insertPosition = findPosition(array->order, array->maxSize, array->next) ;
	}

	(*array->cpy)(node, array->data[insertPosition]);
	array->order[insertPosition] = array->next;
	array->next = (array->next + 1) % array->maxSize;

	orderPosition(array, insertPosition);
}


void foreach(CircularOrderedArray* array, void (*f)(void*)){
	for(int i = 0; i < array->size; i ++) {
		(*f)(array->data[i]);
	}
}
