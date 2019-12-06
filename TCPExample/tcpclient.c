

//TCP 客户端程序

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>



#define BUF_LEN   128



//.tcpclient  ServerIP  Port


int clientSocket = 0;

char serverIP[50] = {0};
unsigned short serverPort = 0;


int main(int argc, char *argv[])
{
 	int iret = 0;

	if(argc != 3)
	{
		printf("parameter number error\n");
		printf("usage: ./tcpserver serverIP port\n");

		return 0;
	}

	strcpy(serverIP, argv[1]);
	serverPort = atoi(argv[2]);

	//创建socket
	clientSocket = socket(AF_INET,SOCK_STREAM,0);
	if(clientSocket == 0)
	{
		printf("client create socket error\n");
		return 0;
	}


	struct sockaddr_in serverAddr;
	int addr_len = sizeof(struct sockaddr_in);
	memset(&serverAddr,0, sizeof(struct sockaddr_in));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = inet_addr(serverIP);

	iret = connect(clientSocket, (struct sockaddr *)&serverAddr, addr_len);

	if(iret < 0)
	{
		printf("connect error\n");
		return 0;
	}

	printf("connect server ok\n");


	int dataLen = 0;
	char dataBuf[BUF_LEN];

	//客户端连接成功，开始发送和接收数据,服务器先接收数据
	while(1)
	{

		memset(dataBuf, 0, BUF_LEN);
		strcpy(dataBuf, "I am client");
		dataLen = send(clientSocket,dataBuf, strlen(dataBuf),0);

		printf("send return :%d\n", dataLen);

	
		memset(dataBuf, 0, BUF_LEN);
		dataLen = recv(clientSocket,dataBuf, BUF_LEN,0);

		printf("client receive data:%s  recv return:%d\n", dataBuf, dataLen);
		
		sleep(2);
	   
	}
	
}


