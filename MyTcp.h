#ifndef _MYTCP_H_
#define _MYTCP_H_

#include "MyMysql.h"
#include <iostream>
using namespace std;



class MyTcp
{
public:
	MyTcp(const char *ip,unsigned short port):pMysql(NULL)
	{
		mlistenfd = socket(AF_INET,SOCK_STREAM,0);
		if( mlistenfd == -1 )
		{
			cout<<"socket create failed..."<<endl;
			exit(0);
		}

		struct sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = inet_addr(ip);

		if(-1 == bind(mlistenfd,(struct sockaddr*)&server,sizeof(server)) )
		{
			cout<<"bind failed..."<<endl;
			exit(0);
		}

		if(-1 == listen(mlistenfd,20) )
		{
			cout<<"listen failed..."<<endl;
			exit(0);
		}
	}

	~MyTcp()
	{
		close(mlistenfd);
	}

	int GetListenfd() const
	{return mlistenfd;}

	void run()
	{
		char ip[24];
		unsigned short port;
		char user[24];
		char pwd[24];

		cout<<"start server ..."<<endl;
		cout<<"start connect Mysql..."<<endl;
		/*
		cout<<"please init Mysql.."<<endl;
		cout<<"ip:";
		cin.getline(ip,24);

		cout<<"port:";
		cin>>port;
		cin.get();

		cout<<"user:";
		cin.getline(user,24);

		cout<<"passwd:";
		cin.getline(pwd,24);
		*/
		strcpy(ip,"127.0.0.1");
		port = 3306;
		strcpy(user,"root");
		strcpy(pwd,"123456");
		
		pMysql = new MyMysql(ip,port,user,pwd);
		if(pMysql != NULL)
		{
			cout<<"Mysql start success..."<<endl;
		}
		else
		{
			cout<<"Mysql start failed..."<<endl;
		}

		cout<<"server start success..."<<endl;
	}

	bool DoRegister(const char* name,const char* pwd)
	{
		bool ret = pMysql->QueryRegister(name);
		if ( ret )
		{
			if( pMysql->AddValueToMysql(name,pwd) )
				return true;
		}
		return false;
	}

	bool DoLogin(const char* name,const char* pwd)
	{
		return pMysql->QueryLogin(name,pwd);
	}
	
private:
	int mlistenfd;
	MyMysql *pMysql;

	friend void client_cd(int fd, short event,void *arg);
};

#endif