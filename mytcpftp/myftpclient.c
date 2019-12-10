

/*

myftpclient  
*/

#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>


struct myftp_msg
{
	 char msg_type;
	 unsigned short buf_len;  
	 char buf[512];   
};

typedef struct myftp_msg  myftpMsg_t;


myftpMsg_t  myftpMsg;
myftpMsg_t  recvftpMsg;

char usrcmd[50]={0};

char ftpcmd[30] = {0};
char filename[50]={0};

int sockfd = 0;

void show_menu(void);

int  sendFtpMsg(myftpMsg_t *msg);
int  myftp_upload_file(char *upfilename);
int  myftp_download_file(char *downfilename);
int  myftp_download_filelist(void);


int recvFtpMsg(myftpMsg_t *msg);


int split_usrcmd(char *buf)
{
	  memset(ftpcmd, 0,30);
		memset(filename, 0,50);

		int i = 0;
		
	  char *p= buf;

		while(*p == ' ')
		{
			p++;
		}

		while( (*p != ' ')&&(*p != '\0')&&(*p != '\n'))
		{
			 ftpcmd[i] = *p;
			 i++;
			 p++;
		}

		while(*p == ' ')
		{
			p++;
		}

		i = 0;

		while( (*p != ' ')&&(*p != '\0')&&(*p != '\n'))
		{
			 filename[i] = *p;
			 i++;
			 p++;
		}

		return 0;
		
}

int main(int argc,char *argv[])
{
	
	struct sockaddr_in servaddr;

	if(argc !=3)
	{
		printf("usage: %s serverIP  serverPort \n",argv[0]);

		return 0;
	}


	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd  < 0)
	{
		printf("sockfd fail\n");

		return 0;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(atoi(argv[2]));
	servaddr.sin_addr.s_addr=inet_addr(argv[1]);
	
	if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
	{
		printf("connect fail\n");

		return 0;
	}

	printf("connect ftp server ok!!!!\n");


	while(1)
	{
		show_menu();

		fgets(usrcmd,50,stdin);

		printf("cmd: %s\n", usrcmd);

		split_usrcmd(usrcmd);

		printf("ftpcmd: %s  filenmae:%s\n", ftpcmd, filename);


	    if(strcmp(ftpcmd,"put") == 0)
	    {
			myftpMsg.msg_type = 'P';

			strcpy(myftpMsg.buf,filename);

			myftpMsg.buf_len = htons(strlen(filename));

			sendFtpMsg(&myftpMsg);
			recvFtpMsg(&recvftpMsg);
			if(recvftpMsg.msg_type != 'A')
			{
				  continue;
			}

			myftp_upload_file(filename);
	    }
	    else if(strcmp(ftpcmd,"get") == 0)
	    {
	    	//从服务器下载软件
	    	myftpMsg.msg_type = 'G';

			strcpy(myftpMsg.buf,filename);

			myftpMsg.buf_len = htons(strlen(filename));

			sendFtpMsg(&myftpMsg);
			recvFtpMsg(&recvftpMsg);
			if(recvftpMsg.msg_type != 'A')
			{
				  continue;
			}

			//开始下载文件
			myftp_download_file(filename);
			
	    }
	    else if(strcmp(ftpcmd,"list") == 0)
	    {
	        //从服务器下载目录列表
	        //从服务器下载软件

	        memset(&myftpMsg,0, sizeof(myftpMsg_t));
	    	myftpMsg.msg_type = 'L';
			sendFtpMsg(&myftpMsg);

			//接收并显示服务器文件列表
			myftp_download_filelist();
			
	        
	    }
	    
	
	}

	
	close(sockfd);


	return 0;
	
}


//下载并显示文件列表
int  myftp_download_filelist(void)
{
	do
	{
		memset(&recvftpMsg,0,sizeof(myftpMsg_t));
		recvFtpMsg(&recvftpMsg);
		if(recvftpMsg.msg_type == 'R')
		{
			printf("%s\n", recvftpMsg.buf);
		}

		
	}while(recvftpMsg.msg_type == 'R');

	return 0;
}


//下载文件
int  myftp_download_file(char *downfilename)
{

	FILE *fp_write = NULL;

	int write_len = 0;

	fp_write = fopen(downfilename, "wb");

	if(fp_write == NULL) 
	{
		perror("file open error");
	  	return -1;
	}
	
	do
	{
		recvFtpMsg(&recvftpMsg);
		if(recvftpMsg.msg_type == 'D')
		{
			write_len = fwrite(recvftpMsg.buf,1,recvftpMsg.buf_len, fp_write);
		}
		else
		{
			printf("msg type error\n");
			break;
		}
		
	
	}while(write_len >= 512);

	fclose(fp_write);
	
	return 0;
}



//上传文件
int  myftp_upload_file(char *upfilename)
{
		FILE *fp_read = NULL;

		int read_len = 0;

		fp_read = fopen(upfilename, "rb");

		if(fp_read == NULL) 
		{
			  perror("file open error");
		  	return -1;
		}

		while(1)
		{
				read_len =  fread(myftpMsg.buf, 1,512,fp_read);
				myftpMsg.buf_len = htons(read_len);
				myftpMsg.msg_type = 'D';

				sendFtpMsg(&myftpMsg);

				if(read_len < 512)
				{
					
					 fclose(fp_read);
					 break;
				}

				usleep(100);
				
		}

    return 0;
}


int sendFtpMsg(myftpMsg_t *msg)
{
	int send_len = 0;
	send_len = send(sockfd,msg,sizeof(myftpMsg_t),0);

	return 0;
}

int recvFtpMsg(myftpMsg_t *msg)
{
	 int recv_len = 0;

   recv_len = recv(sockfd,msg,sizeof(myftpMsg_t),0);

	 return recv_len;

}


void show_menu(void)
{
	 printf("-------------------------------------\n");
	 printf("list --> get file list from server\n");
	 printf("put file --> upload file to server\n");
	 printf("get file --> dowload file from server\n");
	 printf("-------------------------------------\n");
	 printf("<input>:");
}


