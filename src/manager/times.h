#ifndef _TIMES_H_
#define _TIMES_H_

#include"memory_pool.h"
#include<ctime>
#include<chrono>
#include<sys/time.h>
#include<functional>

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


class Time{
  public: 
       Time();
       ~Time();
  private:
      class_memory_pool::_pool m_memory;
      std::mutex m_mutex;
  private:
    typedef std::chrono::system_clock timetype;
    struct time_s* m_computer_time;//计算机时间
    struct date_s* m_local_date;//本地时间
    bool m_update;  
    void update();
  public:
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
       uint get_elapsetime(std::function<void()>& test);//测试一个函数运行所需要的时间  
};







#endif