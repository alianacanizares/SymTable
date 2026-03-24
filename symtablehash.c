/* symtablehash.c - hash table implementation of symbol table */

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "symtable.h"

enum {BUCKET_COUNT = 509};

struct hashTableBinding {
    const char *key;
    const void *value;
    struct hashBinding *psNextBinding;
};

struct Table {
    struct hashTableBinding **buckets;
    size_t listSize;
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
    newSymTable->bucketCount = BUCKET_COUNT; 
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
    if (oSymTable == NULL) return NULL;
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

int SymTable_put(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue)
{
    struct hashTableBinding *psCurrentBinding;
    struct hashTableBinding *newBinding;
    assert(pcKey != NULL);
    assert(oSymTable != NULL);
    size_t hashedAddress = SymTable_hash(pcKey, oSymTable->bucketCount);
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
    oSymTable->listSize++;
    oSymTable->buckets[hashedAddress] = newBinding;                     
}


void *SymTable_replace(SymTable_T oSymTable,
     const char *pcKey, const void *pvValue)
{
    struct hashTableBinding *psCurrentBinding;
    void *oldValue;
    assert(oSymTable != NULL);
    size_t hashedAddress = SymTable_hash(pcKey, oSymTable->bucketCount);
    for (psCurrentBinding = oSymTable->buckets[hashedAddress]; 
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding) 
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0) {
        oldValue = psCurrentBinding->value;         
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
    size_t hashedAddress = SymTable_hash(pcKey, oSymTable->bucketCount);
    for (psCurrentBinding = oSymTable->buckets[hashedAddress]; 
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding) 
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0)
        return psCurrentBinding->value;   
    }
        return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{
    struct hashTableBinding *psCurrentBinding;
    struct hashTableBinding *psPreviousBinding = NULL;
    void *oldValue;
    size_t hashedAddress = SymTable_hash(pcKey, oSymTable->bucketCount);
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
            oldValue = psCurrentBinding->value;
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
                (*pfApply)(psCurrentBinding->key, psCurrentBinding->value,
                (void*)pvExtra);
             }
    }
}
     