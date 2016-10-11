#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <event.h>
#include <json/json.h>
#include <iostream>
using namespace std;
#include "MyTcp.h"

struct event_base *base = NULL;
enum
{
	EN_ERROR,
    EN_LOGIN,
    EN_REGISTER,
    EN_LOGIN_SUCCESS,
    EN_LOGIN_ERROR,
    EN_REGISTER_SUCCESS,
    EN_REGISTER_ERROR
};

void client_cd(int fd, short event,void *arg)
{
	MyTcp *pMyTcp = (MyTcp*) arg;
	Json::Value root;
    Json::Reader reader;
	char buffer[1024] = {0};
	bool ret;

	int len = recv(fd,&buffer,1024,0);
	if(len <= 0)
	{
		close(fd);
		return ;
	}
	if( reader.parse(buffer, root) )
    {
        int type = root["type"].asInt();
        string namestr = root["name"].asString();
        char name[24];
        strcpy(name,namestr.c_str());
        string pwdstr = root["pwd"].asString();
        char pwd[24];
        strcpy(pwd,pwdstr.c_str());

        switch( type )
		{
			case EN_LOGIN:
			{
				ret = pMyTcp->DoLogin(name,pwd);
				if( ret )
					root["type"] = EN_LOGIN_SUCCESS;
				else
					root["type"] = EN_LOGIN_ERROR;
			}
			break;

			case EN_REGISTER:
			{
				ret = pMyTcp->DoRegister(name,pwd);
				if( ret )
					root["type"] = EN_REGISTER_SUCCESS;
				else
					root["type"] = EN_REGISTER_ERROR;	
			}
			break;
		}	
    }
    else
    {
		root["type"] = EN_ERROR;
	}

	string tmpstr = root.toStyledString();
    strcpy(buffer, tmpstr.c_str());
	send(fd,&buffer,sizeof(buffer),0);
}

void listen_cd(int fd , short event, void *arg)
{
	struct sockaddr_in client;
	socklen_t socklen = sizeof(client);

	int clientfd = accept(fd,(struct sockaddr*)&client, &socklen);
	if(clientfd == -1)
	{
		cout<<"TCP had died!!!"<<endl;
		close(fd);
		exit(0);
	}

	cout<<"client connected! \nip:"<<inet_ntoa(client.sin_addr)<<" port "<<ntohs(client.sin_port)<<endl;

	struct event* client_event = event_new(base,clientfd,EV_READ|EV_PERSIST,client_cd,arg);

	event_add(client_event,NULL);
}

int main(int argc,char *argv[])
{
	if(argc < 3)
	{
		cout<<"Required parameter missing.."<<endl;
		cout<<"./a.out  ip  port!!!"<<endl;
		exit(0);
	}

	unsigned short port = atoi(argv[2]);
	MyTcp server(argv[1],port);
	server.run();

	int listenfd = server.GetListenfd();
	base = event_init();
	//struct event *event_new(struct event_base *base, evutil_socket_t fd, short events, void (*cb)(evutil_socket_t, short, void *), void *arg)
	struct event* listen_event = event_new(base, listenfd,EV_READ|EV_PERSIST,listen_cd,&server);

	//int event_add(struct event *ev, const struct timeval *tv)
	event_add(listen_event,NULL);

	event_base_dispatch(base);

	event_free(listen_event);
	event_base_free(base);
	
	return 0;
}