#include "xv6-user/user.h"

// 系统启动时间：将在Makefile中自动注入
#define SYSTEM_START_YEAR 2023
#define SYSTEM_START_MONTH 12
#define SYSTEM_START_DAY 25
#define SYSTEM_START_HOUR 10
#define SYSTEM_START_MIN 30
#define SYSTEM_START_SEC 0

// 每月的天数
static const int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 判断是否为闰年
static int is_leap_year(int year)
{
    if (year % 4 != 0)
        return 0;
    if (year % 100 != 0)
        return 1;
    if (year % 400 != 0)
        return 0;
    return 1;
}

// 获取当前时间并转换为日期格式
void get_current_time(int *year, int *month, int *day, int *hour, int *min, int *sec)
{
    // 使用uptime()获取系统启动后的秒数 (假设uptime返回的是秒数)
    int elapsed_sec = uptime();

    // 初始化时间为系统启动时间
    *year = SYSTEM_START_YEAR;
    *month = SYSTEM_START_MONTH;
    *day = SYSTEM_START_DAY;
    *hour = SYSTEM_START_HOUR;
    *min = SYSTEM_START_MIN;
    *sec = SYSTEM_START_SEC;

    // 添加经过的秒数
    *sec += elapsed_sec;

    // 转换为正确的时间格式
    while (*sec >= 60)
    {
        *sec -= 60;
        *min += 1;
    }

    while (*min >= 60)
    {
        *min -= 60;
        *hour += 1;
    }

    while (*hour >= 24)
    {
        *hour -= 24;
        *day += 1;
    }

    // 处理月份和年份
    while (1)
    {
        int days = days_in_month[*month];
        if (*month == 2 && is_leap_year(*year))
        {
            days += 1;
        }

        if (*day <= days)
        {
            break;
        }

        *day -= days;
        *month += 1;

        if (*month > 12)
        {
            *month = 1;
            *year += 1;
        }
    }
}

// 打印当前时间
void print_current_time()
{
    int year, month, day, hour, min, sec;

    get_current_time(&year, &month, &day, &hour, &min, &sec);

    // 格式化输出时间
    printf("%04d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, min, sec);
}

int main()
{
    // 测试系统时钟
    printf("Real Time Clock Test\n");
    printf("===================\n");

    // 初始时间
    printf("Initial time: ");
    print_current_time();

    // 延迟5秒
    printf("\nSleeping for 5 seconds...\n");
    sleep(5);

    // 延迟后的时间
    printf("\nTime after sleep: ");
    print_current_time();

    // 再延迟3秒
    printf("\nSleeping for 3 more seconds...\n");
    sleep(3);

    // 再延迟后的时间
    printf("\nFinal time: ");
    print_current_time();

    // 测试uptime系统调用
    int up_time = uptime();
    printf("\nSystem uptime: %d seconds\n", up_time);

    exit(0);
}
