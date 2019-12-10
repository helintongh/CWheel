

//组播接收

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
char dataBuf[BUF_LEN]="group recv message";

//组播IP地址
char groupIP[30]="224.10.10.1";


int groupSocket = 0;  //组播socket
unsigned short  groupPort = 0;//保存广播端口号

struct sockaddr_in groupAddr;  //本机地址

struct sockaddr_in peerAddr;   //对方IP地址

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

	//绑定组播IP和端口号
    memset(&groupAddr,0,sizeof(struct sockaddr_in));

    groupAddr.sin_family = AF_INET;
    groupAddr.sin_port = htons(groupPort);
    groupAddr.sin_addr.s_addr = inet_addr(groupIP);

    int addr_len = sizeof(struct sockaddr_in);

    iret = bind(groupSocket, (struct sockaddr*)&groupAddr, addr_len);
	if(iret < 0)
	{
		printf("bind error \n");

		return 0;
	}

	//将该网络接口加入多播组
	struct ip_mreq  mreq;
	memset((unsigned char *)&mreq,0, sizeof(struct ip_mreq));

	mreq.imr_multiaddr.s_addr = inet_addr(groupIP); /*多播地址*/
    mreq.imr_interface.s_addr = INADDR_ANY; /*将本机任意网络接口*/
	
	     /*将本机加入多播组*/
    iret = setsockopt(groupSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof(mreq));
    if (iret < 0)
    {
        perror("setsockopt():IP_ADD_MEMBERSHIP");
        return -4;
    }

    int dataLen = 0;

    //接收组播数据
    while(1)
    {

    	memset(dataBuf, 0 ,BUF_LEN);
    	
    	dataLen=recvfrom(groupSocket,dataBuf, BUF_LEN,0,
    	        (struct sockaddr*)&peerAddr, &addr_len);

    	printf("len:%d  data:%s\n",dataLen,dataBuf);


		strcpy(dataBuf,  "group recv message");
		dataLen=sendto(groupSocket,dataBuf, strlen(dataBuf),0,
	        (struct sockaddr*)&peerAddr, addr_len);

	   	printf("send len:%d\n",dataLen);
   	
    }
    
}


