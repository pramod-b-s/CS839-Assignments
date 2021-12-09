#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libpmemobj.h>

#include "pgm1.h"

static PMEMobjpool *pool;

TOID_DECLARE(struct my_hash, 0);

void printfunc(){
	TOID(struct my_hash) iter;
	POBJ_FOREACH_TYPE(pool, iter) {
		printf("Key %ld\tValue:%s\n", D_RO(iter)->key, D_RO(iter)->val);
	}
}

int getfunc(uint64_t key){
	int f = 0;
	TOID(struct my_hash) iter;
	
	POBJ_FOREACH_TYPE(pool, iter) {				
		if(D_RO(iter)->key == key){
			printf("Key %ld found\tValue:%s\n", D_RO(iter)->key, D_RO(iter)->val);
			f=1;			
			break;
		}				
	}

	if(!f){
		printf("Key %ld not found\n", atoi(key));
	}
	return f;
}

void putfunc(uint64_t key, char *val){
	int i, n, f = 0;
	n = strlen(val);
	TOID(struct my_hash) myhash, iter;
	struct pobj_action act[2];
	
	/*if(getfunc(key)){
		f = 1;
		printf("Key %d already existing, updating value!\n", key);
	}*/
	
	PMEMoid myhashoid = pmemobj_reserve(pool, act + 0, sizeof(struct my_hash), 0);
	TOID_ASSIGN(myhash, myhashoid);
	if (TOID_IS_NULL(myhash)){
		printf("Can't allocate hash: %ld\n", atoi(key));
	}
	
	/*POBJ_FOREACH_TYPE(pool, iter) {				
		if(D_RO(iter)->key == key){
			printf("Key %ld found\tValue:%s\n", D_RO(iter)->key, D_RO(iter)->val);
			f=1;			
			break;
		}				
	}*/

	if(f){
		for(i=0;i<n;i++){
			D_RW(myhash)->val[i]=val[i];
		}
		D_RW(myhash)->val[i]='\0';
		pmemobj_persist(pool, D_RW(myhash), sizeof(struct my_hash));
	}
	else{
		//TOID(struct my_hash) myhash = POBJ_RESERVE(pop, struct my_hash);
		D_RW(myhash)->key = key;
		for(i=0;i<n;i++){
			D_RW(myhash)->val[i]=val[i];
		}
		D_RW(myhash)->val[i]='\0';
		
		pmemobj_persist(pool, D_RW(myhash), sizeof(struct my_hash));
		pmemobj_publish(pool, act + 0, 1);
		//TX_PUBLISH(D_RW(root)->data, myhash);
	}
}

int main(int argc, char *argv[])
{
	char *ip,*ip1,*ip2;
	int i,n;
	struct pobj_action act;
	
	if(!(pool = pmemobj_create(FILE_NAME, LAYOUT_NAME, PMEMOBJ_MIN_POOL, 0666))){
		pool = pmemobj_open(FILE_NAME, LAYOUT_NAME);
		//printf("Opened existing pool!!\n");
	}

	TOID(struct my_hash) myhash;
	if(argc>1){
		ip = strdup(argv[1]);	
	}
	else{
		printf("Invalid command\n");
	}
	if(!strcmp(ip,"p")){
		if(argc != 4){
			printf("Invalid put command!\n");
		}
		else{
			ip1 = strdup(argv[2]);
			ip2 = strdup(argv[3]);

			putfunc(atoi(ip1), ip2);
			pmemobj_close(pool);
		}
	}
	else if(!strcmp(ip,"g")){
		if(argc != 3){
			printf("Invalid get command!\n");
		}
		else{
			ip1 = strdup(argv[2]);			
			
			getfunc(atoi(ip1));
			pmemobj_close(pool);
		}
	}
	else if(!strcmp(ip,"d")){
		if(argc != 2){
			printf("Invalid get command!\n");
		}
		else{
			printfunc();
			pmemobj_close(pool);
		}
	}
	else if(!strcmp(ip,"c")){
		int f=1/0;
	}
	else{
		printf("Invalid command!\n");
	}
}
