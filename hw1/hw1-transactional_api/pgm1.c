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
	printf("Key\tValue\n");
	POBJ_FOREACH_TYPE(pop, iter) {
		printf("%ld\t:%s\n", D_RO(iter)->key, D_RO(iter)->val);
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

void putfunc(PMEMobjpool *pop, char *key, char *val){
	int i, n, f = 0;
	n = strlen(val);
	TOID(struct my_hash) myhash, iter;
	TOID(struct my_hash) root = POBJ_ROOT(pop, struct my_hash);
	
	POBJ_FOREACH_TYPE(pop, iter) {				
		if(D_RO(iter)->key == atoi(key)){
			f=1;
			break;
		}				
	}

	TX_BEGIN(pop) {
		TX_ADD(root);
		if(!f){
			myhash = TX_NEW(struct my_hash);
		}
		else{
			myhash = iter;
		}	 
		D_RW(myhash)->key = atoi(key);
		for(i=0;i<n;i++){D_RW(myhash)->val[i]=val[i];}
		D_RW(myhash)->val[i]='\0';
	} TX_END
}

int main(int argc, char *argv[])
{
	char *ip,*ip1,*ip2;
	int i,n;
	PMEMobjpool *pop;

	pop = pmemobj_open(FILE_NAME, LAYOUT_NAME);	
	if(pop == NULL){
		pop = pmemobj_create(FILE_NAME, LAYOUT_NAME, PMEMOBJ_MIN_POOL, 0666);					
	}

	if(argc > 1){
		ip = strdup(argv[1]);
	}
	else{
		printf("Invalid command\n");
		exit(1);
	}
	
	if(!strcmp(ip,"p")){
		if(argc != 4){
			printf("Invalid put command!\n");
		}
		else{
			ip1 = strdup(argv[2]);
			ip2 = strdup(argv[3]);
			putfunc(pop, ip1, ip2);

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
			printf("Invalid display command!\n");
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
