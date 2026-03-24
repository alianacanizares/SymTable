/* symtablehash.c - hash table implementation of symbol table */

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "symtable.h"

/* global variables for expansion */
enum {BUCKET_COUNT_509, BUCKET_COUNT_1021, BUCKET_COUNT_2039, BUCKET_COUNT_4093, BUCKET_COUNT_8191, 
    BUCKETCOUNT5_16381, BUCKET_COUNT_32749, BUCKET_COUNT_MAX};

static const size_t bucketSize[] = {509, 1021, 2039, 4093, 8191, 16381, 32749, 65521};

/* struct object for bindings that has key, value, and pointer to psNextBinding*/
struct hashTableBinding {
    /* key for each binding */
    const char *key;
    /* value associated */
    const void *value;
    /* pointer to next binding */
    struct hashTableBinding *psNextBinding;
};

/* struct object that has buckets, listSize, and bucketcount */
struct Table {
    /* pointer to pointer of bindings */
    struct hashTableBinding **buckets;
    /* amount of bindings */
    size_t listSize;
    /* amount of buckets in array */
    size_t bucketCount;
};

/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
   inclusive. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
   const size_t HASH_MULTIPLIER = 65599;
   size_t u;
   size_t uHash = 0;

   assert(pcKey != NULL);

   for (u = 0; pcKey[u] != '\0'; u++)
      uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

   return uHash % uBucketCount;
}

SymTable_T SymTable_new(void)
{
    SymTable_T newSymTable;
    size_t i;
    newSymTable = malloc(sizeof(struct Table));
    if (newSymTable == NULL) return NULL;
    newSymTable->bucketCount = bucketSize[0]; 
    newSymTable->listSize = 0;
    newSymTable->buckets = malloc(sizeof(struct hashTableBinding*) * newSymTable->bucketCount);
    if(newSymTable->buckets == NULL) {
        free(newSymTable);
        return NULL;
    }
    for (i = 0; i < newSymTable->bucketCount; i++) {
        newSymTable->buckets[i]=NULL;                         
    }  
    return newSymTable;
}

void SymTable_free(SymTable_T oSymTable)
{
    size_t psCurrentBinding;
    struct hashTableBinding* currentBinding;
    if (oSymTable == NULL) return;
    for (psCurrentBinding = 0;
         psCurrentBinding < oSymTable->bucketCount;          
         psCurrentBinding++)             
    {
        currentBinding = oSymTable->buckets[psCurrentBinding];
        while(currentBinding != NULL) 
        {
        free((char*)currentBinding->key);
        free(currentBinding);
        currentBinding = currentBinding->psNextBinding;
        }

    }
    free(oSymTable->buckets);
    free(oSymTable);
}


size_t SymTable_getLength(SymTable_T oSymTable)
{
    return oSymTable->listSize;
}

static void SymTable_expand(SymTable_T oSymTable)
{
    size_t i = 0;
    size_t newBucketCount = 0;
    size_t newHashedAddress = 0;
    struct hashTableBinding **newBuckets;
    struct hashTableBinding *psCurrentBinding;
    struct hashTableBinding *psNextBinding;
    for (i = 0; i < BUCKET_COUNT_MAX; i++)
    {
        if(oSymTable->listSize > bucketSize[i]) {
            newBucketCount = bucketSize[i + 1];
            break;
         }
    }
    if (i == BUCKET_COUNT_MAX)
        return;

    newBuckets = malloc(newBucketCount * sizeof(*newBuckets));
    if(newBuckets == NULL) {
        return;
    }
    for (i = 0; i < newBucketCount; i++) {
    newBuckets[i] = NULL;
    }
    for (i = 0; i < oSymTable->bucketCount; i++) {
        psCurrentBinding = oSymTable->buckets[i];
        while(psCurrentBinding != NULL) {
            /* saving next pointer before changing */
            psNextBinding = psCurrentBinding->psNextBinding;
            /* new hashed address using new bucket count*/
            newHashedAddress = SymTable_hash(psCurrentBinding->key, newBucketCount);
            /* saving next before changing pointer */
            psCurrentBinding->psNextBinding = newBuckets[newHashedAddress];
            /* changing to point to new head */
            newBuckets[newHashedAddress] = psCurrentBinding;
            /* moving back to original place */
            psCurrentBinding = psNextBinding;
        }
    }
    free(oSymTable->buckets);
    oSymTable->buckets = newBuckets;
    oSymTable->bucketCount = newBucketCount;
}

