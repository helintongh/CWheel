

/*
 udp chat server

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


/*
 msg_type:

'L': login
'C': chat message
   
*/


//定义保存在线用户信息结构体
struct  login_info   
{
	 char name[50];
	 struct sockaddr_in addr;
};

typedef struct  login_info loginfo_t;




#define MAX_USER  100
loginfo_t  loginfo[MAX_USER];  //保存登录用户列表
int usernum = 0;  //保存在线用户人数


struct  msg_chat
{
	 char msg_type;  //消息类型
	 char from[30]; //消息来源
	 char to[30];  //消息目标用户
	 char content[128];  //聊天内容
};

typedef struct  msg_chat msg_chat_t;

int sendMsg(char *buf, int len, struct sockaddr_in *addr);
int transMsgToUser(msg_chat_t        *msg, char *userName);

int transMsgToAllUser(msg_chat_t        *msg);


int process_chat_msg(msg_chat_t  *msg_chat);

msg_chat_t  msg_buf;

//保存服务器地址
struct sockaddr_in  severaddr;

//定义保存客户端地址的变量
struct sockaddr_in  clientaddr;

int addrlen = sizeof(struct sockaddr_in);

int  consock = 0;

/*
  ./chatser  serverip serverport
  
*/

int main(int argc, char *argv[])
{

		int iret = 0;

		int recv_len = 0;
		int msg_len = sizeof(msg_chat_t);

		if(argc != 3)
		{
			printf("<usage: ./chats ServerIP Port> \n");

			return 0;
		}

    	consock = socket(AF_INET, SOCK_DGRAM,0);	 


		memset(&severaddr,0,sizeof(struct sockaddr_in));

		severaddr.sin_family = AF_INET;
		severaddr.sin_port = htons(atoi(argv[2]));
		severaddr.sin_addr.s_addr = inet_addr(argv[1]);

		iret = bind(consock,(struct sockaddr *)&severaddr, sizeof(struct sockaddr_in));
		if(iret < 0)
		{
			perror("bind error");
			return 0;
		}

		while(1)
		{
			 //UDP server
			 memset(&clientaddr,0,sizeof(struct sockaddr_in));
			 memset(&msg_buf,0,sizeof(msg_chat_t));
			 
		     recv_len = recvfrom(consock,&msg_buf, msg_len,0,
			 	          (struct sockaddr *)&clientaddr, &addrlen);

			 if(recv_len <= 0)
			 {
			 		printf("recvfrom %d error\n", recv_len);
					continue;
			 }	

			 printf("recvmsg type:%c  content:%s  from:%s to:%s\n",
			 msg_buf.msg_type, msg_buf.content, msg_buf.from,msg_buf.to);

			 process_chat_msg(&msg_buf);

		}
		return 0;
}



int process_chat_msg(msg_chat_t  *msg_chat)
{
	 loginfo_t loguser = {0};
	 char toUser[30]={0};
	 int slen = 0;
	 char sbuf[50] = {0};

	 char msg_type = msg_chat->msg_type;
	 strcpy(toUser,msg_chat->to);

	 printf("toUser:%s\n", toUser);

	 if(msg_type == 'L')
	 {   
	 	//处理用户登录消息
	 	strcpy(loguser.name, msg_chat->content);
		memcpy(&loguser.addr, &clientaddr,sizeof(struct sockaddr_in));

		
		memcpy(&loginfo[usernum],&loguser,sizeof(loginfo_t));
		usernum++;

		strcpy(sbuf, "success");
		slen = strlen(sbuf);

		sendMsg("success",slen,&clientaddr);

		printf("%s login success!\n", loguser.name);
	 	  
	 }
	 else if(msg_type == 'C')
	 {
	 	//处理用户聊天消息,根据私聊和公共聊天转发消息到相应的在线用户
	 	if(strcmp(toUser,"public") == 0)
	 	{
	 		//公共聊天 将消息转发到所有在线用户
	 		transMsgToAllUser(msg_chat);
	 		printf("broad msg ok!\n");
	 	}
	 	else
	 	{
	 		//私聊，将消息转发到 toUser 用户
	 		transMsgToUser(msg_chat, toUser);
	 		printf("transfer msg to %s ok!\n", toUser);
	 		
	 	}
	 	
	 	
	 
	 }
	
}


//往 addr地址发送 buf里的内容
int sendMsg(char *buf, int len,struct sockaddr_in *addr)
{
	int send_len = 0;
	send_len = sendto(consock,buf, len,0,
               (struct sockaddr *)addr, sizeof(struct sockaddr_in));

	return send_len;
}

//转发消息到指定用户
int transMsgToUser(msg_chat_t        *msg, char *userName)
{

	int i = 0;
	
	//在登录用户信息表中查找用户的地址信息
	for(i = 0; i < usernum; i++)
	{
		if(strcmp(loginfo[i].name,userName) == 0)
		{
			//成功找到了该用户
			break;
		}
	}

	if(i >= usernum)
	{
		printf("can not find user:%s\n", userName);
		return -1;
	}

	//发送消息给指定用户，完成私聊功能
	sendMsg((char *)msg, sizeof(msg_chat_t),&loginfo[i].addr);

	return 0;
}

//转发消息到所有用户
int transMsgToAllUser(msg_chat_t        *msg)
{
	int i = 0;
	
	//在登录用户信息表中查找用户的地址信息
	for(i = 0; i < usernum; i++)
	{
		sendMsg((char *)msg, sizeof(msg_chat_t),&loginfo[i].addr);
	}

	return 0;
}



