#include <stdio.h>
#include <curl/curl.h>
#include "cJSON.h"
int i;
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) return 0;
    mem->memory = ptr;
    memcpy(&mem->memory[mem->size], contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = '\0';
    return realsize;
}

char *http_get(const char *url)
{
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;
    if (!chunk.memory) return NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (curl_handle)
    {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/120.0.0.0 Safari/537.36");
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_ENCODING, "UTF-8");
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 8L);

        printf("[DEBUG] Requesting: %s\n", url);
        res = curl_easy_perform(curl_handle);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "[ERROR] Request failed: %s\n", curl_easy_strerror(res));
            free(chunk.memory);
            chunk.memory = NULL;
        }
        else
        {
            printf("[DEBUG] Request success, response size: %zu bytes\n", chunk.size);
        }
        curl_easy_cleanup(curl_handle);
    }
    curl_global_cleanup();
    return chunk.memory;
}

void stamp_to_time(long long stamp, char *buf)
{
    time_t t = (time_t)stamp;
    struct tm *lt = localtime(&t);
    if (lt)
    {
        strftime(buf, 32, "%Y-%m-%d %H:%M:%S", lt);
    }
    else
    {
        strcpy(buf, "Unknown Time");
    }
}

char *read_user_id()
{
    char *handle = (char *)malloc(128);
    if (!handle) return NULL;
    printf("enter Codeforces ID: ");
    scanf("%127s", handle);
    return handle;
}

void fetch_user_info(const char *handle)
{
    char url[256];
    if (snprintf(url, sizeof(url), "https://codeforces.com/api/user.info?handles=%s", handle) >= sizeof(url))
    {
        fprintf(stderr, "[ERROR] URL too long\n");
        return;
    }

    char *json = http_get(url);
    if (!json)
    {
        fprintf(stderr, "[ERROR] Failed to get user info, json is NULL\n");
        return;
    }

    printf("\n[DEBUG] Raw API data:\n%s\n", json);

    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        fprintf(stderr, "[ERROR] JSON parse failed\n");
        free(json);
        return;
    }

    cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
    if (!status || strcmp(status->valuestring, "OK") != 0)
    {
        fprintf(stderr, "[ERROR] API status error: %s\n", status ? status->valuestring : "Unknown");
        cJSON_Delete(root);
        free(json);
        return;
    }

    cJSON *res = cJSON_GetObjectItemCaseSensitive(root, "result");
    if (!res || !cJSON_IsArray(res) || cJSON_GetArraySize(res) == 0)
    {
        fprintf(stderr, "[ERROR] User data not found\n");
        cJSON_Delete(root); free(json);
        return;
    }
    cJSON *u = cJSON_GetArrayItem(res, 0);
    if (!u)
    {
        fprintf(stderr, "[ERROR] Empty user data\n");
        cJSON_Delete(root); free(json);
        return;
    }

    char rank[64] = "";
    char maxrank[64] = "";
    int rating = 0, maxrat = 0;
    const char *city = "None";

    cJSON *p = cJSON_GetObjectItemCaseSensitive(u, "rank");
    if (p && cJSON_IsString(p)) strncpy(rank, p->valuestring, sizeof(rank)-1);
    p = cJSON_GetObjectItemCaseSensitive(u, "maxRank");
    if (p && cJSON_IsString(p)) strncpy(maxrank, p->valuestring, sizeof(maxrank)-1);
    p = cJSON_GetObjectItemCaseSensitive(u, "rating");
    if (p && cJSON_IsNumber(p)) rating = p->valueint;
    p = cJSON_GetObjectItemCaseSensitive(u, "maxRating");
    if (p && cJSON_IsNumber(p)) maxrat = p->valueint;
    p = cJSON_GetObjectItemCaseSensitive(u, "city");
    if (p && cJSON_IsString(p)) city = p->valuestring;

    printf("Account: %s\n", handle);
    printf("Current Rank: %s\n", rank[0] ? rank : "None");
    printf("Max Rank: %s\n", maxrank[0] ? maxrank : "None");
    printf("Current Rating: %d\n", rating);
    printf("Max Rating: %d\n", maxrat);
    printf("City: %s\n", city);

    cJSON_Delete(root);
    free(json);
}

