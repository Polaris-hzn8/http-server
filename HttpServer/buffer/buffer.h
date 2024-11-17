/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sun 27 Oct 2024 22:40:45 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#pragma once

#include <stdio.h>
#include <stdlib.h>

#define LOGOUT(fmt, ...) do { printf(fmt, ##__VA_ARGS__); fflush(stdout); } while(0)

typedef struct buffer_st {
	char* data_;
	int capacity_;
	int read_pos_;
	int write_pos_;
}BUFFER, *PBUFFER;

PBUFFER buffer_init(int capacity);
int buffer_destroy(PBUFFER buffer);
int buffer_resize(PBUFFER buffer, int capacity);
int buffer_write_size_remain(PBUFFER buffer);
int buffer_read_size_remain(PBUFFER buffer);

// 直接写
int buffer_write(PBUFFER buffer, const char* data, int size);
int buffer_write_ex(PBUFFER buffer, const char* data);

// 接收套接字数据
int buffer_read_from_socket(PBUFFER buffer, int fd);
