/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sun 13 Oct 2024 23:07:29 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#pragma once

#include <stdbool.h>

typedef int(*handle_func)(void* arg);

typedef struct channel_st
{
	int				fd_;
	int				events_;
	handle_func		read_call_back_;	//读回调
	handle_func		write_call_back_;	//写回调
	void*			arg_;
}CHANNEL, *PCHANNEL;

enum fd_event
{
	TIME_OUT	= 0x01,
	READ_EVENT	= 0x02,
	WRITE_EVENT = 0x04
};

PCHANNEL channel_init(int fd, int events, handle_func read_call_back, handle_func write_call_back, void* arg);

bool set_write_event_enable(PCHANNEL channel, bool b_enable);
bool get_write_event_enable(PCHANNEL channel);

