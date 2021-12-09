#include<stdio.h>
#include<libpmemobj.h>
#include<pthread.h>
#include <valgrind/pmemcheck.h>

struct my_root{
	int key,val;
};

POBJ_LAYOUT_BEGIN(test);
POBJ_LAYOUT_ROOT(test,struct my_root);
POBJ_LAYOUT_END(test);

int main(int argc, char *argv[]){
	PMEMobjpool *pop = pmemobj_open("test4",POBJ_LAYOUT_NAME(test));	
	if(pop==NULL){
		pop = pmemobj_create("test4",POBJ_LAYOUT_NAME(test),(1024*1024*100),0666);
	}

	TX_BEGIN(pop){
		TOID(struct my_root) rt=POBJ_ROOT(pop,struct my_root);
		VALGRIND_PMC_START_TX;
		// TX_ADD(rt);
		VALGRIND_PMC_ADD_TO_TX(&rt, sizeof(struct my_root));
		D_RW(rt)->key=1;
		D_RW(rt)->val=(D_RO(rt)->key)*(D_RO(rt)->key);
		VALGRIND_PMC_END_TX;
	}TX_END

	return 0;
}
