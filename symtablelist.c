
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "symtable.h"

struct symTableBinding 
{
    const char *key; 
    const void  *value;
    struct symTableBinding *psNextBinding;
};

struct Table 
{
    struct symTableBinding* firstBinding;
    int size;
};

SymTable_T SymTable_new(void) 
{                     
    SymTable_T newSymTable;
    newSymTable = malloc(sizeof(struct Table)); 
    if (newSymTable == NULL) return NULL;
    newSymTable->firstBinding = NULL;
    newSymTable->size = 0;
    return newSymTable;
}

void SymTable_free(SymTable_T oSymTable) 
{
    struct symTableBinding *psCurrentBinding;
    struct symTableBinding *psNextBinding;         
    assert(oSymTable != NULL);
    for (psCurrentBinding = oSymTable->firstBinding;
         psCurrentBinding != NULL;          
         psCurrentBinding = psNextBinding)             
    {
        psNextBinding = psCurrentBinding->psNextBinding;
        free(psCurrentBinding);
    }
        free(oSymTable);
}


size_t SymTable_getLength(SymTable_T oSymTable) 
{   
    return oSymTable->size;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) 
{
    struct symTableBinding *psCurrentBinding;
    struct symTableBinding *newBinding;
    assert(oSymTable != NULL);
    for (psCurrentBinding = oSymTable->firstBinding; 
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding) 
    {
        if (strcmp(pcKey, psCurrentBinding->key) == 0) {    
            return 0;
        }
    }
    newBinding = malloc(sizeof(*newBinding));
    newBinding->key = (const char*)malloc(strlen(pcKey)+1);
    strcpy((char*)newBinding->key, pcKey);
    newBinding->value = pvValue;      
    newBinding->psNextBinding = oSymTable->firstBinding;
    oSymTable->size++;
    oSymTable->firstBinding = newBinding;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue)
{
    struct symTableBinding *psCurrentBinding;
    void *oldValue;
    assert(oSymTable != NULL);
    for (psCurrentBinding = oSymTable->firstBinding;
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding)
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0){
            oldValue = psCurrentBinding->value;         
            psCurrentBinding->value = pvValue;         
            return oldValue;                         
        }
    }
        return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{
    struct symTableBinding *psCurrentBinding;
    for (psCurrentBinding = oSymTable->firstBinding;
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
    struct symTableBinding *psCurrentBinding;
    for (psCurrentBinding = oSymTable->firstBinding;
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
    struct symTableBinding *psCurrentBinding;
    struct symTableBinding *psPreviousBinding = NULL;

    void *oldValue;
    for (psCurrentBinding = oSymTable->firstBinding;
         psCurrentBinding != NULL;
         psPreviousBinding = psCurrentBinding,
         psCurrentBinding = psCurrentBinding->psNextBinding
        )
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0) {
            if(psPreviousBinding == NULL)
            {
                oSymTable->firstBinding = psCurrentBinding->psNextBinding;
            } else {       
                psPreviousBinding->psNextBinding = psCurrentBinding->psNextBinding;
            }
            oldValue = psCurrentBinding->value;
            oSymTable->size--;
            free(psCurrentBinding);
            return oldValue;
    }
    }
    return NULL;
}
/*
void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra);
*/
