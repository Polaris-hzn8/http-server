/*************************************************************************
	> File Name: tool.h
	> Author: luochenhao
	> Mail: 3453851623@qq.com
	> Created Time: Thu 15 Jun 2023 06:47:44 PM CST
 ************************************************************************/

#ifndef _TOOL_H
#define _TOOL_H

#include "head.h"

//发送文件
int mySendFile(const char *filename, int cfd);
//发送目录
int mySendDir(const char *dirname, int cfd);


#endif
