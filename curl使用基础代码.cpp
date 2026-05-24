#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

//contest libcurl传递给我们接收到数据的指针

// size 接收到每个数据的大小

//nmemb  接收到数据总大小

//  userdata 我们自己传递的参数
 
// 回调函数：接收服务器返回的数据


size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    // 取出文件指针
    FILE *fp = (FILE *)userp;
    // 写入文件
    fwrite(contents, size, nmemb, fp);
	// 把数据直接打印到屏幕
    //fwrite(contents, size, nmemb, stdout);
    return size * nmemb;
}

int main(void) {
    CURL *curl;
    CURLcode res;
	//设置控制台编码
	system("chcp 65001");
	 
    // 1. 全局初始化（整个程序只需要调用一次）
    curl_global_init(CURL_GLOBAL_DEFAULT);
	//创建文件
	 FILE *fp = fopen("D:\\CC\\text.html","wb");
	 if(!fp){
	 	goto done;
	 }
    // 2. 创建 curl 句柄
    curl = curl_easy_init();
    if (curl) {
        // 3. 设置要请求的 URL
        curl_easy_setopt(curl, CURLOPT_URL, "https://codeforce.com/api/contest.list");

        // 4. 设置数据接收回调（必须有，否则 curl 不知道怎么处理返回数据）
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);//把从服务器接收数据写入指定文件指针 
        // 5. 执行请求
        res = curl_easy_perform(curl);

        // 检查是否请求失败
        if (res != CURLE_OK) {
            fprintf(stderr, "curl 请求失败: %s\n", curl_easy_strerror(res));
            goto done;
        }
		else{
			printf("ok");
		} 
	}
	done:
    // 6. 释放 curl 句柄
    curl_easy_cleanup(curl);
	fclose(fp);
    // 7. 全局清理
    curl_global_cleanup();

    return 0;
}
