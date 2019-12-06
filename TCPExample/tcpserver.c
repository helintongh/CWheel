
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <unistd.h>

#include <string.h>


/*TCP 服务器程序

./tcpserver serverip  port

*/

#define BUF_LEN  128


char serverIP[50] = {0};//保存IP地址
unsigned short serverPort = 0;//保存端口号
int serverSocket = 0; //服务socket,用来接收客户端的连接请求

int connSocket = 0; //连接socket,用来保存客户端(accept函数)的返回值

int main(int argc, char *argv[])
{

	int iret = 0;

	if(argc != 3)//需要三个参数，参数小于3说明出错了
	{
		printf("parameter number error\n");
		printf("usage: ./tcpserver serverIP port\n");

		return 0;
	}

	strcpy(serverIP, argv[1]);//将输入的参数2拷贝到serverIP中
	serverPort = atoi(argv[2]);//字符串转换从整数,参数3传入serverPort

	//创建socket
	serverSocket = socket(AF_INET,SOCK_STREAM,0);
	if(serverSocket == 0)
	{
		printf("create socket error\n");
		return 0;
	}

	struct sockaddr_in serverAddr;//保存服务器的socket地址
	int addr_len = sizeof(struct sockaddr_in);//地址长度
	memset(&serverAddr,0, sizeof(struct sockaddr_in));//初始化为0
	//初始化服务器的IP地址和Port
	serverAddr.sin_family = AF_INET;//IPV4
	serverAddr.sin_port = htons(serverPort);//转换成网络字节序
	serverAddr.sin_addr.s_addr = inet_addr(serverIP);//将点分十进制转换成二进制
	

	//绑定socket
	iret = bind(serverSocket,(struct sockaddr *)&serverAddr,addr_len);
	if(iret < 0)
	{
		printf("bind error\n");
		return 0;
	}

	//监听socket
	iret = listen(serverSocket, 20);
	if(iret < 0)
	{
		printf("listen error\n");
		return 0;
	}

	printf("server is listening!!!\n");
	
	struct sockaddr_in clientAddr;//用来保存客户端的socket地址结构
	memset(&clientAddr,0, sizeof(struct sockaddr_in));//局部变量清0
	

	char clientIP[50] = {0};//客户端IP
	unsigned short clientPort = 0;//客户端port，清0
	//调用accpet函数来处理客户端连接请求
	connSocket = accept(serverSocket,(struct sockaddr *)&clientAddr, &addr_len);

	clientPort = ntohs(clientAddr.sin_port);
	strcpy(clientIP,inet_ntoa(clientAddr.sin_addr));

	printf("new client connect:IP: %s   Port:%d\n",clientIP,clientPort);

	

	if(connSocket < 0)//如果小于0说明没有连接上
	{
	   	printf("accept error\n");
		return 0;
	}

	int dataLen = 0;


	char dataBuf[BUF_LEN];

	//客户端连接成功，开始发送和接收数据,服务器先接收数据
	while(1)
	{
		memset(dataBuf, 0, BUF_LEN);
		dataLen = recv(connSocket,dataBuf, BUF_LEN,0);

		printf("server receive data:%s  recv return:%d\n", dataBuf, dataLen);

		memset(dataBuf, 0, BUF_LEN);
		strcpy(dataBuf, "I am Server");
		dataLen = send(connSocket,dataBuf, strlen(dataBuf),0);


	   
	}

	return 0;

}
