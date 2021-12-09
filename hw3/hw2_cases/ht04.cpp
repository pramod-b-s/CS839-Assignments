#include <unistd.h>
#include <libpmemobj.h>
#include <libpmemobj++/transaction.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/container/string.hpp>

#include<valgrind/pmemcheck.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>

#define PATH "myhash"
#define TOLERANCE_LIMIT 21
#define DEF_CAP 128
#define MAX_CAP 1024

static int hash(int key,int n){
	return key%n;
}

using string_type = pmem::obj::string;
typedef struct hash_entry {
	void initentry(int k) {
		key = k;
		isval = 0;
	}

	pmem::obj::p<uint64_t> key;
	pmem::obj::p<int> isval;
	pmem::obj::persistent_ptr<pmem::obj::string> value;
} hash_entry;

using vector_type = pmem::obj::vector<hash_entry>;
typedef struct hash_table {
	hash_table(int capacity1, int length1) {
		capacity = capacity1;
		length = length1;
	}

	void display() {
	}

	pmem::obj::p<int> capacity;
	pmem::obj::p<int> length;
	pmem::obj::persistent_ptr<vector_type> entries;
} hash_table;

struct root {
	pmem::obj::persistent_ptr<hash_table> hashtable;
	pmem::obj::persistent_ptr<hash_table> hashtable1;
};

void createhtables(pmem::obj::pool<root> pop, int initcapacity){
	auto r1 = pop.root();
	hash_entry hentry;
	int i;
	std::cout<<"Hash table creation start!\n";
	pmem::obj::transaction::run(pop, [&]{		
		r1->hashtable = pmem::obj::make_persistent<hash_table>(initcapacity,0);
		r1->hashtable1 = pmem::obj::make_persistent<hash_table>(initcapacity,0);
	});
	pmem::obj::transaction::run(pop, [&]{		
		(r1->hashtable)->entries = pmem::obj::make_persistent<vector_type>();
		vector_type &pvector = *((r1->hashtable)->entries);					
		char t[4];
		strcpy(t,"--");
		for(i=0;i<initcapacity;i++){				
			hentry.initentry(-1);
			auto t1 = pmem::obj::make_persistent<string_type>(t, strlen(t));
			pvector.push_back(hentry);
			pvector[i].value=t1;				
		}		
	});

	pmem::obj::transaction::run(pop, [&]{
		pmem::obj::persistent_ptr<vector_type> entriesnp;
		//TEST04
		VALGRIND_PMC_REGISTER_PMEM_MAPPING(&((r1->hashtable1)->entries), sizeof(vector_type));		
		(r1->hashtable1)->entries = entriesnp;
		// (r1->hashtable1)->entries = pmem::obj::make_persistent<vector_type>();
		vector_type &pvector = *((r1->hashtable1)->entries);
		char t[4];
		strcpy(t,"--");		
		for(i=0;i<initcapacity;i++){				
			hentry.initentry(-1);
			auto t1 = pmem::obj::make_persistent<string_type>(t, strlen(t));			
			pvector.push_back(hentry);
			pvector[i].value=t1;
		}
		VALGRIND_PMC_REMOVE_PMEM_MAPPING(&((r1->hashtable1)->entries), sizeof(vector_type));
	});
	std::cout<<"Created hash tables\n";
}

void get(pmem::obj::pool<root> pop){
	int htnum,key1,n,i,cnt,f;	
	pmem::obj::persistent_ptr<hash_table> htable;
	std::string str;
	auto r = pop.root();

	std::cout<<"enter ht number, key\t";
	std::cin>>htnum>>key1;
	if(htnum==0){htable=(r->hashtable);}
	else if(htnum==1){htable=(r->hashtable1);}
	else{std::cout<<"invalid hashtable\n"; return;}
	pmem::obj::transaction::run(pop, [&]{
		n=htable->capacity;				
		vector_type &pvector = *(htable->entries);
		i=hash(key1,n);
		cnt=0;
		f=0;
		while(cnt<n && pvector[i].isval!=0){
			if(pvector[i].key==key1){
				str = std::string(pvector[i].value->c_str(),pvector[i].value->size());
				f=1;
				break;
			}
			i=(i+1)%n; cnt++;
		}
		if(f){
			std::cout<<" Key: "<<pvector[i].key<<" Value: "<<str<<"\n";										
		}
		else{
			std::cout<<"key not found!\n";
		}
	});
}

