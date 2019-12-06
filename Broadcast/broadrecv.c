
/*
   广播接收端程序
   
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

	./br hostIP, port
	
*/


#define BUF_LEN  128

char dataBuf[BUF_LEN]={0};


char  strHostIP[50] = {0};
unsigned short broadPort = 0;

struct sockaddr_in  hostAddr;   //本机IP地址和端口号
struct sockaddr_in  peerAddr;   //对方IP地址


int recvSocket = 0;

int main(int argc, char *argv[])
{
    int iret = 0;
	
    if(argc !=3 )
    {
    	printf("parameter error!\n");
    	printf("usage: ./broadSend HostIP  Port\n");
    	return 0;
    }

    memset(&hostAddr,0, sizeof(struct sockaddr_in));
    memset(&peerAddr,0, sizeof(struct sockaddr_in));

    strcpy(strHostIP, argv[1]);
    broadPort = (unsigned short)atoi(argv[2]);

    // 创建接收端socket
    recvSocket = socket(AF_INET, SOCK_DGRAM,0);
    if(recvSocket < 0)
    {
    	printf("create socket error!\n");

    	return 0;
    }

    //绑定socket
    hostAddr.sin_port = htons(broadPort);
    hostAddr.sin_family = AF_INET;
    hostAddr.sin_addr.s_addr = INADDR_ANY; //inet_addr(strHostIP);

    int addr_len = sizeof(struct sockaddr_in);

    iret = bind(recvSocket, (struct sockaddr *)&hostAddr, addr_len);
    if(iret < 0)
    {
    	printf("bind error\n");
    	return 0;
    }

    int dataLen = 0;

    while(1)
    {
    	memset(dataBuf, 0, BUF_LEN );
	    dataLen = recvfrom(recvSocket,dataBuf, BUF_LEN, 0,
								(struct sockaddr *)&peerAddr, &addr_len);


	   printf("recv len:%d data:%s\n",dataLen, dataBuf);

	

	   //发送数据
	   strcpy(dataBuf,"udp receiver");
	   dataLen = sendto(recvSocket,dataBuf, strlen(dataBuf),0,
	                      (struct sockaddr *)&peerAddr, addr_len);

	   printf("send len:%d\n",dataLen );

	   
    }

    

    
}


