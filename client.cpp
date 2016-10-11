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

int main()
{
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1)
    {
        cout<<"socket create error"<<endl;
        exit(0);
    }
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(7000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    connect(clientfd, (struct sockaddr*)&server, sizeof(server));
    
    int choice = 0;
    Json::Value root;
    Json::Reader reader;
    char buffer[1024] = {0};
    while(true)
    {
        cout<<"---------------"<<endl;
        cout<<"1.register"<<endl;
        cout<<"2.login"<<endl;
        cout<<"3.exit"<<endl;
        cout<<"---------------"<<endl;
        cout<<"choice:";
        cin>>choice;
        cin.get();
        
        switch(choice)
        {
        case 1:
            {
                root["type"] = EN_REGISTER;
                cout<<"username:";
                cin.getline(buffer, 24);
				root["name"] = buffer;
                
                cout<<"password:";
                cin.getline(buffer, 24);
                root["pwd"] = buffer;

                string tmpstr = root.toStyledString();
                strcpy(buffer, tmpstr.c_str());
                
                send(clientfd, &buffer, sizeof(buffer), 0);
                
                recv(clientfd, &buffer, sizeof(buffer), 0);
                if( reader.parse(buffer, root) )
                {
	                int type = root["type"].asInt();
	                if ( type == EN_REGISTER_SUCCESS ) 
                    	cout<<"注册成功，请登录!"<<endl;
                    else if (type == EN_REGISTER_ERROR)
                    	cout<<"用户名已注册!"<<endl;
                    else
                    	cout<<"服务器出错，返回非法消息!"<<endl;
                }
            }
            break;
        case 2:
	        {
                root["type"] = EN_LOGIN;
                cout<<"username:";
                cin.getline(buffer, 24);
				root["name"] = buffer;
                
                cout<<"password:";
                cin.getline(buffer, 24);
                root["pwd"] = buffer;

                string tmpstr = root.toStyledString();
                strcpy(buffer, tmpstr.c_str());
                
                send(clientfd, &buffer, sizeof(buffer), 0);
                
                recv(clientfd, &buffer, sizeof(buffer), 0);

                if( reader.parse(buffer, root) )
                {
	                int type = root["type"].asInt();
	                if ( type == EN_LOGIN_SUCCESS ) 
                    	cout<<"登陆成功!"<<endl;
                    else if (type == EN_LOGIN_ERROR)
                    	cout<<"用户名不存在或者密码错误!"<<endl;
                    else
                    	cout<<"服务器出错，返回非法消息!"<<endl;
                }
            }
            break;
        case 3:
	        {
		        cout<<"bye!"<<endl;
		        exit(0);
	        }
            break;
        }
    }
    
    return 0;
}