void put(pmem::obj::pool<root> pop){
	int htnum,key1,n,i,cnt,f;	
	pmem::obj::persistent_ptr<hash_table> htable;
	std::string str;
	auto r = pop.root();

	std::cout<<"enter ht num\t";
	std::cin>>htnum;
	if(htnum==0){htable=(r->hashtable);}
	else if(htnum==1){htable=(r->hashtable1);}
	else{std::cout<<"invalid hashtable\n"; return;}

	if(htable->length+TOLERANCE_LIMIT>htable->capacity){
		std::cout<<"Hash table doesn't have enough capacity, resize first\n";
		return;
	}			
	char t[1000];
	std::cout<<"enter key value\t";
	std::cin>>key1>>t;
	pmem::obj::transaction::run(pop, [&]{
		n=htable->capacity;				
		vector_type &pvector = *(htable->entries);
		i=hash(key1,n);	
		cnt=0;
		f=0;
		while(cnt<n && pvector[i].isval!=0){
			if(pvector[i].key==key1){
				auto t1 = pmem::obj::make_persistent<string_type>(t, strlen(t));
				pvector[i].value=t1;
				f=1;
				break;
			}
			i=(i+1)%n; cnt++;
		}

		if(!f){
			pvector[i].isval=1;
			pvector[i].key=key1;
			auto t1 = pmem::obj::make_persistent<string_type>(t, strlen(t));
			pvector[i].value=t1;
			htable->length++;
		}
	});
}

void print(pmem::obj::pool<root> pop){	
	int htnum,i;
	std::string str;
	pmem::obj::persistent_ptr<hash_table> htable;
	auto r = pop.root();

	std::cout<<"enter ht num\t";
	std::cin>>htnum;
	if(htnum==0){htable=(r->hashtable);}
	else if(htnum==1){htable=(r->hashtable1);}
	else{std::cout<<"invalid hashtable\n"; return;}

	std::cout<<"Capacity: "<<htable->capacity<<" Length: "<<htable->length<<"\n";
	vector_type &pvector = *(htable->entries);
	for(i=0;i<pvector.size();i++){
		if(pvector[i].isval){
			str = std::string(pvector[i].value->c_str(),pvector[i].value->size());
			std::cout<<"Key: "<<pvector[i].key<<" Value: "<<str<<"\t";			
		}
	}
}

void mov(pmem::obj::pool<root> pop){
	int ch,i,j,n,key1,cnt,f,f1,htnum,htnum2,newsize;
	std::string val,str;
	hash_entry hentry;
	pmem::obj::persistent_ptr<hash_table> htable,htable1;

	auto r = pop.root();

	std::cout<<"enter src dest ht num\t";
	std::cin>>htnum>>htnum2;
	if(htnum==htnum2){std::cout<<"src and dest table num must differ\n"; return;}
	if(htnum==0){htable=(r->hashtable);}
	else if(htnum==1){htable=(r->hashtable1);}
	else{std::cout<<"invalid hashtable\n"; return;}
	if(htnum2==0){htable1=(r->hashtable);}
	else if(htnum2==1){htable1=(r->hashtable1);}
	else{std::cout<<"invalid hashtable\n"; return;}

	std::cout<<"enter key\t";
	std::cin>>key1;
	pmem::obj::transaction::run(pop, [&]{
		n=htable->capacity;				
		vector_type &pvector = *(htable->entries);
		i=hash(key1,n);
		cnt=0;
		f=0;
		f1=0;
		while(cnt<n && pvector[i].isval!=0){
			if(pvector[i].key==key1){
				str = std::string(pvector[i].value->c_str(),pvector[i].value->size());
				f=1;
				break;
			}
			i=(i+1)%n; cnt++;
		}
		if(f){
			std::cout<<" Key: "<<pvector[i].key<<" Value: "<<str<<"\n";	
			htable->length--;
			pvector[i].isval=0;									
		}
		else{
			std::cout<<"key not found!\n";
			f1=1;
		}

		if(f1){
			std::cout<<"Key not found, no movement\n";
		}
		else{
			auto tmpvector=pvector[i];
			key1=tmpvector.key;
			pmem::obj::transaction::run(pop, [&]{
				n=htable1->capacity;				
				vector_type &pvector1 = *(htable1->entries);
				i=hash(key1,n);	
				cnt=0;
				f=0;
				while(cnt<n && pvector1[i].isval!=0){
					if(pvector1[i].key==key1){
						pvector1[i].value=tmpvector.value;
						f=1;
						break;
					}
					i=(i+1)%n; cnt++;
				}

				if(!f){
					pvector1[i]=tmpvector;
					pvector1[i].isval=1;
					htable1->length++;
				}
			});
		}
	});
}

