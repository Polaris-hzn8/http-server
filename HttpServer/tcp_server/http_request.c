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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "http_request.h"

PHTTP_REQUEST hr_init()
{
	PHTTP_REQUEST request = (PHTTP_REQUEST)malloc(sizeof(HTTP_REQUEST));
	request->_headers = (PKVPAIR)malloc(sizeof(KVPAIR) * MAX_HEADER_NUMS);
	request->_cur_hrps = HRPS_LINE;
	request->_method = NULL;
	request->_resurl = NULL;
	request->_version = NULL;
	request->_header_nums = 0;
	return request;
}

void hr_reset(PHTTP_REQUEST request)
{
	if (NULL == request)
		return;

	free(request->_method);
	free(request->_resurl);
	free(request->_version);
	request->_method = NULL;
	request->_resurl = NULL;
	request->_version = NULL;
	request->_cur_hrps = HRPS_LINE;

	if (request->_headers) {
		for (int i = 0; i < request->_header_nums; ++i) {
			free(request->_headers[i]._key);
			request->_headers[i]._key = NULL;
			free(request->_headers[i]._value);
			request->_headers[i]._value = NULL;
		}
		free(request->_headers);
		request->_headers = NULL;
	}
	request->_header_nums = 0;
}

void hr_destroy(PHTTP_REQUEST request)
{
	if (NULL == request)
		return;
	hr_reset(request);
	free(request);
	request = NULL;
}

enum http_request_parse_state hr_get_parse_state(PHTTP_REQUEST request)
{
	return request->_cur_hrps;
}

bool hr_add_header(PHTTP_REQUEST request, PKVPAIR pkvpair)
{
	if (NULL == pkvpair)
		return false;
	request->_headers[request->_header_nums]._key = pkvpair->_key;
	request->_headers[request->_header_nums]._value = pkvpair->_value;
	request->_header_nums++;
	return true;
}

char* hr_get_header_value(PHTTP_REQUEST request, const char* key)
{
	if (NULL == request)
		return NULL;

	for (int i = 0; i < request->_header_nums; ++i) {
		if (0 == strncasecmp(request->_headers[i]._key, key, strlen(key))) {
			return request->_headers[i]._value;
		}
	}
	return NULL;
}

char* find_request_line_part(const char* start, const char* end, const char* splitstr, char** ptr)
{
	// find the split
	char* split = end;
	int line_size = start - end;
	if (splitstr != NULL) {
		split = memmem(start, line_size, splitstr, strlen(splitstr));
		if (NULL == split) {
			LOGOUT("find req_line content split failed, invalid data.");
			return NULL;
		}
	}
	// find the content
	int content_size = split - start;
	char* tmp = (char*)malloc(content_size + 1);
	strncpy(tmp,start, content_size);
	tmp[content_size] = '\0';
	*ptr = tmp;
	return split + 1;
}
bool hr_parse_req_line(PHTTP_REQUEST request, PBUFFER buffer)
{
	char* end = buffer_find_crlf(buffer);
	char* start = buffer->data_ + buffer->read_pos_;
	int line_size = end - start;
	if (line_size) {
		// GET /?username=tom&phone=123&email=hello%40qq.com&rule=on HTTP/1.1
		// request method
		char* left = start;
		left = find_request_line_part(left, end, " ", &request->_method);
		// request url
		left = find_request_line_part(left, end, " ", &request->_resurl);
		// request version
		left = find_request_line_part(left, end, NULL, &request->_version);

		// parse finished
		buffer->read_pos_ += (line_size + 2);
		request->_cur_hrps = HRPS_HEAD;

		return true;
	}
	return false;
}

bool hr_parse_req_header(PHTTP_REQUEST request, PBUFFER buffer)
{


	return false;
}
