#ifndef __TIMER_H
#define __TIMER_H

#include "types.h"
#include "spinlock.h"

extern struct spinlock tickslock;
extern uint ticks;

// 时间结构
struct rtc_time {
    int year;   // 年份
    int month;  // 月份 (1-12)
    int day;    // 日期 (1-31)
    int hour;   // 小时 (0-23)
    int min;    // 分钟 (0-59)
    int sec;    // 秒 (0-59)
};

void timerinit();
void set_next_timeout();
void timer_tick();
void rtc_get_time(struct rtc_time *time);  // 获取当前时间
uint64 get_current_time_ns();               // 获取当前时间（纳秒）
uint64 get_current_time_s();                // 获取当前时间（秒）
#endif
