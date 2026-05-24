#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>

// 缓冲区结构体
struct Buffer
{
    char* buf;    // 缓冲区指针
    size_t size;  // 已存储数据长度
};

// 回调函数：接收服务器返回的数据
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realSize = size * nmemb;
    struct Buffer *buffer = (struct Buffer *)userp;
    static bool isWrite = true;

    //爬取照片// 先判断是否遇到 \r\n，再开启写入
//    	if (realSize == 2 && strncmp((char*)contents, "\r\n", 2) == 0)
//    	{
//    	    isWrite = true;
//    	    return realSize;
//   	}

    // 开启写入后才追加数据
    if (isWrite && realSize > 0)
    {
        char *tmp = (char*)realloc(buffer->buf, buffer->size + realSize + 1);
        if (tmp != NULL)
        {
            buffer->buf = tmp;
            memcpy(buffer->buf + buffer->size, contents, realSize);
            buffer->size += realSize;
            buffer->buf[buffer->size] = '\0'; // 字符串结束符
        }
        else
        {
            printf("realloc buffer failed\n");
            return 0; // 分配失败返回0，终止传输
        }
    }

    return realSize;
}

int main(void)
{
    CURL *curl;
    CURLcode res;

    // 设置控制台UTF-8编码
    system("chcp 65001");

    struct Buffer buffer = {0};

    // 1. 全局初始化
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // 2. 创建curl句柄
    curl = curl_easy_init();
    if (curl != NULL)
    {
        // 3. 设置请求URL
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com/");

        // 4. 设置回调与用户数据
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        // 5. 执行请求
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl 请求失败: %s\n", curl_easy_strerror(res));
        }
        else
        {
             printf("ok\n");
             FILE* fp = fopen("D:\\CC\\text4.html","wb");
             fwrite(buffer.buf,sizeof(char),buffer.size,fp);
             fclose(fp);
        }

        // 6. 释放句柄
        curl_easy_cleanup(curl);
    }

    // 释放缓冲区内存
    free(buffer.buf);
    buffer.buf = NULL;

    // 7. 全局清理
    curl_global_cleanup();

    return 0;
}
