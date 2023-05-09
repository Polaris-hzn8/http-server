# URL请求信息截取

---

### 1.sscanf

- 利用格式化输入sscanf函数实现

- 函数原型：`sscanf(const char *str, const char *format, ...)`
- 主要功能：将参数str的字符串根据参数format字符串来转换并格式化数据，转换后的结果存于对应的参数中，
    - 根据格式从字符串中提取数据，如从字符串中取出整数、浮点数和字符串等。
    - 取<font color='#BAOC2F'>指定长度</font>的字符串
    - 取到<font color='#BAOC2F'>指定字符为止</font>的字符串
    - 取仅<font color='#BAOC2F'>包含指定字符集</font>的字符串
    - 取到<font color='#BAOC2F'>指定字符集为止</font>的字符串

### 2.实际案例

从 `http://www.baidu.com:1234` 中分别获取传输协议、网站域名、端口等信息

```cpp
#include <stdio.h>

int main() {
    const char *str = "https://www.baidu.com:20000";
    char protocal[32] = {0};
    char host[128] = {0};
    char port[8] = {0};
    sscanf(str, "%[^:]://%[^:]:%[1-9]", protocal, host, port);

    printf("protocal : %s\n", protocal);
    printf("host : %s\n", host);
    printf("port : %s\n", port);
    return 0;
}
```











