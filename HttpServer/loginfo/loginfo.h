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

#include <time.h>
#include <stdio.h>

#define LOG_OUT(format, ...) \
    do { \
        time_t now = time(NULL); \
        struct tm *t = localtime(&now); \
        char time_str[20]; \
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t); \
        printf("[%s] [%s:%d] " format "\n", time_str, __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)

//使用方式
//LOG("This is a test log with no variables.");
//LOG("This log has a variable: %d", 42);