void fetch_contest_list(const char *handle)
{
    char url[256];
    if (snprintf(url, sizeof(url), "https://codeforces.com/api/user.rating?handle=%s", handle) >= sizeof(url))
    {
        fprintf(stderr, "[ERROR] URL too long\n");
        return;
    }

    char *json = http_get(url);
    if (!json)
    {
        fprintf(stderr, "[ERROR] Failed to get contest record, json is NULL\n");
        return;
    }

    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        fprintf(stderr, "[ERROR] JSON parse failed\n");
        free(json);
        return;
    }

    cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
    if (!status || strcmp(status->valuestring, "OK") != 0)
    {
        fprintf(stderr, "[ERROR] API status error: %s\n", status ? status->valuestring : "Unknown");
        cJSON_Delete(root);
        free(json);
        return;
    }

    cJSON *arr = cJSON_GetObjectItemCaseSensitive(root, "result");
    if (!arr || !cJSON_IsArray(arr))
    {
        fprintf(stderr, "[ERROR] Invalid contest data format\n");
        cJSON_Delete(root); free(json);
        return;
    }

    FILE *fp = fopen("contest_data.txt", "w");
    if (!fp)
    {
        fprintf(stderr, "[ERROR] Cannot open file contest_data.txt\n");
        cJSON_Delete(root);
        free(json);
        return;
    }

    int cnt = cJSON_GetArraySize(arr);
    printf("\n===== Contest Record (Total: %d) =====\n", cnt);
    printf("No. Contest Name                Rating Change\n");
    for (i = 0; i < cnt; i++)
    {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        if (!item) continue;
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "contestName");
        cJSON *old = cJSON_GetObjectItemCaseSensitive(item, "oldRating");
        cJSON *now = cJSON_GetObjectItemCaseSensitive(item, "newRating");
        if (!name || !old || !now) continue;
        int diff = now->valueint - old->valueint;
        printf("%d %-30s %+d\n", i+1, name->valuestring, diff);
        fprintf(fp, "%s %d %d %d\n", name->valuestring, old->valueint, now->valueint, diff);
    }
    fclose(fp);
    printf("Data exported to contest_data.txt\n");

    cJSON_Delete(root);
    free(json);
}

void fetch_problem_stat(const char *handle)
{
    char url[256];
    if (snprintf(url, sizeof(url), "https://codeforces.com/api/user.status?handle=%s&count=2000", handle) >= sizeof(url))
    {
        fprintf(stderr, "[ERROR] URL too long\n");
        return;
    }

    char *json = http_get(url);
    if (!json)
    {
        fprintf(stderr, "[ERROR] Failed to get submission record, json is NULL\n");
        return;
    }

    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        fprintf(stderr, "[ERROR] JSON parse failed\n");
        free(json);
        return;
    }

    cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
    if (!status || strcmp(status->valuestring, "OK") != 0)
    {
        fprintf(stderr, "[ERROR] API status error: %s\n", status ? status->valuestring : "Unknown");
        cJSON_Delete(root);
        free(json);
        return;
    }

    cJSON *arr = cJSON_GetObjectItemCaseSensitive(root, "result");
    if (!arr || !cJSON_IsArray(arr))
    {
        fprintf(stderr, "[ERROR] Invalid submission data format\n");
        cJSON_Delete(root); free(json);
        return;
    }

    int all_sub = 0, ac_cnt = 0;
    int total = cJSON_GetArraySize(arr);
    char timestr[32];

    printf("\n===== Submission Record =====\n");
    for (i = 0; i < total; i++)
    {
        cJSON *it = cJSON_GetArrayItem(arr, i);
        if (!it) continue;
        cJSON *cont = cJSON_GetObjectItemCaseSensitive(it, "contestId");
        if (!cont || cJSON_IsNull(cont)) continue;

        all_sub++;
        cJSON *ver = cJSON_GetObjectItemCaseSensitive(it, "verdict");
        cJSON *prob = cJSON_GetObjectItemCaseSensitive(it, "problem");
        cJSON *pidx = prob ? cJSON_GetObjectItemCaseSensitive(prob, "index") : NULL;
        cJSON *stamp = cJSON_GetObjectItemCaseSensitive(it, "creationTimeSeconds");

        int ok = 0;
        if (ver && strcmp(ver->valuestring, "OK") == 0)
        {
            ac_cnt++; ok = 1;
        }
        if(stamp) stamp_to_time(stamp->valueint, timestr);
        else strcpy(timestr, "Unknown Time");
        printf("Problem:%s Status:%s Time:%s\n",
            (pidx && cJSON_IsString(pidx)) ? pidx->valuestring : "--",
            ok?"AC":"WA", timestr);
    }
    double rate = 0.0;
    if(all_sub > 0) rate = 1.0 * ac_cnt / all_sub;
    printf("\nStats: Total Submit:%d Accepted:%d Pass Rate:%.2f%%\n",
        all_sub, ac_cnt, rate*100);

    cJSON_Delete(root);
    free(json);
}

int main(void)
{
    system("chcp 65001 > nul");
    SetConsoleOutputCP(65001);
    printf("==== Codeforces Crawler ====\n");
    char *user = read_user_id();
    if (!user)
    {
        fprintf(stderr, "[ERROR] Failed to read user ID\n");
        return 1;
    }

    fetch_user_info(user);
    fetch_contest_list(user);
    fetch_problem_stat(user);

    free(user);
    printf("\nProgram finished\n");
    return 0;
}
