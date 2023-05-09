/*************************************************************************
	> File Name: server.c
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Mon 24 Apr 2023 11:51:55 CST
************************************************************************/

#include "head.h"
#include "socket.h"

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "./%s port path", argv[0]);
		exit(1);
	}

	//1.切换服务器的工作路径（设置资源根目录）
	chdir(argv[2]);
	
	//2.建立用于监听的套接字
	unsigned short port = atoi(argv[1]);
	int lfd = initListenFd(port);

	//3.启动服务器程序
	


	return 0;
}



