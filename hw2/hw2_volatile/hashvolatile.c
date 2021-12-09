#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_CAPACITY 1024
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

typedef struct{
    int isval;
    uint64_t key; 
    char value[64000];
}ht_entry;

struct ht {
    ht_entry* entries;
    int capacity;
    int length;
};
typedef struct ht ht;

static uint64_t hash_key(uint64_t key, int n) {
    uint64_t hash = key%n;
    return hash;
}

ht* ht_create() {
    ht* table = (ht*)malloc(sizeof(ht));
    if (table == NULL) {
        return NULL;
    }
    table->length = 0;
    table->capacity = INITIAL_CAPACITY;

    table->entries = malloc(table->capacity*sizeof(ht_entry));
    if(table->entries == NULL) {
        free(table);
        return NULL;
    }
    return table;
}

void ht_get(ht* table, uint64_t key) {
    uint64_t hash = hash_key(key, table->capacity);
    int index = hash_key(key, table->capacity);

    while (table->entries[index].isval != 0) {
        if (key == table->entries[index].key) {
            printf("idx: %ld\t key: %s\t key1: %s\t value: %s\n", index, key, table->entries[index].key, table->entries[index].value);
            return;
        }
        else{
            index++;
        }

        if(index >= table->capacity) {
            index = 0;
        }
    }
    printf("entry not found\n");
}

int ht_set_entry(ht_entry* entries, int capacity, uint64_t key, char* value, int *plength) {
    int index = hash_key(key, capacity);

    while(entries[index].isval != 0) {
        if(strcmp(key, entries[index].key) == 0) {
            strcpy(entries[index].value, value);
            return 0;
        }

        index++;
        if(index >= capacity) {
            index = 0;
        }
    }
    
    (*plength)++;    
    entries[index].isval = 1;
    entries[index].key = key;
    strcpy(entries[index].value, value);
    return 1;
}

bool ht_expand(ht* table) {
    int new_capacity = table->capacity*2;
    ht_entry* new_entries = malloc(new_capacity*sizeof(ht_entry));
    if(new_entries == NULL) {
        printf("memory allocation failed!\n");
        return false;
    }

    for(int i = 0; i < table->capacity; i++) {
        ht_entry entry = table->entries[i];
        ht_set_entry(new_entries, new_capacity, entry.key, entry.value, NULL);        
    }

    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

int ht_set(ht* table, uint64_t key, char* value) {
    // if(table->length >= table->capacity-10) {
    //     if(!ht_expand(table)) {
    //         printf("couldn't expand table\n");
    //         return 0;
    //     }
    // }

    return ht_set_entry(table->entries, table->capacity, key, value, &table->length);
}

void printhash(ht *htable){        
    int idx = 0;
    char *str = NULL;
    while(idx < htable->capacity) {
        if(htable->entries[idx].isval){
            printf("Key:%ld\t Value:%s\n", htable->entries[idx].key, htable->entries[idx].value);
        }
        /*else{
            printf("Key:%s\t Value:%s\n", htable->entries[idx].key, htable->entries[idx].value);
        }*/
        idx++;      
    }
}

int main(int argc, char *argv[]){
    int ch;
    uint64_t key;
    char val[1000], *gval;    
    ht *htable = ht_create();
    if(htable == NULL){
        printf("error creating hashtable\n");
        return 0;     
    }

    double time_spent = 0.0;
    clock_t begin = clock();
    int dbg=0;
    while(dbg<1000){
        //printf("1-insert 2-retrieve 3-print hashtable 4-quit\n");
        //scanf("%d",&ch);
        ch=1;
        if(ch==1){
            // printf("enter key:\t");
            // scanf("%ld", key);
            // printf("enter value:\t");
            // scanf("%s", val);            
            key=dbg*3;
            strcpy(val,"test-value");
            ht_set(htable, key, val);
            // printf("key %ld val %s stored!\n",key,val);
        }
        else if(ch==2){
            printf("enter key\t");
            scanf("%ld", key);
            ht_get(htable, key);            
        }
        else if(ch==3){
            printhash(htable);
        }
        else if(ch==4){
            //add persist code here
            break;
        }
        else{
            printf("Invalid command\n");
        }
        dbg++;
    }
    clock_t end = clock();
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time taken: %f seconds\n",time_spent);
    return 1;
}