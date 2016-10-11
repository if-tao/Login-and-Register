#ifndef _MYMEMCACHED_H_
#define _MYMEMCACHED_H_

#include <iostream>
#include <string>
using namespace std;
#include <libmemcached/memcached.h>

class MyMemcached
{
public:
	MyMemcached(const char *ip,unsigned short port)
	{
		//memcached_st *memcached_create(memcached_st *ptr);
		mpmemc = memcached_create(NULL);
		//memcached_server_list_st memcached_server_list_append(memcached_server_list_st ptr,const char *hostname,in_port_t port,memcached_return_t *error);
		mpservers = memcached_server_list_append(NULL,ip,port,&mrc);
		//memcached_return_t memcached_server_push(memcached_st *ptr, const memcached_server_list_st list);
		mrc = memcached_server_push(mpmemc,mpservers);
	}
	
	~MyMemcached()
	{
		//void memcached_free(memcached_st *ptr);
		memcached_free(mpmemc);
	}

	char *SearchValueByKey(const char*key)
	{
		//char *memcached_get(memcached_st *ptr,const char *key, size_t key_length,size_t *value_length,uint32_t *flags,memcached_return_t *error);
		size_t value_length = 0;
		memcached_return rc;
		uint32_t flags = 0;
		char *result = memcached_get(mpmemc,key,strlen(key),&value_length,&flags,&rc);
		if(rc == MEMCACHED_SUCCESS)
		{
			//命中
			mMemcachedSpotcount++;
			return result;
		}
		else
			return NULL;
	}

	void SaveValue(const char *key,const char *value)
	{
		//memcached_return_t memcached_set(memcached_st *ptr, const char *key, size_t key_length,const char *value, size_t value_length,time_t expiration,uint32_t  flags);
		//time_t 0 代表永不超时
		memcached_return_t rc;
		cout<<"key:"<<key<<strlen(key)<<"  value:"<<value<<strlen(value)<<endl;
		rc = memcached_set(mpmemc,key,strlen(key),value,strlen(value),0,0);
		if(rc != MEMCACHED_SUCCESS)
		{
			cout<<"save value failed..."<<endl;
		}
	}
private:
	memcached_st *mpmemc;
    memcached_return mrc;
    memcached_server_st *mpservers;

    static int mMemcachedSpotcount;
};

int MyMemcached::mMemcachedSpotcount = 0;

#endif