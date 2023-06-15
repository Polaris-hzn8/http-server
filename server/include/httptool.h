/*************************************************************************
	> File Name: http.h
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Mon 24 Apr 2023 11:23:47 PM CST
 ************************************************************************/

#ifndef _HTTP_H
#define _HTTP_H

#include "head.h"

//在接受到http消息之后 进行具体的解析操作
int parseHttpRequestLine(const char* line, int cfd);
//根据文件后缀 or 名字找到对应的http content type
const char* getFileType(const char *name);
//发送响应头
int sendHeadMsg(int cfd, int status, const char *descr, const char *type, int length);

#endif
