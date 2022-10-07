/*
 * circularOrderedArray.c
 *
 *  Created on: May 24, 2020
 *      Author: jaatadia@gmail.com
 */
#include "circularOrderedArray.h"

void initCircularOrderedArray(CircularOrderedArray *array) {
	array->next = 0;
	array->size = 0;
}

void swap(double *first, double *second) {
	double aux = *first;
	*first = *second;
	*second = aux;
}

int orderRight(double *array, int position, int size) {
	int nextPosition = position;
	while ((nextPosition + 1 < size)
			&& array[nextPosition] > array[nextPosition + 1]) { //hasNext && bigger than next
		swap(&array[nextPosition], &array[nextPosition + 1]);
		nextPosition++;
	}
	return position != nextPosition;
}

void orderLeft(double *array, int position) {
	while (position > 0 && array[position] < array[position - 1]) { //hasPrevious && smaller than previous
		swap(&array[position], &array[position - 1]);
		position--;
	}
}

void orderPosition(double *array, int position, int size) {
	if (!orderRight(array, position, size)) {
		orderLeft(array, position);
	}
}

void insertOrdered(CircularOrderedArray *array, double value) {
	int insertPosition = 0;
	if (array->size < CIRCULAR_ORDERED_ARRAY_MAX_SIZE) {
		insertPosition = array->size;
		array->fifo[array->size] = value;
		array->size++;
	} else {
		double valueToRemove = array->fifo[array->next];
		for (int i = 0; i < CIRCULAR_ORDERED_ARRAY_MAX_SIZE; i++) {
			if (array->array[i] == valueToRemove) {
				insertPosition = i;
				break;
			}
		}
		array->fifo[array->next] = value;
		array->next = (array->next + 1) % CIRCULAR_ORDERED_ARRAY_MAX_SIZE;
	}

	array->array[insertPosition] = value;
	orderPosition(array->array, insertPosition, array->size);
}

double median(CircularOrderedArray *array) {
	return array->array[array->size / 2];
}
