# Linux文件目录的遍历

---

### 1.api

```cpp
DIR *opendir(const char *name);//打开目录
struct dirent *readdir(DIR *dirp);//读取目录
struct dirent {
    ino_t          d_ino;       /* 文件对应的inode编号, 定位文件存储在磁盘的那个数据块上 */
    off_t          d_off;       /* 文件在当前目录中的偏移量 */
    unsigned short d_reclen;    /* 文件名字的实际长度 */
    unsigned char  d_type;      /* 文件的类型, linux中有7中文件类型 */
    char           d_name[256]; /* 文件的名字 */
};
 - DT_BLK：块设备文件
 - DT_CHR：字符设备文件
 - DT_DIR：目录文件
 - DT_FIFO ：管道文件
 - DT_LNK：软连接文件
 - DT_REG ：普通文件
 - DT_SOCK：本地套接字文件
 - DT_UNKNOWN：无法识别的文件类型
int closedir(DIR *dirp);// 关闭目录, 参数是 opendir() 的返回值
```

### 2.遍历单层目录

```cpp
// 打开目录
DIR* dir = opendir("/home/test");
struct dirent* ptr = NULL;
// 遍历目录
while( (ptr=readdir(dir)) != NULL)
{
    .......
}
```

- 例：遍历单层目录是不需要递归，求某个指定目录下 mp3 格式文件的个数：

```cpp
int getMp3Num(const char* path) {
    //1.打开目录
    DIR* dir = opendir(dirpath);
    if(dir == NULL) handle_error("opendir");
    //2.遍历当前目录中的文件
    int count = 0;
    while(1) {
        struct dirent* ptr = readdir(dir);
        if(ptr == NULL) {
            printf("目录读完了...\n");
            break;
        }
        if(ptr->d_type == DT_REG) {
            char* p = strstr(ptr->d_name, ".mp3");
            if(p != NULL && *(p+4) == '\0') {
                count++;
                printf("file %d: %s\n", count, ptr->d_name);
            }
        }
    }
    //3.关闭目录
    closedir(dir);
    return count;
}
```

### 3.遍历多层目录

递归方式进行遍历，递归结束的条件：遍历的文件如果不是目录类型就结束递归。

```cpp
int getMp3Num(const char* path) {
    //1.打开目录
    DIR* dir = opendir(dirpath);
    if(dir == NULL) handle_error("opendir");
    //2.遍历当前目录
    struct dirent* ptr = NULL;
    int count = 0;
    while((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".")==0 || strcmp(ptr->d_name, "..") == 0) continue;//若为特殊目录 则跳过不处理
        if (ptr->d_type == DT_DIR) {
            /* 读到的当前文件是目录 */
            char newPath[1024];
            sprintf(newPath, "%s/%s", path, ptr->d_name);
            count += getMp3Num(newPath);//递归读取
        } else if(ptr->d_type == DT_REG) {
            /* 读到的当前文件是普通文件 */
            char* p = strstr(ptr->d_name, ".mp3");
            if(p != NULL && *(p+4) == '\0') {
                count++;
                printf("%s/%s\n", path, ptr->d_name);
            }
        }
    }
    //3.关闭目录
    closedir(dir);
    return count;
}
```



### 4.scandir函数

scandir() 函数进行目录的遍历，只遍历指定目录，不进入到子目录中进行递归遍历，参数涉及到三级指针和回调函数的使用。

```cpp
#include <dirent.h> 
int scandir(const char *dirp,
            struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*compar)(const struct dirent **, const struct dirent **));
int alphasort(const struct dirent **a, const struct dirent **b);
int versionsort(const struct dirent **a, const struct dirent **b);
```

1. dirp: 需要遍历的目录的名字
2. namelist: 三级指针，传出参数，需要在指向的地址中存储遍历目录得到的所有文件的信息
    - 在函数内部会给这个指针指向的地址分配内存，要注意在程序中释放内存
3. filter: 函数指针，指针指向的函数就是回调函数，需要在自定义函数中指定如果过滤目录中的文件
    - 如果不对目录中的文件进行过滤, 该函数指针指定为NULL即可
    - 如果自己指定过滤函数, 满足条件要返回1, 否则返回 0
4. compar: 函数指针，对过滤得到的文件进行排序，可以使用提供的两种排序方式
    - alphasort: 根据文件名进行排序
    - versionsort: 根据版本进行排序
5. 返回值：函数执行成功返回找到的匹配成功的文件的个数，如果失败返回 - 1

<mark>scandir文件过滤</mark>：

scandir() 可以自定义文件的过滤方式，将过滤函数的地址传递给 scandir () 的第三个参数，过滤函数的原型：

```cpp
// 函数的参数就是遍历的目录中的子文件对应的结构体
int (*filter)(const struct dirent *);
```

基于这个函数指针定义的函数可称为回调函数，

这个函数是通过 scandir () 调用的，因此这个函数的实参也是由 scandir () 函数提供的，作为回调函数的编写人员，只需要搞明白这个参数的含义是什么，然后在函数体中直接使用即可。

```cpp
int isMp3(const struct dirent *ptr) {
    if (ptr->d_type == DT_REG) {
        char* p = strstr(ptr->d_name, ".mp3");
        if(p != NULL && *(p+4) == '\0') return 1;
    }
    return 0;
}
```

<mark>遍历文件目录</mark>：

```cpp
int isMp3(const struct dirent *ptr) {
    if (ptr->d_type == DT_REG) {
        char* p = strstr(ptr->d_name, ".mp3");
        if(p != NULL && *(p+4) == '\0') return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct dirent **namelist = NULL;
    int num = scandir(argv[1], &namelist, isMp3, alphasort);
    for(int i=0; i<num; ++i) {
        printf("file %d: %s\n", i, namelist[i]->d_name);
        free(namelist[i]);
    }
    free(namelist);
    return 0;
}
```

最后再解析一下 scandir () 的第二个参数，传递的是一个二级指针的地址:

```cpp
struct dirent **namelist = NULL;
int num = scandir(argv[1], &namelist, isMp3, alphasort);
```

那么在这个 namelist 中存储的什么类型的数据呢？也就是 `struct dirent **namelist` 指向的什么类型的数据？

答案：指向的是一个指针数组 struct dirent *namelist[]

- 数组元素的个数就是遍历的目录中的文件个数，
- 数组的每个元素都是指针类型: struct dirent *, 指针指向的地址是有 scandir () 函数分配的，因此在使用完毕之后需要释放内存。

























