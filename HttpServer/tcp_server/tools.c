/**
* Copyright (C) 2024 Polaris-hzn8 / LuoChenhao
*
* Author: luochenhao
* Email: lch2022fox@163.com
* Time: Mon 09 Dec 2024 08:07:20 CST
* Github: https://github.com/Polaris-hzn8
* Src code may be copied only under the term's of the Apache License
* Please visit the http://www.apache.org/licenses/ Page for more detail.
*
**/

#include <stdio.h>

int hex_to_dec(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}
void decode_str(char* output, char* input)
{
	while (*input != '\0') {
		if (input[0] == '%'
			&& isxdigit(input[1])
			&& isxdigit(input[2])) {
			*output = hex_to_dec(input[1]) * 16 + hex_to_dec(input[2]);
			input += 3;
		}
		else {
			*output = *input;
			++input;
		}
		++output;
	}
	*output = '\0';
}

const char* get_file_type(const char* file_name)
{
	// a.jpg a.mp4 a.html
	const char* suffix = strrchr(file_name, '.');//strrchar´ÓÓÒÏò×ó²é×Ö·û´®
	if (suffix == NULL)
		return "text/plain; charset=utf-8";
	if (strcmp(suffix, ".html") == 0 || strcmp(suffix, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(suffix, ".jpg") == 0 || strcmp(suffix, ".jpeg") == 0)
		return "image/jpeg";
	if (strcmp(suffix, ".gif") == 0)
		return "image/gif";
	if (strcmp(suffix, ".png") == 0)
		return "image/png";
	if (strcmp(suffix, ".css") == 0)
		return "text/css";
	if (strcmp(suffix, ".au") == 0)
		return "audio/basic";
	if (strcmp(suffix, ".wav") == 0)
		return "audio/wav";
	if (strcmp(suffix, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(suffix, ".mov") == 0 || strcmp(suffix, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(suffix, ".mpeg") == 0 || strcmp(suffix, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(suffix, ".vrml") == 0 || strcmp(suffix, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(suffix, ".midi") == 0 || strcmp(suffix, ".mid") == 0)
		return "audio/midi";
	if (strcmp(suffix, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(suffix, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(suffix, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}

