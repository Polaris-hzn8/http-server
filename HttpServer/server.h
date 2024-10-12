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

int init_listen_fd(unsigned short port);

int epoll_run(int lfd);//ºËÐÄ
void* thread_accept_connect(void* arg);
void* thread_recv_http_request(void* arg);

int parse_request_line(const char* line, int cfd);

int send_http_response_head(int cfd, int statu, const char* descr, const char* type, int length);
int send_http_response_body_file(const char* file_name, int cfd);
int send_http_response_body_directory(const char* dir_name, int cfd);

const char* get_file_type(const char* name);
void decode_str(char* output, char* input);
int hex_to_dec(char c);

