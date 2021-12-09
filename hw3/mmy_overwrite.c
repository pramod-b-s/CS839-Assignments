#include<stdio.h>
#include<emmintrin.h>
#include<stdint.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<valgrind/pmemcheck.h>
#include<libpmem.h>

void flush(const void *addr, size_t len){
	uintptr_t flushalign=64,uptr;

	for(uptr=(uintptr_t)addr & ~(flushalign-1); uptr<(uintptr_t)addr+len; uptr+=flushalign){
		_mm_clflush((char*)uptr);
	}
}

int main(int argc,char *argv[]){
	int fp,is_pmem;
	size_t mapped_len;
	char *val;

	if((val = pmem_map_file("test2",4096,PMEM_FILE_CREATE|PMEM_FILE_EXCL,0666,&mapped_len,&is_pmem)) 
		== NULL) {
		perror("pmem_map_file");
		exit(1);
	}

	VALGRIND_PMC_REGISTER_PMEM_MAPPING(val,sizeof(char));
	memcpy(val,"abc",strlen("abc"));
	memcpy(val,"xyz",strlen("xyz"));
	flush((void*)val,sizeof(char));
	munmap(val,4096);
	VALGRIND_PMC_REMOVE_PMEM_MAPPING(val,sizeof(char));	

	return 0;
}