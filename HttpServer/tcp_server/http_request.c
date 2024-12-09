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
#include <sys/stat.h>
#include "tools.h"
#include "http_request.h"
#include "http_response.h"
#include "../loginfo/loginfo.h"

#define MAX_HEADER_NUMS 12

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

bool hr_add_header(PHTTP_REQUEST request, const char* key, const char* value)
{
	if (NULL == key || NULL == value)
		return false;
	request->_headers[request->_header_nums]._key = key;
	request->_headers[request->_header_nums]._value = value;
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

//////////////////////////////////////////////////////////////////////
char* find_request_line_part(const char* start, const char* end, const char* splitstr, char** ptr)
{
	// find the split
	char* split = end;
	int line_size = start - end;
	if (splitstr != NULL) {
		split = memmem(start, line_size, splitstr, strlen(splitstr));
		if (NULL == split) {
			LOG_OUT("find req_line content split failed, invalid data.");
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
	if (NULL == end) {
		LOG_OUT("find req_line crlf failed, invalid data.");
		return false;
	}

	char* start = buffer->data_ + buffer->read_pos_;
	int line_size = end - start;
	if (line_size <= 0) {
		LOG_OUT("hr_parse_req_line failed line_size less than zero.");
		return false;
	}

	// 请求行解析
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

bool hr_parse_req_header(PHTTP_REQUEST request, PBUFFER buffer)
{
	char* end = buffer_find_crlf(buffer);
	if (NULL == end) {
		LOG_OUT("find req_header crlf failed, invalid data.");
		return false;
	}

	char* start = buffer->data_ + buffer->read_pos_;
	int line_size = end - start;
	if (line_size <= 0) {
		LOG_OUT("hr_parse_req_line failed line_size less than zero.");
		return false;
	}

	// 请求头解析
	// Host: 192.168.26.52 : 6789
	// Connection : keep - alive
	// Cache - Control : max - age = 0
	// Upgrade - Insecure - Requests : 1
	// Origin : null
	// User - Agent : Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 70.0.3538.67 Safari / 537.36
	// Accept : text / html, application / xhtml + xml, application / xml; q = 0.9, image / webp, image / apng, */*;q=0.8
	// Accept-Encoding: gzip, deflate
	// Accept-Language: zh,zh-CN;q=0.9,en;q=0.8
	// Content - Length : 84
	// Content - Type : application / x - www - form - urlencoded
	char* target = NULL;
	target = memmem(target, line_size, ": ", 2);
	if (NULL != target) {
		// key
		int len_k = target - start;
		char* key = malloc(len_k + 1);
		strncpy(key, start, len_k);
		key[len_k] = '\0';
		// value
		int len_v = end - (target + 2);
		char* value = malloc(len_v + 1);
		strncpy(value, target + 2, len_v);
		value[len_v] = '\0';
		// add_header
		hr_add_header(request, key, value);
		// parse finished kv
		buffer->read_pos_ += (line_size + 2);
	} else {
		// parse header finished
		{
			// Get请求方式
			buffer->read_pos_ += 2;	//跳过空行
			request->_cur_hrps = HRPS_DONE;
		}
		{
			// Post请求方式

		}
	}
	return true;
}

bool hr_parse_req(PHTTP_REQUEST request, PBUFFER buffer_read,
	PHTTP_RESPONSE response, PBUFFER buffer_send, int socket)
{
	bool b_ret = true;
	while (true) {
		switch (request->_cur_hrps) {
		case HRPS_LINE:
			/* 解析请求行 */
			b_ret = hr_parse_req_line(request, buffer_read);
			break;
		case HRPS_HEAD:
			/* 解析请求头 */
			b_ret = hr_parse_req_header(request, buffer_read);
			break;
		case HRPS_BODY:
			/* 解析请求体 */
			/**
			 * REM 只处理Get请求（无请求数据）
			 * 客户端向服务器发送的Post请求，数据块格式主要有4种
			 * 若需要处理该请求：
			 * 1.需要编写这4中数据对应的解析方式
			 * 2.根据客户端请求协议的content-length与content-type
			 * 3.调用对应的解析函数进行处理
			 */
			b_ret = hr_parse_req_body(request, buffer_read);
			break;
		case HRPS_DONE:
			break;
		default:
			break;
		}
		if (!b_ret) {
			/* 请求解析异常 */
			LOG_OUT("hr_parse_req parse step[%d] failed.", request->_cur_hrps);
			break;
		}
		if (HRPS_DONE == request->_cur_hrps) {
			/* 请求解析结束 */
			// 准备响应数据
			hr_parse_get(request, response);
			// 返回响应数据
			hres_send_data(response, buffer_send, socket);
		}
	}
	request->_cur_hrps = HRPS_LINE;
	return b_ret;
}

// 处理get请求
bool hr_parse_get(PHTTP_REQUEST request, PHTTP_RESPONSE response)
{
	if (0 == strlen(request->_method)) {
		LOG_OUT("request _method is empty.");
		return false;
	}
	
	if (strcasecmp(request->_method, "get") != 0) {
		LOG_OUT("only Get request _method can process.");
		return false;
	}

	decode_str(request->_resurl, request->_resurl);

	char* req_file = NULL;
	if (strcmp(request->_resurl, "/") == 0)
		req_file = "./";//folder
	else
		req_file = request->_resurl + 1;//file

	LOG_OUT("parse_request_line req_method[%s] req_file[%s]\n", request->_method, req_file);

	struct stat st;
	int ret = stat(req_file, &st);
	if (ret == -1) {
		//404 no found
		response->_state_code = HRSC_NotFound;
		strcpy(response->_statu_desc, "Desc: Not Found");

		hres_add_header(response, "Content-type", get_file_type(".html"));
		
		response->_send_hres_func = hres_send_file;

		strcpy(response->_file_name, "404.html");

		return true;
	}

	response->_state_code = HRSC_OK;
	strcpy(response->_statu_desc, "Desc: OK");
	strcpy(response->_file_name, req_file);

	if (S_ISDIR(st.st_mode)) {
		//将目录内容返回
		hres_add_header(response, "Content-type", get_file_type(".html"));
		response->_send_hres_func = hres_send_directory;
	} else {
		//将文件内容返回
		char file_size[64];
		sprintf("%ld", st.st_size);
		hres_add_header(response, "Content-type", get_file_type(req_file));
		hres_add_header(response, "Content-length", file_size);
		response->_send_hres_func = hres_send_file;
	}

	return true;
}

