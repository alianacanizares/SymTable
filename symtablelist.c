/* symtablist.c - linked list implementation of symbol table */

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "symtable.h"

/* Binding struct */
struct symTableBinding 
{
    /* key string */
    const char *key; 
    /* value */
    const void  *value;
    /* pointer to nextBinding */
    struct symTableBinding* psNextBinding;
};

/* Table struct*/
struct Table 
{
    /* first binding in list */
    struct symTableBinding* firstBinding;
    /* amount of bindings in list */
    size_t size;
};

/* creates new SymTable */
SymTable_T SymTable_new(void) 
{                     
    SymTable_T newSymTable;
    newSymTable = malloc(sizeof(struct Table)); 
    if(newSymTable == NULL){
        return NULL;
    }
    if (newSymTable == NULL) return NULL;
    newSymTable->firstBinding = NULL;
    newSymTable->size = 0;
    return newSymTable;
}

/* Frees memory in each binding of oSymTable, then frees oSymTable */
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

/* Returns number of bindings in oSymTable */
size_t SymTable_getLength(SymTable_T oSymTable) 
{   
    return oSymTable->size;
}

/* Puts a binding with pcKey and pvValue into oSymTable */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) 
{
    struct symTableBinding *psCurrentBinding;
    struct symTableBinding *newBinding;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrentBinding = oSymTable->firstBinding; 
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding) 
    {
        if (strcmp(pcKey, psCurrentBinding->key) == 0) {    
            return 0;
        }
    }
    newBinding = malloc(sizeof(*newBinding));
    if(newBinding == NULL){
        return 0;
    }
    newBinding->key = (const char*)malloc(strlen(pcKey)+1);
    if(newBinding->key == NULL) {
    free(newBinding);
    return 0;
    }
    strcpy((char*)newBinding->key, pcKey);
    newBinding->value = pvValue;      
    newBinding->psNextBinding = oSymTable->firstBinding;
    oSymTable->size++;
    oSymTable->firstBinding = newBinding;
    return 1;
}

/* Replaces a binding with given pcKey and pvValue in oSymTable */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue)
{
    struct symTableBinding *psCurrentBinding;
    void *oldValue;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (psCurrentBinding = oSymTable->firstBinding;
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding)
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0){
            oldValue = (void*) psCurrentBinding->value;         
            psCurrentBinding->value = pvValue;         
            return oldValue;                         
        }
    }
        return NULL;
}

/* Checks if oSymTable contains pcKey */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{
    struct symTableBinding *psCurrentBinding;
    assert(pcKey != NULL);
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

/* Returns value of given pcKey in oSymTable */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{
    struct symTableBinding *psCurrentBinding;
    assert(pcKey != NULL);
    for (psCurrentBinding = oSymTable->firstBinding;
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding)
    {
        if(strcmp(pcKey, psCurrentBinding->key) == 0)
        return (void*) psCurrentBinding->value;                    
    }
        return NULL;
}

/* Removes pcKey in oSymTable and frees memory */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{
    struct symTableBinding *psCurrentBinding;
    struct symTableBinding *psPreviousBinding = NULL;
    void *oldValue;
    assert(pcKey != NULL);
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
            oldValue = (void*) psCurrentBinding->value;
            oSymTable->size--;
            free(psCurrentBinding);
            return oldValue;
    }
    }
    return NULL;
}

/* Maps the given pcKey, pvValue, pvExtra to a binding in oSymTable */
void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra)
{
    struct symTableBinding *psCurrentBinding;
    assert(pfApply != NULL);
    for (psCurrentBinding = oSymTable->firstBinding;
         psCurrentBinding != NULL;
         psCurrentBinding = psCurrentBinding->psNextBinding)
    {
        (*pfApply)(psCurrentBinding->key, (void*)psCurrentBinding->value, (void*)pvExtra);
    }
}
