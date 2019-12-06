
/*
 udp chat client

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>



struct  msg_chat
{
	 char msg_type;  
	 char from[30]; //消息来源
	 char to[30];  //消息目标用户
	 char content[128];
};

typedef struct  msg_chat msg_chat_t;

struct sockaddr_in  severaddr;
struct sockaddr_in  peeraddr;


int sendChatMsg(msg_chat_t *buf, struct sockaddr_in *addr);

int addrlen = sizeof(struct sockaddr_in);

int  consock = 0;

char buf[100] = {0};

char myName[50]={0};

char recv_buf[256] = {0};

/*
 ./chatcli serverip  serverport
*/

int main(int argc, char *argv[])
{

	int iret = 0;

	int recv_len = 0;
	int msg_len = sizeof(msg_chat_t);

	msg_chat_t  msg_buf ={0};

	int pid_recv = 0;

	consock = socket(AF_INET, SOCK_DGRAM,0);	  


	memset(&severaddr,0,sizeof(struct sockaddr_in));

	severaddr.sin_family = AF_INET;
	severaddr.sin_port = htons(atoi(argv[2]));
	severaddr.sin_addr.s_addr = inet_addr(argv[1]);

/*	iret = bind(consock,(struct sockaddr *)&severaddr, sizeof(struct sockaddr_in));
	if(iret < 0)
	{
		perror("bind error");
		return 0;
	}*/


  	printf("input login name:");
	fgets(buf,100,stdin);
	buf[strlen(buf)-1] = 0; //去掉回车符

	msg_buf.msg_type = 'L';
	strcpy(msg_buf.content, buf);

	strcpy(myName, buf);

	//发送用户登录消息
	sendChatMsg(&msg_buf, &severaddr); 

	//接收服务器发送的登录响应消息
	recv_len = recvfrom(consock,&recv_buf, 256,0,
						 (struct sockaddr *)&peeraddr, &addrlen);

	printf("recv_len:%d  %s\n", recv_len,recv_buf);

	if(strncmp(recv_buf,"success",7) != 0)
	{
		printf("login error!\n");
		return 0;
	}


	//登录成功,创建子进程接收服务器转发过来的聊天消息，主进程用来
	//发送自己输入的聊天消息

	int chatMode = 0; //0 公共聊天  1: 私聊
	char toUser[30]={"public"};

	pid_recv =  fork();
	if(pid_recv == 0)
	{//子进程接收聊天消息
		while(1)
		{
		  	recv_len = recvfrom(consock,&msg_buf, sizeof(msg_chat_t),0,
				 	          (struct sockaddr *)&peeraddr, &addrlen);
			printf("recv_len:%d  from:%s to:%s  content:%s\n", 
					recv_len,msg_buf.from,msg_buf.to,msg_buf.content);


			printf("%s say: %s\n",msg_buf.from,msg_buf.content);

				
		}

		exit(0);
	}

	while(1)
	{
		if(chatMode == 0)
		{
			printf("<public>:");
		}
		else if(chatMode == 1)
		{
			printf("<say to %s>:",toUser);
		}

		memset(buf,0,100);
	
		
		fgets(buf,100,stdin);

		if(strncmp(buf,"to:",3) == 0)
		{
			//切换聊天模式
			memset(toUser,0,30);
			strcpy(toUser, buf+3);
			toUser[strlen(toUser) - 1] = 0;

			if(strcmp(toUser,"public") == 0)
			{
				chatMode = 0;
			}
			else
			{
				chatMode = 1;
			}

			continue;
			
		}

		//聊天消息

		msg_buf.msg_type = 'C';
		
		strcpy(msg_buf.to,toUser);
		strcpy(msg_buf.from,myName);
		strcpy(msg_buf.content, buf);

		sendChatMsg(&msg_buf, &severaddr); 
	}
	


}



int sendChatMsg(msg_chat_t *buf, struct sockaddr_in *addr)
{
	int send_len = 0;
	int len = sizeof(msg_chat_t);
	send_len = sendto(consock,buf, len,0,
						 	          (struct sockaddr *)addr, sizeof(struct sockaddr_in));

	printf("send len:%d\n", send_len);

	return send_len;
}

