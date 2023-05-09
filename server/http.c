/*************************************************************************
	> File Name: http.c
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Mon 24 Apr 2023 11:23:38 PM CST
 ************************************************************************/

#include "head.h"
#include "http.h"

/**
 * @brief 在接受到http消息之后 进行具体的解析操作
 * 
 * @param line 请求行字符串内容
 * @param cfd 请求行解析后 需要将数据发回给客户端
 * @return int 
 */
int parseHttpRequestLine(const char* line, int cfd) {
	/* 请求行内容：请求的方式POST|GET 客户端请求的静态资源 客户端使用的HTTP协议的版本 */	
	/* sprintf: 对字符串进行格式化 */
	/* sscanf: 对格式化的字符串进行拆分 */

	//1.解析请求行
	char method[12];//GTE|POST
	char path[1024];//请求服务器静态资源的路径
	sscanf(line, "%[^ ] %[^ ]", method, path);
	if (strcasecmp(method, "get") != 0) return -1;//不区分大小写进行字符串比较
	
	//2.处理客户端请求的静态资源 的路径
	char *filepath = NULL;
	if (strcmp(path, "/") == 0) filepath = "./";//直接访问当前的资源目录下（将绝对路径切换为相对路径）
	else filepath = path + 1;//去掉反斜杠即可转化相对路径

	//3.判断访问的资源是文件 or 目录
	struct stat st;//封装文件or目录属性的结构体
	if (stat(filepath, &st) == -1) {
		/* 3 - 1 : 获取目标文件属性失败 资源不存在 返回值为-1 回复404页面 */
		sendHeadMsg(cfd, 404, "Resource Not Found", getFileType(".html"), -1);
		mySendFile("404.html", cfd);//404页面存在根目录中
		return 0;
	}
	if (S_ISDIR(st.st_mode)) {
		/* 3 - 2 : 请求的内容是一个目录 则将本地目录的内容发送给客户端 */


	} else {
		/* 3 - 3 : 请求的内容是一个文件 则将文件的内容发送给客户端 */
		sendHeadMsg(cfd, 200, "OK", getFileType(filepath), st.st_size);
		mySendFile(filepath, cfd);//404页面存在根目录中
	}

	return 0;
}


/**
 * @brief 根据文件名找到对应的http content type
 * 
 * @param name 文件的名字
 * @return const char* 
 */
const char* getFileType(const char *name) {
	// a.jpg a.mp4 a.html
	const char *dot = strrchar(name, '.');//strrchar从右向左查字符串
	if (dot == NULL) return "text/plain; charset=utf-8";
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
	if (strcmp(dot, ".gif") == 0) return "image/gif";
	if (strcmp(dot, ".png") == 0) return "image/png";
	if (strcmp(dot, ".css") == 0) return "text/css";
	if (strcmp(dot, ".au") == 0) return "audio/basic";
	if (strcmp(dot, ".wav") == 0) return "audio/wav";
	if (strcmp(dot, ".avi") == 0) return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0) return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0) return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0) return "model/vrml";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0) return "audio/midi";
	if (strcmp(dot, ".mp3") == 0) return "audio/mpeg";
	if (strcmp(dot, ".ogg") == 0) return "application/ogg";
	if (strcmp(dot, ".pac") == 0) return "application/x-ns-proxy-autoconfig";
	return "text/plain; charset=utf-8";
}


/**
 * @brief 发送http消息响应头
 * 
 * @param cfd 用于通信的文件描述符
 * @param status http状态码
 * @param descr 状态描述
 * @param type 消息类型 content type
 * @param length 消息长度 content length
 * @return int 
 */
int sendHeadMsg(int cfd, int status, const char *descr, const char *type, int length) {
	//1.拼接状态行
	char buff[4096];
	sprintf(buff, "http/1.1 %d %s\r\n", status, descr);
	
	//2.拼接响应头
	sprintf(buff + strlen(buff), "content-type: %s\r\n", type);
	sprintf(buff + strlen(buff), "content-length: %s\r\n", length);
	
	//3.空行
	sprintf(buff + strlen(buff), "\r\n");
	
	//4.发送响应头
	send(cfd, buff, strlen(buff), 0);
	return 0;
}


/**
 * @brief 发送文件数据
 * 
 * @param filename 发送文件的名字 
 * @param cfd 用于通信的文件描述符
 * @return int 
 */
int mySendFile(const char *filename, int cfd) {
	//1.打开文件
	int fd = open(filename, O_RDONLY); assert(fd > 0);

	//2.从文件中读取数据 并将数据发送给客户端
	// while (1) {
	// 	char buff[1024];
	// 	int len = read(fd, buff, sizeof(buff));
	// 	if (len > 0) {
	// 		/* 正常读取文件并发送 */
	// 		send(cfd, buff, len, 0);
	// 		usleep(10);//控制发送端发送数据的速率
	// 	} else if (len == 0) {
	// 		/* 文件读取结束退出循环 */
	// 		break;
	// 	} else {
	// 		/* 异常读取文件 输出错误信息 */
	// 		perror("sendFile:read");
	// 	}
	// }
	/* 直接调用linux提供的系统函数sendfile进行文件发送更高效 */
	/* 效率更高 且能够减少内存的拷贝 数据块有内存区到用户区的拷贝过程 */
	int size = lseek(fd, 0, SEEK_END);//利用lseek求文件的大小
	sendfile(cfd, fd, NULL, size);

	return 0;
}


/**
 * @brief 发送目录数据
 * 
 * @param dirname 发送目录的路径
 * @param cfd 用于通信的文件描述符
 * @return int 
 */
int mySendDir(const char *dirname, int cfd) {
	
	
	
	return 0;
}


