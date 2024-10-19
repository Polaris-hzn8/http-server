/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Sun 13 Oct 2024 23:34:36 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "channel.h"

typedef struct channel_map_st
{
	int			size_;
	CHANNEL**	list_;
}CHANNELMAP, *PCHANNELMAP;

PCHANNELMAP channel_map_init(int size);
bool channel_map_uninit(PCHANNELMAP channel_map);
bool channel_map_resize(PCHANNELMAP channel_map, int size, int unit_size);