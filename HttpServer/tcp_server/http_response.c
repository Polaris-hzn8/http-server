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

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "http_response.h"

#define MAX_HEADER_NUMS 12

PHTTP_RESPONSE hres_init()
{
	PHTTP_RESPONSE response = (PHTTP_RESPONSE)malloc(sizeof(HTTP_RESPONSE));

	response->_state_code = HRSC_Unknow;
	bzero(response->_statu_desc, sizeof(response->_statu_desc));

	response->_headers = (PKVPAIR)malloc(sizeof(KVPAIR) * MAX_HEADER_NUMS);	
	response->_header_nums = 0;
	bzero(response->_headers, sizeof(KVPAIR) * MAX_HEADER_NUMS);

	response->_send_hres_func = NULL;

	// other params
	bzero(response->_file_name, sizeof(response->_file_name));

	return response;
}

void hres_destroy(PHTTP_RESPONSE response)
{
	if (NULL != response) {
		free(response->_headers);
		free(response);
	}
}

bool hres_add_header(PHTTP_RESPONSE response, const char* key, const char* value)
{
	if (NULL == response || NULL == key || NULL == value)
		return false;
	strcpy(response->_headers[response->_header_nums]._key, key);
	strcpy(response->_headers[response->_header_nums]._value, value);
	response->_header_nums++;
	return true;
}

char* hres_get_header_value(PHTTP_RESPONSE response, const char* key)
{
	if (NULL == response)
		return NULL;

	for (int i = 0; i < response->_header_nums; ++i) {
		if (0 == strncasecmp(response->_headers[i]._key, key, strlen(key))) {
			return response->_headers[i]._value;
		}
	}
	return NULL;
}

void hres_send_data(PHTTP_RESPONSE response, PBUFFER buffer, int socket)
{
	// status line
	char tmp[4096] = { 0 };
	sprintf(tmp, "http/1.1 &d &s\r\n", response->_state_code, response->_statu_desc);
	buffer_write(buffer, tmp);

	// response header
	for (int i = 0; i < response->_header_nums; ++i) {
		PKVPAIR pkvpair = &response->_headers[i];
		sprintf(tmp, "%s: %s\r\n", pkvpair->_key, pkvpair->_value);
		buffer_write(buffer, tmp);
	}

	// empty line
	buffer_write(buffer, "\r\n");

	// response body
	response->_send_hres_func(response->_file_name, buffer, socket);
}

int hres_send_directory(const char* dir_name, int cfd)
{
	LOGOUT("send_http_response_body_directory dir_name[%s]\n", dir_name);

	char buff[4096] = { 0 };
	sprintf(buff, "<html><head><title>%s</title></head><body><table>", dir_name);

	struct dirent** name_list = NULL;
	int name_count = scandir(dir_name, &name_list, NULL, alphasort);
	for (int i = 0; i < name_count; ++i) {
		struct stat st;
		char item_name_impact[4096] = { 0 };
		char* item_name = name_list[i]->d_name;
		sprintf(item_name_impact, "%s/%s", dir_name, item_name);
		stat(item_name_impact, &st);
		if (S_ISDIR(st.st_mode)) {	//目录项
			sprintf(buff + strlen(buff), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", item_name, item_name, st.st_size);
		}
		else {						//文件项
			sprintf(buff + strlen(buff), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", item_name, item_name, st.st_size);
		}

		send(cfd, buff, strlen(buff), 0);
		memset(buff, 0, sizeof(buff));
		free(name_list[i]);
	}

	sprintf(buff, "</table></body></html>");
	send(cfd, buff, strlen(buff), 0);
	memset(buff, 0, sizeof(buff));
	free(name_list);
	return 0;
}

int hres_send_file(const char* file_name, int cfd)
{
	int fd = open(file_name, O_RDONLY);
	if (fd <= 0) {
		perror("send_file_info open fd <= 0");
		return -1;
	}

	//while (1) {
	//	char buff[1024] = { 0 };
	//	int len = read(fd, buff, sizeof(buff));
	//	if (len > 0) {
	//		send(cfd, buff, len, 0);
	//		usleep(10);
	//	} else if (len == 0) {
	//		break;
	//	} else {
	//		perror("send_file_info read");
	//		return -1;
	//	}
	//}

	struct stat st;
	int ret = stat(file_name, &st);
	if (ret < 0) {
		perror("stat failed");
		return -1;
	}

	int file_size = st.st_size;

	off_t offset = 0;
	while (offset < file_size) {
		int ret = sendfile(cfd, fd, &offset, file_size - offset);
		LOGOUT("sendfile ret value[%d].\n", ret);
		if (ret == -1) {
			if (errno == EAGAIN)
				LOGOUT("sendfile no data prepared.\n");
			else
				perror("sendfile failed");
		}
	}

	close(fd);

	return 0;
}

int hres_send_line(int cfd, int statu, const char* descr, const char* type, int length)
{
	char buff[4096] = { 0 };
	sprintf(buff, "http/1.1 &d &s\r\n", statu, descr);
	sprintf(buff + strlen(buff), "content-type: %s\r\n", type);
	sprintf(buff + strlen(buff), "content-length: %d\r\n\r\n", length);

	send(cfd, buff, strlen(buff), 0);
	return 0;
}