void dbg(pmem::obj::pool<root> pop){	
	int htnum,i;
	std::string str;
	pmem::obj::persistent_ptr<hash_table> htable;
	auto r = pop.root();

	std::cout<<"enter ht num\t";
	std::cin>>htnum;
	if(htnum==0){htable=(r->hashtable);}
	else if(htnum==1){htable=(r->hashtable1);}
	else{std::cout<<"invalid hashtable\n"; return;}

	std::cout<<"Capacity: "<<htable->capacity<<" Length: "<<htable->length<<"\n";
	vector_type &pvector = *(htable->entries);
	for(i=0;i<pvector.size();i++){				
		if(i%4==0){std::cout<<"\n";}
		str = std::string(pvector[i].value->c_str(),pvector[i].value->size());
		std::cout<<"IsValid: "<<pvector[i].isval<<" Key: "<<pvector[i].key<<" Value: "<<str<<"\t";				
	}
}

void resize(pmem::obj::pool<root> pop){
	int ch,i,j,n,key1,cnt,f,f1,htnum,htnum2,newsize;
	std::string val,str;
	hash_entry hentry;
	pmem::obj::persistent_ptr<hash_table> htable,htable1;

	auto r = pop.root();

	std::cout<<"enter ht num to resize\t";
	std::cin>>htnum;
	if(htnum==0){htable=(r->hashtable);}
	else if(htnum==1){htable=(r->hashtable1);}
	else{std::cout<<"invalid hashtable\n"; return;}
	std::cout<<"enter new size\t";
	std::cin>>newsize;
	if(newsize<htable->capacity){std::cout<<"New size must be greater than old size\n"; return;}
	if(newsize>MAX_CAP){std::cout<<"New size must be less than max capacity\n"; return;}

	pmem::obj::transaction::run(pop, [&]{
		htable1 = pmem::obj::make_persistent<hash_table>(newsize,0);

		htable1->entries = pmem::obj::make_persistent<vector_type>();
		vector_type &pvector1 = *(htable1->entries);
		char t[4];
		strcpy(t,"--");				
		for(i=0;i<newsize;i++){				
			hentry.initentry(-1);
			auto t1 = pmem::obj::make_persistent<string_type>(t, strlen(t));								
			pvector1.push_back(hentry);
			pvector1[i].value=t1;
		}

		htable1->length = htable->length;
		vector_type &pvector = *(htable->entries);
		n=htable1->capacity;

		for(i=0;i<pvector.size();i++){
			if(pvector[i].isval){
				j=hash(pvector[i].key,n);
				cnt=0;
				f=0;
				while(cnt<n && pvector1[j].isval!=0){
					j=(j+1)%n; cnt++;
				}

				pvector1[j].isval=1;
				pvector1[j].key=pvector[i].key;
				pvector1[j].value=pvector[i].value;
			}
		}

		if(htnum==0){pmem::obj::delete_persistent<hash_table>(r->hashtable); r->hashtable=htable1;}
		else if(htnum==1){pmem::obj::delete_persistent<hash_table>(r->hashtable1); r->hashtable1=htable1;}
	});
}

int main(int argc,char *argv[]){
	pmem::obj::pool<root> pop;
	int ch,i,j,n,key1,cnt,f,f1,htnum,htnum2,newsize;
	std::string val,str;
	hash_entry hentry;
	pmem::obj::persistent_ptr<hash_table> htable,htable1;

	if (access(PATH, F_OK) != 0) {
		pop = pmem::obj::pool<root>::create(PATH, "some_layout", PMEMOBJ_MIN_POOL);		
		
		createhtables(pop, DEF_CAP);
	}
	else {
		pop = pmem::obj::pool<root>::open(PATH, "some_layout");
	}

	auto r = pop.root();

	while(1){
		std::cout<<"\n1-Get k,v\t2-Put k,v\t3-Print\t4-Move between hashtables\t5-Debug\t6-Resize\t7-Quit\n";
		std::cin>>ch;

		if(ch==1){			
			get(pop);
		}
		else if(ch==2){			
			put(pop);
		}
		else if(ch==3){
			print(pop);
		}
		else if(ch==4){			
			mov(pop);
		}
		else if(ch==5){
			dbg(pop);
		}
		else if(ch==6){
			resize(pop);
		}
		else if(ch==7){
			break;
		}
		else{
			std::cout<<"error\n";
		}
	}

	return 1;	
}