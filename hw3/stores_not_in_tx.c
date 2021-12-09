#include<stdio.h>
#include<libpmemobj.h>
#include<pthread.h>

struct my_root{
	int key,val;
};

POBJ_LAYOUT_BEGIN(test);
POBJ_LAYOUT_ROOT(test,struct my_root);
POBJ_LAYOUT_END(test);

int main(int argc, char *argv[]){
	PMEMobjpool *pop = pmemobj_open("test2",POBJ_LAYOUT_NAME(test));	
	if(pop==NULL){
		pop = pmemobj_create("test2",POBJ_LAYOUT_NAME(test),(1024*1024*100),0666);
	}

	TX_BEGIN(pop){
		TOID(struct my_root) rt=POBJ_ROOT(pop,struct my_root);
		TX_ADD_FIELD(rt,val);
		// TX_ADD(rt);
		D_RW(rt)->key=1;
		D_RW(rt)->val=(D_RO(rt)->key)*(D_RO(rt)->key);
	}TX_END

	return 0;
}
