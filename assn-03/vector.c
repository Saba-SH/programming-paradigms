#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <assert.h>

/**
 * Method: vectorGrow
 * ----------------
 * used to increase the space allocated for the vector. Used in the implementation of 
 * VectorInsert. Not supposed to be called by the client.
*/
static void vectorGrow(vector *v)
{
    //make sure that the vector address is valid
    assert(v != NULL);
    //reallocate the memory
    v->elems = realloc(v->elems, (v->allocLen * 2 + 1) * v->elemSize);
    //update the allocLen
    v->allocLen *= 2;
    v->allocLen++;
}

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    v->logLen = 0;
    v->allocLen = initialAllocation;
    v->freeFn = freeFn;
    v->elemSize = elemSize;
    v->elems = malloc(v->elemSize * initialAllocation);
}

void VectorDispose(vector *v)
{
    //if the free function exists, free all the elements
    if(v->freeFn != NULL)
    {
        for(int i = 0; i < v->logLen; i++)
        {
            v->freeFn(VectorNth(v, i));
        }
    }
    //free the space allocated for the vector
    free(v->elems);
}

int VectorLength(const vector *v)
{
    return v->logLen;       //return the logical length of the vector
}

void *VectorNth(const vector *v, int position)
{
    assert(position >= 0 && position < v->logLen);      //make sure that the index is valid
    return (void*)((char*)v->elems + v->elemSize * position);    //return the element at that index
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    //free the old element if necessary
    if(v->freeFn != NULL)
        v->freeFn(VectorNth(v, position));

    //find the position in the vector
    void *pos = VectorNth(v, position);
    //insert the given element at the position
    memmove(pos, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    //make sure that the index is valid
    assert(position >= 0 && position <= v->logLen);
    //grow the vector if necessary
    if(v->logLen >= v->allocLen)
    {
        vectorGrow(v);
    }

    //move over the elements after the position of the new one
    for(int i = v->logLen - 1; i >= position; i--)
    {
        //shift one element to the right by one
        memcpy((char*)VectorNth(v, i) + v->elemSize, VectorNth(v, i), v->elemSize);
    }

    //increase the logical length of the vector
    v->logLen++;
    //copy the given element to its position in the vector
    memmove(VectorNth(v, position), elemAddr, v->elemSize);
}

void VectorAppend(vector *v, const void *elemAddr)
{
    //insert the element at the end of the vector
    VectorInsert(v, elemAddr, v->logLen);
}

void VectorDelete(vector *v, int position)
{
    assert(position >= 0 && position < v->logLen);
    void *elemAddr = VectorNth(v, position);
    //free the element if the free function is not null
    if(v->freeFn != NULL)
    {
        v->freeFn(elemAddr);
    }
    //move all elements after the one that we removed
    for(int i = position + 1; i < v->logLen; i++)
    {
        //shift an element to the left by one
        memcpy((char*)VectorNth(v, i) - v->elemSize, VectorNth(v, i), v->elemSize);
    }
    //decrease the logical length of the vector
    v->logLen--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);

    qsort(v->elems, v->logLen, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    //make sure that the map function is not null
    assert(mapFn != NULL);
    //iterate over all the elements and call the map function on all of them
    for(int i = 0; i < v->logLen; i++)
    {
        mapFn(VectorNth(v, i), auxData);
    }
}

/**Wrapper for the VectorSearch function. We assume that the input provided for this function is proper.*/
static int VectorSearchSafe(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    void *searchResult = NULL;
    size_t amountOfItems = (size_t)(v->logLen - startIndex);
    if(isSorted)
        searchResult = bsearch(key, VectorNth(v, startIndex), amountOfItems, v->elemSize, searchFn);
    else
        searchResult = lfind(key, VectorNth(v, startIndex), &amountOfItems, v->elemSize, searchFn);

    //return the index of the element in the vector
    if(searchResult == NULL)
        return -1;
    else
        return ((char*)searchResult - (char*)v->elems) / v->elemSize;
}

int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    //make sure that all the input is valid
    assert(key != NULL);
    assert(searchFn != NULL);
    //don't search through an empty vector
    if(v->logLen == 0)
    {
        return -1;
    }
    assert(startIndex >= 0 && startIndex < v->logLen);

    return VectorSearchSafe(v, key, searchFn, startIndex, isSorted);
}
