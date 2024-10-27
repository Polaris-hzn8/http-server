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
#include "workthread.h"
#include "../reactor/eventloop.h"

#define LOGOUT(fmt, ...) do { printf(fmt, ##__VA_ARGS__); fflush(stdout); } while(0)

typedef struct thread_pool_st {
	bool is_running_;
	int idx_;
	int thread_num_;
	PWORK_THREAD* work_threads_;
	PEVENTLOOP main_loop_;
}THREAD_POLL, *PTHREAD_POOL;

PTHREAD_POOL thread_pool_init(PEVENTLOOP main_loop, int thread_num);
int thread_pool_run(PTHREAD_POOL thread_pool);

PEVENTLOOP get_event_loop_from_workthread(PTHREAD_POOL thread_pool);
