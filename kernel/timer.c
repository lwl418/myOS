// Timer Interrupt handler

#include "include/types.h"
#include "include/param.h"
#include "include/riscv.h"
#include "include/sbi.h"
#include "include/spinlock.h"
#include "include/timer.h"
#include "include/printf.h"
#include "include/proc.h"
#include "include/syscall.h"

struct spinlock tickslock;
uint ticks;

// 系统启动时间：从Makefile中传入的实时本地时间
#ifndef SYSTEM_START_YEAR
#define SYSTEM_START_YEAR 2026
#endif
#ifndef SYSTEM_START_MONTH
#define SYSTEM_START_MONTH 1
#endif
#ifndef SYSTEM_START_DAY
#define SYSTEM_START_DAY 1
#endif
#ifndef SYSTEM_START_HOUR
#define SYSTEM_START_HOUR 0
#endif
#ifndef SYSTEM_START_MIN
#define SYSTEM_START_MIN 0
#endif
#ifndef SYSTEM_START_SEC
#define SYSTEM_START_SEC 0
#endif

// 系统启动时间：从Makefile中传入的实时本地时间
static int system_start_year = SYSTEM_START_YEAR;
static int system_start_month = SYSTEM_START_MONTH;
static int system_start_day = SYSTEM_START_DAY;
static int system_start_hour = SYSTEM_START_HOUR;
static int system_start_min = SYSTEM_START_MIN;
static int system_start_sec = SYSTEM_START_SEC;

// 时钟频率：QEMU中rdtime的默认频率是10MHz
#define CLOCK_FREQUENCY 10000000ULL // 10,000,000 Hz

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

// 获取当前时间（纳秒）
uint64 get_current_time_ns()
{
    return r_time() * (1000000000ULL / CLOCK_FREQUENCY); // 将周期转换为纳秒
}

// 获取当前时间（秒）
uint64 get_current_time_s()
{
    return r_time() / CLOCK_FREQUENCY; // 将周期转换为秒
}

// 获取当前日期和时间
void rtc_get_time(struct rtc_time *time)
{
    if (time == NULL)
        return;

    // 计算自系统启动以来的秒数
    uint64 elapsed_sec = get_current_time_s();

    // 初始化时间为系统启动时间
    time->year = system_start_year;
    time->month = system_start_month;
    time->day = system_start_day;
    time->hour = system_start_hour;
    time->min = system_start_min;
    time->sec = system_start_sec;

    // 添加经过的秒数
    time->sec += elapsed_sec;

    // 转换为正确的时间格式
    while (time->sec >= 60)
    {
        time->sec -= 60;
        time->min++;
    }

    while (time->min >= 60)
    {
        time->min -= 60;
        time->hour++;
    }

    while (time->hour >= 24)
    {
        time->hour -= 24;
        time->day++;
    }

    // 处理月份和年份
    while (1)
    {
        int days = days_in_month[time->month];
        if (time->month == 2 && is_leap_year(time->year))
        {
            days++;
        }

        if (time->day <= days)
        {
            break;
        }

        time->day -= days;
        time->month++;

        if (time->month > 12)
        {
            time->month = 1;
            time->year++;
        }
    }
}

void timerinit()
{
    initlock(&tickslock, "time");
#ifdef DEBUG
    printf("timerinit\n");
#endif
}

void set_next_timeout()
{
    // There is a very strange bug,
    // if comment the `printf` line below
    // the timer will not work.

    // this bug seems to disappear automatically
    // printf("");
    sbi_set_timer(r_time() + INTERVAL);
}

void timer_tick()
{
    acquire(&tickslock);
    ticks++;
    wakeup(&ticks);
    release(&tickslock);
    set_next_timeout();
}

// 系统调用：设置系统时间
uint64 sys_settime(void)
{
    int year, month, day, hour, min, sec;

    // 从用户进程获取参数
    if(argint(0, &year) < 0 || argint(1, &month) < 0 || argint(2, &day) < 0 ||
       argint(3, &hour) < 0 || argint(4, &min) < 0 || argint(5, &sec) < 0)
        return -1;

    // 简单的时间有效性检查
    if(month < 1 || month > 12 || day < 1 || day > 31 ||
       hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 || sec > 59)
        return -1;

    // 更新全局系统时间变量
    system_start_year = year;
    system_start_month = month;
    system_start_day = day;
    system_start_hour = hour;
    system_start_min = min;

    // 我们需要将当前已经流逝的秒数减去，这样系统时间才能正确反映设置的时间
    uint64 elapsed_sec = get_current_time_s();
    system_start_sec = sec - elapsed_sec;

    return 0;
}
