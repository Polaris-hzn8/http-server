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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "server.h"

#define LOGOUT(fmt, ...) do { printf(fmt, ##__VA_ARGS__); fflush(stdout); } while(0)

int main(int argc, char* argv[])
{
    //if (argc < 3) {
    //    printf("./a.out server_port resour_path\n");
    //    return -1;
    //}
    //unsigned short server_port = atoi(argv[1]);
    //const char* resour_path = argv[2];

    unsigned short server_port = 10000;
    const char* resour_path = "/home/luochenhao/web";

    chdir(resour_path);
    int lfd = init_listen_fd(server_port);
    if (lfd < 0) {
        perror("init_listen_fd");
        return 0;
    }

    epoll_run(lfd);
    
    return 0;
}