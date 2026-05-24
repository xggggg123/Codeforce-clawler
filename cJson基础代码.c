#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

int main() {
    // ====================== 1. 构建 JSON ======================
    // 创建根对象 {}
    cJSON* root = cJSON_CreateObject();

    int i;
    // 添加键值对（字符串/数字/布尔/null）
    cJSON_AddStringToObject(root, "name", "张三");
    cJSON_AddNumberToObject(root, "age", 20);
    cJSON_AddBoolToObject(root, "student", 1);  // 1=true, 0=false
    cJSON_AddNullToObject(root, "remark");

    // 创建数组 []
    cJSON* hobby = cJSON_CreateArray();
    // 往数组添加元素
    cJSON_AddItemToArray(hobby, cJSON_CreateString("篮球"));
    cJSON_AddItemToArray(hobby, cJSON_CreateString("编程"));
    // 把数组加入根对象
    cJSON_AddItemToObject(root, "hobby", hobby);

    // 打印格式化 JSON（带换行缩进，好看）
    char* json_str = cJSON_Print(root);
    printf("=== 生成的 JSON ===\n%s\n", json_str);

    // ====================== 2. 解析 JSON ======================
    cJSON* parse_root = cJSON_Parse(json_str);
    if (parse_root == NULL) {
        printf("JSON 解析失败！\n");
        return -1;
    }

    // 读取字段
    cJSON* name = cJSON_GetObjectItemCaseSensitive(parse_root, "name");
    cJSON* age = cJSON_GetObjectItemCaseSensitive(parse_root, "age");
    cJSON* hobby_arr = cJSON_GetObjectItemCaseSensitive(parse_root, "hobby");

    printf("\n=== 解析结果 ===\n");
    if (cJSON_IsString(name)) printf("姓名：%s\n", name->valuestring);
    if (cJSON_IsNumber(age)) printf("年龄：%d\n", age->valueint);

    // 遍历数组
    if (cJSON_IsArray(hobby_arr)) {
        int arr_size = cJSON_GetArraySize(hobby_arr);
        printf("爱好（共%d个）：", arr_size);
        for (i = 0; i < arr_size; i++) {
            cJSON* item = cJSON_GetArrayItem(hobby_arr, i);
            if (cJSON_IsString(item)) {
                printf("%s ", item->valuestring);
            }
        }
        printf("\n");
    }

    // ====================== 3. 释放内存 ======================
    free(json_str);        // 释放 cJSON_Print 生成的字符串
    cJSON_Delete(root);    // 释放整个 JSON 树
    cJSON_Delete(parse_root);

    return 0;
}
