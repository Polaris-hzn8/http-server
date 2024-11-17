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

#include "../buffer/buffer.h"
#include "../reactor/channel.h"
#include "../reactor/eventloop.h"

typedef struct tcp_connection_st {
	char name_[32];
	PBUFFER _buffer_r;
	PBUFFER _buffer_w;
	PCHANNEL _channel;
	PEVENTLOOP _event_loop;
}TCP_CONNECTION, *PTCP_CONNECTION;

PTCP_CONNECTION tcp_connection_init(int cfd, PEVENTLOOP event_loop);