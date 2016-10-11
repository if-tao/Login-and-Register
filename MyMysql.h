#ifndef _MYMYSQL_H_
#define _MYMYSQL_H_


#include <iostream>
using namespace std;
#include <mysql/mysql.h>
#include "MyMemcached.h"

class MyMysql
{
public:
	MyMysql(const char *ip,unsigned short port,const char *user,const char *passwd):mpcon(NULL),mpres(NULL),mpMemcached(NULL)
	{
		//初始化数据库,返回创建的对象
		/*MYSQL * STDCALL
		mysql_init(MYSQL *mysql)*/
		mpcon = mysql_init((MYSQL*)0);
		if(mpcon == NULL)
		{
			cout<<"mysql_init failed"<<endl;
			exit(0);
		}

		//连接数据库
		/*MYSQL * STDCALL
		mysql_real_connect(MYSQL *mysql,const char *host, const char *user,
		   const char *passwd, const char *db,
		   uint port, const char *unix_socket,ulong client_flag)*/
		if( !mysql_real_connect(mpcon,ip,user,passwd,"User",port,NULL,0) )
		{
			cout<<"mysql_real_connect error or select db error"<<endl;
			exit(0);
		}

		//初始memcached client(mc)
		InitMemcachedClient();
	}

	~MyMysql()
	{
		//释放资源void STDCALLmysql_free_result(MYSQL_RES *result)
		if(mpres != NULL)
			mysql_free_result(mpres);
		//断开连接关闭数据库void STDCALL mysql_close(MYSQL *mysql)
		if(mpcon != NULL)
			mysql_close(mpcon);
	}

	void InitMemcachedClient()
	{
		char ip[24];
		unsigned short port;
		
		cout<<"please init Memcached ..."<<endl;
		cout<<"ip:";
		cin.getline(ip,24);
		cout<<"port:";
		cin>>port;
		cin.get();
		/*
		strcpy(ip,"127.0.0.1");
		port = 11211;
		*/
		mpMemcached = new MyMemcached(ip,port);
		if(mpMemcached == NULL)
		{
			cout<<"Memcached start failed"<<endl;
			exit(0);
		}
		else
		{
			cout<<"Memcached start success..."<<endl;
		}
	}

	bool QueryLogin(const char*name,const char *pwd)
	{
		//先在memcached中查询
		char *value = mpMemcached->SearchValueByKey(name);
		if(value != NULL)
		{
			if( strcmp(value,pwd) == 0)
				return true;
			else
				return false;
		}

		//从数据库中查
		char sql[128] = {0};
		const char *table = "user";
		sprintf(sql,"select * from %s where name='%s' and pwd='%s';",table,name,pwd);
		//int STDCALL mysql_real_query(MYSQL *mysql, const char *query, ulong length)
		//函数内调用了send_query...
		//int STDCALL mysql_send_query(MYSQL* mysql, const char* query, ulong length)
		if( !mysql_real_query(mpcon,sql,strlen(sql)) )
		{
			mMySqlQueryCount++;
			//先将结果存起来
			//MYSQL_RES * STDCALL mysql_store_result(MYSQL *mysql)
			mpres = mysql_store_result(mpcon);
			//MYSQL_ROW STDCALL mysql_fetch_row(MYSQL_RES *res)
			while ( mrow = mysql_fetch_row(mpres) )
			{
				if( strcmp(mrow[0],name) == 0 )
				{
					if( strcmp(mrow[1],pwd) == 0 )
					{
						mpMemcached->SaveValue(name,pwd);
						return true;
					}
				}
			}
		}
		return false;
	}

	bool QueryRegister(const char *key)
	{
		if( !mpMemcached->SearchValueByKey(key) )
		{
			char sql[128] = {0};
			const char *table = "user";
			sprintf(sql,"select * from %s where name='%s' ;",table,key);

			if( !mysql_real_query(mpcon,sql,strlen(sql)) )
			{
				mMySqlQueryCount++;
				mpres = mysql_store_result(mpcon);
				while ( mrow = mysql_fetch_row(mpres) )
				{
					if( strcmp(mrow[0],key) == 0 )
					{
						return false;
					}
				}
			}
			return true;
		}

		return false;
	}

	bool AddValueToMysql(const char *key,const char *value)
	{
		char sql[128] = {0};
		const char *table = "user";
		sprintf(sql,"insert into %s (name,pwd) values('%s','%s');",table,key,value);
		if( mysql_real_query(mpcon,sql,strlen(sql)) )
			return false;
		else
			return true;
	}
private:
	MYSQL *mpcon;
	MYSQL_RES *mpres;
	MYSQL_ROW mrow;

	MyMemcached *mpMemcached;
	static 	int mMySqlQueryCount;

	friend class MyTcp;
};

int MyMysql::mMySqlQueryCount = 0;

#endif