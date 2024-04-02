#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
    h->elemAmount = 0;
    h->elemSize = elemSize;
    h->numBuckets = numBuckets;
    h->hashFn = hashfn;
    h->cmpFn = comparefn;
    h->freeFn = freefn;
    h->elems = malloc(numBuckets * sizeof(vector));
    
    for(int i = 0; i < numBuckets; i++)
    {
        VectorNew(&(h->elems[i]), h->elemSize, h->freeFn, 0);
    }
}

void HashSetDispose(hashset *h)
{
    //iterate over all the buckets and dispose of the vectors
    for(int i = 0; i < h->numBuckets; i++)
    {
        VectorDispose(&(h->elems[i]));
    }
    //free the space allocated for the vectors
    free(h->elems);
}

int HashSetCount(const hashset *h)
{
    return h->elemAmount;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
    //use the map function for all the vectors in the hashset
    for(int i = 0; i < h->numBuckets; i++)
    {
        VectorMap(&(h->elems[i]), mapfn, auxData);
    }
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
    int bucket = h->hashFn(elemAddr, h->numBuckets);
    int indexInVector = VectorSearch(&(h->elems[bucket]), elemAddr, h->cmpFn, 0, false);
    //insert the element if it hasn't been inserted before
    if(indexInVector == -1){
        VectorAppend(&(h->elems[bucket]), elemAddr);
        h->elemAmount++;
    }
    //replace the old one otherwise
    else
        VectorReplace(&(h->elems[bucket]), elemAddr, indexInVector);
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
    int bucket = h->hashFn(elemAddr, h->numBuckets);
    int indexInVector = VectorSearch(&(h->elems[bucket]), elemAddr, h->cmpFn, 0, false);
    //return NULL if the element can't be found
    if(indexInVector == -1)
        return NULL;
    //return the pointer to the element if it can be found
    return VectorNth(&(h->elems[bucket]), indexInVector);
}