int SymTable_put(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue)
{
    struct hashTableBinding *psCurrentBinding;
    struct hashTableBinding *newBinding;
    size_t hashedAddress;
    assert(pcKey != NULL);
    assert(oSymTable != NULL);
    hashedAddress = SymTable_hash(pcKey, oSymTable->bucketCount);
    for (psCurrentBinding = oSymTable->buckets[hashedAddress]; 
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding) 
    {
        if (strcmp(pcKey, psCurrentBinding->key) == 0) {    
            return 0;
        }
    }
    newBinding = malloc(sizeof(*newBinding));
    if(newBinding == NULL) return 0;
    newBinding->key = (const char*)malloc(strlen(pcKey)+1);
    if (newBinding->key == NULL) {
        free(newBinding);
        return 0;
    }
    strcpy((char*)newBinding->key, pcKey);
    newBinding->value = (void*) pvValue; 
    newBinding->psNextBinding = oSymTable->buckets[hashedAddress];
    oSymTable->buckets[hashedAddress] = newBinding;   
    oSymTable->listSize++;

    SymTable_expand(oSymTable);

    return 1;               
}


void *SymTable_replace(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue)
{
    struct hashTableBinding *psCurrentBinding;
    void *oldValue;
    size_t hashedAddress = 0;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    hashedAddress = SymTable_hash(pcKey, oSymTable->bucketCount);
    for (psCurrentBinding = oSymTable->buckets[hashedAddress]; 
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding) 
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0) {
        oldValue = (void *)psCurrentBinding->value;         
        psCurrentBinding->value = pvValue;         
        return oldValue;    
        }
    }
        return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{
    struct hashTableBinding *psCurrentBinding;
    size_t hashedAddress = SymTable_hash(pcKey, oSymTable->bucketCount);
    assert(pcKey != NULL);
    for (psCurrentBinding = oSymTable->buckets[hashedAddress]; 
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding) 
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0){
        return 1;
        }
    }
    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{
    struct hashTableBinding *psCurrentBinding;
    size_t hashedAddress;
    assert(pcKey != NULL);
    hashedAddress = SymTable_hash(pcKey, oSymTable->bucketCount);
    for (psCurrentBinding = oSymTable->buckets[hashedAddress]; 
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding) 
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0)
        return (void *)psCurrentBinding->value;   
    }
        return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{
    struct hashTableBinding *psCurrentBinding;
    struct hashTableBinding *psPreviousBinding = NULL;
    void *oldValue;
    size_t hashedAddress;
    assert(pcKey != NULL);
    hashedAddress = SymTable_hash(pcKey, oSymTable->bucketCount);
    for (psCurrentBinding = oSymTable->buckets[hashedAddress];
         psCurrentBinding != NULL;
         psPreviousBinding = psCurrentBinding,
         psCurrentBinding = psCurrentBinding->psNextBinding
        )
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0) {
            if(psPreviousBinding == NULL)
            {
                oSymTable->buckets[hashedAddress] = psCurrentBinding->psNextBinding;
            } else {       
                psPreviousBinding->psNextBinding = psCurrentBinding->psNextBinding;
            }
            oldValue = (void *)psCurrentBinding->value;
            oSymTable->listSize--;
            free(psCurrentBinding);
            return oldValue;
    }
    }
    return NULL;
}


void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra)
{
        size_t psCurrentBucket = 0;
        struct hashTableBinding *psCurrentBinding;
        
    for (psCurrentBucket = 0; 
         psCurrentBucket < oSymTable->bucketCount;
         psCurrentBucket++) 
    {
        for (psCurrentBinding = oSymTable->buckets[psCurrentBucket];
             psCurrentBinding != NULL;
             psCurrentBinding = psCurrentBinding->psNextBinding)
             {
                (*pfApply)(psCurrentBinding->key, (void *)psCurrentBinding->value,
                (void*)pvExtra);
             }
    }
}
     