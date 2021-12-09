#include <stdio.h>
#include <string.h>
#include <libpmemobj.h>

#include "pgm1.h"

POBJ_LAYOUT_BEGIN(hashcalc);
POBJ_LAYOUT_ROOT(hashcalc, struct my_root);
POBJ_LAYOUT_TOID(hashcalc, struct my_hash);
POBJ_LAYOUT_END(hashcalc);

void printfunc(PMEMobjpool *pop){
	TOID(struct my_hash) iter;
	POBJ_FOREACH_TYPE(pop, iter) {
		printf("Key %ld\tValue:%s\n", D_RO(iter)->key, D_RO(iter)->val);
	}
}

int getfunc(PMEMobjpool *pop, char *key){
	int f = 0;

	TOID(struct my_hash) iter;
	POBJ_FOREACH_TYPE(pop, iter) {				
		if(D_RO(iter)->key == atoi(key)){
			printf("Key %ld found\tValue:%s\n", D_RO(iter)->key, D_RO(iter)->val);
			f=1;
		}				
	}

	if(!f){
		printf("Key %ld not found\n", atoi(key));
	}
	return f;
}

PMEMoid putfunc(uint64_t key, char *val){
	int i, n, f = 0;
	n = strlen(val) + 1;
	if(getfunc(pop, key)){
		f = 1;
	}

	struct pobj_action act[2];
	PMEMoid str = pmemobj_reserve("myhash1", act + 0, n, 1);
	pmemobj_memcpy("myhash1", pmemobj_direct(str), val, n, PMEMOBJ_F_MEM_NODRAIN);
	TOID(struct my_hash) myhash;
	PMEMoid myhashoid = pmemobj_reserve("myhash1", act + 1, sizeof(struct my_hash), 1);
	TOID_ASSIGN(myhash, myhashoid);
	D_RW(myhash)->val = str;
	D_RW(myhash)->key = key;
	pmemobj_persist("myhash1", D_RW(myhash), sizeof(struct my_hash));
	pmemobj_publish("myhash1", act, 2);
}

int main(int argc, char *argv[])
{
	char *ip,*ip1,*ip2;
	int i,n;
	
	PMEMobjpool *pop1;
	PMEMobjpool *pop;

	ip = strdup(argv[1]);
	pop1 = pmemobj_open("myhash1", LAYOUT_NAME);	
	if(pop1 == NULL){
		pop = pmemobj_create("myhash1", LAYOUT_NAME, PMEMOBJ_MIN_POOL, 0666);					
	}
	else{
		pop=pop1;
	}

	if(!strcmp(ip,"p")){
		if(argc != 4){
			printf("Invalid put command!\n");
		}
		else{
			ip1 = strdup(argv[2]);
			ip2 = strdup(argv[3]);
			putfunc(atoi(ip1), ip2);

			pmemobj_close(pop);
		}
	}
	else if(!strcmp(ip,"g")){
		if(argc != 3){
			printf("Invalid get command!\n");
		}
		else{
			ip1 = strdup(argv[2]);			
			getfunc(pop, ip1);
			pmemobj_close(pop);
		}
	}
	else if(!strcmp(ip,"d")){
		if(argc != 2){
			printf("Invalid get command!\n");
		}
		else{
			printfunc(pop);
			pmemobj_close(pop);
		}
	}
	else if(!strcmp(ip,"c")){
		int f=1/0;
	}
	else{
		printf("Invalid command!\n");
	}
}
