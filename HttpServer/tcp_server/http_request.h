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

#define MAX_HEADER_NUMS 12

enum http_request_parse_state
{
	HRPS_LINE,	//req_line
	HRPS_HEAD,	//req_head
	HRPS_BODY,	//req_body
	HRPS_DONE,	//done
};

typedef struct kv_pair_st {
	char* _key;
	char* _value;
}KVPAIR, *PKVPAIR;

typedef struct http_request_st
{
	enum http_request_parse_state _cur_hrps;

	// request line
	char* _method;
	char* _resurl;
	char* _version;

	// request headers
	int _header_nums;
	PKVPAIR _headers;

	// request data(post)

}HTTP_REQUEST, *PHTTP_REQUEST;

PHTTP_REQUEST hr_init();
void hr_reset(PHTTP_REQUEST request);
void hr_destroy(PHTTP_REQUEST request);

enum http_request_parse_state hr_get_parse_state(PHTTP_REQUEST request);
bool hr_add_header(PHTTP_REQUEST request, PKVPAIR pkvpair);
char* hr_get_header_value(PHTTP_REQUEST request, const char* key);

bool hr_parse_req_line(PHTTP_REQUEST request, PBUFFER buffer);
bool hr_parse_req_header(PHTTP_REQUEST request, PBUFFER buffer);
