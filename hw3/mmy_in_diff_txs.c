#include<stdio.h>
#include<libpmemobj.h>

struct my_root{
	int key,val;
};

POBJ_LAYOUT_BEGIN(test);
POBJ_LAYOUT_ROOT(test,struct my_root);
POBJ_LAYOUT_END(test);

pthread_mutex_t lock;

void *thread_func(PMEMobjpool *pop){
	TX_BEGIN(pop){
		pthread_mutex_lock(&lock);
		TOID(struct my_root) rt=POBJ_ROOT(pop,struct my_root);
		TX_ADD(rt);
		D_RW(rt)->key=1;
		D_RW(rt)->val=(D_RO(rt)->key)*(D_RO(rt)->key);
		pthread_mutex_unlock(&lock);
	}TX_END
}

int main(int argc, char *argv[]){
	PMEMobjpool *pop = pmemobj_open("test1",POBJ_LAYOUT_NAME(test));	
	if(pop==NULL){
		pop = pmemobj_create("test1",POBJ_LAYOUT_NAME(test),(1024*1024*100),0666);
	}

	pthread_t thread;
	pthread_mutex_init(&lock,NULL);

	TX_BEGIN(pop){
		pthread_mutex_lock(&lock);
		TOID(struct my_root) rt=POBJ_ROOT(pop,struct my_root);
		TX_ADD(rt);
		pthread_create(&thread,NULL,thread_func,pop);
		D_RW(rt)->key=1;
		D_RW(rt)->val=(D_RO(rt)->key)*(D_RO(rt)->key);
		pthread_mutex_unlock(&lock);
		pthread_join(thread,NULL);
	}TX_END
	pthread_mutex_destroy(&lock);
	
	return 0;
}
