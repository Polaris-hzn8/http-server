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

#include "channel.h"
#include "eventloop.h"

typedef struct dispatcher_st
{
	void* (*init)();
	int (*add)(PCHANNEL channel, PEVENTLOOP event_loop);
	int (*remove)(PCHANNEL channel, PEVENTLOOP event_loop);
	int (*modify)(PCHANNEL channel, PEVENTLOOP event_loop);
	int (*dispatch)(PEVENTLOOP event_loop, int timeout);
	int (*clear)(PEVENTLOOP event_loop);
}DISPATCHER, *PDISPATCHER;
