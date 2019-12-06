

/*
   广播发送端程序
   
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>


/*
 ./broadSend  port
*/

#define BUF_LEN  128

char dataBuf[BUF_LEN]="broad send message";


int broadSocket = 0;
unsigned short  broadPort = 0;//保存广播端口号

struct sockaddr_in broadAddr;  //广播地址

struct sockaddr_in peerAddr;   //对方IP地址


char broadIP[30]="192.168.1.255";

int main(int argc, char *argv[])
{
	int iret = 0;
	
    if(argc !=2 )
    {
    	printf("parameter error!\n");
    	printf("usage: ./broadSend Port\n");
    	return 0;
    }

    broadPort = (unsigned short)atoi(argv[1]);

    //创建广播socket

    broadSocket = socket(AF_INET, SOCK_DGRAM,0);
    if(broadSocket < 0)
    {
    	printf("create socket error!\n");

    	return 0;
    }

    memset(&broadAddr,0,sizeof(struct sockaddr_in));

    broadAddr.sin_family = AF_INET;
    broadAddr.sin_port = htons(broadPort);
    broadAddr.sin_addr.s_addr = inet_addr(broadIP);

    //设置socket 广播属性

   int  so_broadcast  = 1; //支持广播
    
   iret = setsockopt(broadSocket,SOL_SOCKET,SO_BROADCAST,&so_broadcast,sizeof(int));
   if(iret < 0)
   {
      printf("setsockopt error\n");
      return 0;
   }

   int addr_len = sizeof(struct sockaddr_in);

   int dataLen = 0;

	//发送广播报文  dataBuf
   while(1)
   {

   	   strcpy(dataBuf, "broad send message");
	   dataLen = sendto(broadSocket,dataBuf, strlen(dataBuf),0,
	                      (struct sockaddr *)&broadAddr, addr_len);

	   printf("send len:%d\n",dataLen );


	   
	   memset(dataBuf, 0, BUF_LEN );
	   dataLen = recvfrom(broadSocket,dataBuf, BUF_LEN, 0,
								(struct sockaddr *)&peerAddr, &addr_len);


	   printf("recv len:%d data:%s\n",dataLen, dataBuf);

	   sleep(1);

	   
   }
    
    

    
}

