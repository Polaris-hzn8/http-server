/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Tue 08 Oct 2024 00:28:16 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#pragma once

#include <stdbool.h>
#include "../buffer/buffer.h"

enum http_response_state_code
{
	HRSC_Unknow				= 0,
	HRSC_OK					= 200,
	HRSC_MovedPermanently	= 301,
	HRSC_MovedTemporarily	= 302,
	HRSC_BadRequest			= 400,
	HRSC_NotFound			= 404
};

typedef struct kv_pair_st {
	char _key[64];
	char _value[128];
}KVPAIR, * PKVPAIR;

typedef void (*send_hres_func)(const char* file_name, PBUFFER buffer, int socket);
typedef struct http_response_st {
	// status line
	enum http_response_state_code _state_code;
	char _statu_desc[1024];
	// response headers
	int _header_nums;
	PKVPAIR _headers;
	// response data(get/post)
	send_hres_func _send_hres_func;
	// other params
	char _file_name[1024];
}HTTP_RESPONSE, *PHTTP_RESPONSE;

PHTTP_RESPONSE hres_init();
void hres_destroy(PHTTP_RESPONSE response);

bool hres_add_header(PHTTP_RESPONSE response, const char* key, const char* value);
char* hres_get_header_value(PHTTP_RESPONSE response, const char* key);

// 组织响应数据
void hres_send_data(PHTTP_RESPONSE response, PBUFFER buffer, int socket);

// 文件发送
int hres_send_line(int cfd, int statu, const char* descr, const char* type, int length);
int hres_send_directory(const char* dir_name, int cfd);
int hres_send_file(const char* file_name, int cfd);

