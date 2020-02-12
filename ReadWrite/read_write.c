/*
有一个很大的文件，
open打开
read读出来
open打开另一个文件
write到open打开的另一个文件里
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/type.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, const char* argv[])
{
	//打开一个文件
	int fd = open("english.txt", O_RDWR);
	printf("fd = %d\n", fd); 

	//打开另一个文件，做写操作
	int fd1 = open("temp", O_WRONLY | O_CREAT, 0664);
	printf("fd1 = %d\n", fd1);

	//先read操作
	char buf[4096];//一次读4k
	int len = read(fd, buf, sizeof(buf));//read返回字节长度
	//一次读不完一直读
	while( len > 0 )
	{
		//数据写入文件中，len就是实际读出的字节数
		int ret = write(fd1, buf, len);
		printf("ret = %d\n", ret);
		//第n次直到读完
		len = read(fd, buf, sizeof(buf));
		
	}

	close(fd);
	close(fd1);

	return 0;
}