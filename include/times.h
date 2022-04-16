#ifndef _TIMES_H_
#define _TIMES_H_

#include<iostream>
#include<memory>
#include<cstdio>
#include<cstdlib>
#include<ctime>
#include<chrono>
#include<sys/time.h>
#include<sys/times.h>
#include<functional>
#include<mutex>
#include<cstring>
#include<unistd.h>

class Time{
  public:
       struct time_s{
         long second;
         long msecond;
         int gmtofff;
      };
       struct date_s{
         int second;
         int minute;
         int hour;
         int monthday;
         int weekday;
         int month;
         int year;
     }; 
     typedef std::shared_ptr<time_s>time;
     typedef std::shared_ptr<date_s>date;
       ~Time();
  private:
      std::mutex m_mutex;
      Time();
  private:
      static Time* time_manager;
      typedef std::chrono::system_clock timetype;
      int m_tck;
      struct tms m_begin_t,m_end_t;
      clock_t m_begin,m_end;//程序运行时间
      time m_computer_time;//计算机时间
      date m_local_date;//本地时间
      bool m_update;  
      void update();
  public:
       static Time* GetTimeManager();
       void now_time();
       uint get_second();
       uint get_minute();
       uint get_hour();
       uint get_monthday();
       uint get_weekday();
       uint get_month();
       uint get_year();
       int get_date();
       std::string get_formate_date();//将本地日期转化为字符串形式
       uint get_lasttime(struct time_s* &lasttime);
       uint get_elapsetime();//获取当前程序执行时间 
       static uint64_t GetElapseMs(std::function<void()> test);//测试一个函数运行所需要的时间  
       static uint64_t GetCurrentMs(); 
};







#endif