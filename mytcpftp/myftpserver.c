


/*
  myftp server
*/


#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>


/*
msg_type:

'L': 获取文件列表
'P': 上传文件命令        buf:保存文件名
'G': 下载文件命令        buf: 保存着文件名
'D': 数据报文      buf: 保存着文件数据
'E': 文件列表响应消息，buf里面保存文件列表名称
'R': 文件列表      buf 中保存文件目录信息
'X': 发送结束标志

'A': OK 响应消息
'N': NAK ,出错了, buf保存错误原因

*/


struct myftp_msg
{
	 char msg_type;
	 unsigned short buf_len;  
	 char buf[512];   
};

typedef struct myftp_msg  myftpMsg_t;

int process_myftpmsg(myftpMsg_t *msg);

int sendFtpMsg(myftpMsg_t *msg);

myftpMsg_t myFtpMsg; 


int consock = 0;  

FILE *fp_write = NULL;
FILE *fp_read = NULL;


int main(int argc,char *argv[])
{
	 int iret = 0;

	 int recv_len = 0;
	 int client_port = 0;
	
	 fd_set  rfdset; 
	 fd_set  rfdset_back; 
	 int  fd_max = 0; 


	 struct sockaddr_in serveraddr;
	 struct sockaddr_in clientaddr;
	 int addrlen = sizeof(struct sockaddr_in);

	 int reuse = 1;
	 int optlen = sizeof(int);


	 if(argc !=3)
	 {
	 	  printf("usage: %s  serverAddr Port\n", argv[0]);
		  return 0;
	 }


	 int listenSock = socket(AF_INET,SOCK_STREAM,0);


	 iret = setsockopt(listenSock, SOL_SOCKET,SO_REUSEADDR,&reuse,optlen);
	 if(iret < 0)
	 {
	 	 perror("setoptsock error\n");
		 return 0;
	 }


	 bzero(&serveraddr,sizeof(struct sockaddr_in));

	 serveraddr.sin_family = AF_INET;
	 serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	 serveraddr.sin_port = htons(atoi(argv[2]));
	 
	 //绑定socket
	 iret =bind(listenSock,(struct sockaddr *)&serveraddr,sizeof(struct sockaddr_in));
	 if(iret < 0)
	 {
	 	 perror("bind error");
		 return 0;
	 }

	 listen(listenSock,20);

	 printf("Listening....OK\n");


	 FD_ZERO(&rfdset);
	 FD_ZERO(&rfdset_back);
	 fd_max = listenSock; 


	 FD_SET(listenSock, &rfdset);
	 FD_SET(listenSock, &rfdset_back);

	 while(1)
	 {			
 		rfdset = rfdset_back; 
 		iret = select(fd_max+1,&rfdset,NULL,NULL,NULL);
		if(iret < 0)
		{
			perror("select error\n");
			continue;
		}

		if(FD_ISSET(listenSock, &rfdset))
		{
			
		 	consock = accept(listenSock, (struct sockaddr *)&clientaddr, &addrlen);
			FD_SET(consock, &rfdset_back);

			if(consock > fd_max )
			{
				fd_max = consock;
			}
				
		}

			
		if(FD_ISSET(consock, &rfdset))
		{   
			
			 recv_len = recv(consock, &myFtpMsg,sizeof(myftpMsg_t),0);

			// printf("recv:%d %c  %s\n",recv_len, myFtpMsg.msg_type,myFtpMsg.buf);

			 process_myftpmsg(&myFtpMsg);			 
		}
					
				
	 }

	 return 0;
}


// 处理上传文件命令函数
int ftp_upload_file(myftpMsg_t *msg)
{
	char filename[50]={0};

	strcpy(filename, msg->buf);
	fp_write = fopen(filename,"wb");

	if(fp_write == NULL)
	{
		 perror("file open fail");
		 
		 return -1;
	}

		
	myFtpMsg.msg_type ='A';
	
	sendFtpMsg(&myFtpMsg);

	return 0;
			 
		
}


//处理下载文件命令
int ftp_download_file(myftpMsg_t *msg)
{
	char filename[50]={0};

	strcpy(filename, msg->buf);
	fp_read = fopen(filename,"rb");

	if(fp_read == NULL)
	{
		 perror("file open fail");
		 
		 return -1;
	}
		
	myFtpMsg.msg_type ='A';
	sendFtpMsg(&myFtpMsg);

	int data_len = 0;
	int r_len = 0;

	usleep(10000);

	//开始发送文件数据
	do
	{
		r_len = fread(myFtpMsg.buf,1,512, fp_read);
		myFtpMsg.buf_len = r_len;
		myFtpMsg.msg_type = 'D';
		sendFtpMsg(&myFtpMsg);
		printf("download data:%d\n", r_len);
		usleep(1000);
	
	}while(r_len >= 512);

	//download file ok

	fclose(fp_read);

	printf("download file ok\n");
	

	return 0;
}


//处理下载目录列表

int ftp_download_filelist(myftpMsg_t *msg)
{
	//服务器每次向客户端发送一个目录列表

	DIR  *myDir = NULL;  //定义目录指针
	struct dirent *myItem=NULL;  //定义目录条目指针

	myDir = opendir(".");   //打开当前目录

	do
	{
		memset(&myFtpMsg,0,sizeof(myftpMsg_t));
		myItem = readdir(myDir); //读取目录条目
		if(myItem != NULL)
		{
			//成功读取目录条目
			if((strcmp(myItem->d_name,".") == 0)||
			   (strcmp(myItem->d_name,"..") == 0))
			{
				continue;  //跳过当前目录和父母录
			}

			if(myItem->d_type & DT_DIR)
			{
			    //如果是目录
			    sprintf(myFtpMsg.buf,"directory --- %s", myItem->d_name);
			 
			}
			else
			{
				sprintf(myFtpMsg.buf,"file --- %s", myItem->d_name);
			}

			myFtpMsg.buf_len = strlen(myFtpMsg.buf);
			myFtpMsg.msg_type = 'R';
			
			sendFtpMsg(&myFtpMsg);

			usleep(1000);
		}	
	}while(myItem !=NULL);

	//发送结束标志
	myFtpMsg.msg_type = 'X';
			
	sendFtpMsg(&myFtpMsg);
	
	
	return 0;
}



int process_myftpmsg(myftpMsg_t *msg)
{

	int data_len = 0;
	int w_len = 0;
	
	if(msg->msg_type == 'P')
	{
		//上传文件命令
		ftp_upload_file(msg); // 准备接收客户端发过来的文件数据
	}
  	else if(msg->msg_type == 'D')
 	{
 		//接收到数据报文
		data_len = ntohs(msg->buf_len);
		w_len = fwrite(msg->buf,1,data_len, fp_write);
		printf("upload data:%d\n", w_len);
		if(w_len < 512)
		{
			fclose(fp_write);
			printf("upload success\n");
		}
		  
  	}
  	else if(msg->msg_type == 'G')
  	{
  		//下载文件命令
  		ftp_download_file(msg); //下载文件
  	}
  	else if(msg->msg_type == 'L')
  	{
  		//下载目录列表
  		ftp_download_filelist(msg);
  	}
	
	return 0;
}


int sendFtpMsg(myftpMsg_t *msg)
{
	int send_len = 0;
	send_len = send(consock,msg,sizeof(myftpMsg_t),0);

	return 0;
}





