

//组播发送

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

#define BUF_LEN  128

char dataBuf[BUF_LEN]="group send message";

int groupSocket = 0;  //组播socket
unsigned short  groupPort = 0;//保存组播端口号

struct sockaddr_in groupAddr;  //组播地址

struct sockaddr_in peerAddr;   //对方IP地址


char groupIP[30]="224.10.10.1";


// ./groupSend port

int main(int argc, char *argv[])
{

	int iret = 0;
	
    if(argc !=2 )
    {
    	printf("parameter error!\n");
    	printf("usage: ./groupSend Port\n");
    	return 0;
    }

    groupPort = (unsigned short)atoi(argv[1]);

    //创建组播socket

    groupSocket = socket(AF_INET, SOCK_DGRAM,0);
    if(groupSocket < 0)
    {
    	printf("create groupSocket error!\n");

    	return 0;
    }

	//初始化组播IP和端口号
    memset(&groupAddr,0,sizeof(struct sockaddr_in));

    groupAddr.sin_family = AF_INET;
    groupAddr.sin_port = htons(groupPort);
    groupAddr.sin_addr.s_addr = inet_addr(groupIP);


    int addr_len = sizeof(struct sockaddr_in);
    int dataLen = 0;


	//组播发送数据
    while(1)
    {

    	strcpy(dataBuf, "group send message!!!");
		dataLen = sendto(groupSocket, dataBuf, strlen(dataBuf),0, 
					(struct sockaddr *)&groupAddr, addr_len);


	    printf("group send len:%d\n",dataLen);


	    //接收数据
	    
		dataLen = recvfrom(groupSocket, dataBuf,BUF_LEN,0, 
							(struct sockaddr *)&peerAddr, &addr_len);
		
		
	    printf("group recv len:%d  data:%s\n",dataLen, dataBuf);

	    sleep(2);
	    
     	
    }



    
}



