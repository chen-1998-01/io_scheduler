#include"../../include/times.h"
#include<utility>



Time* Time::time_manager=nullptr;

Time* Time::GetTimeManager(){
  if(!time_manager)
    time_manager=new Time();
  return time_manager;  
}

Time::Time():
m_update(false),m_local_date(nullptr),m_computer_time(nullptr){
   m_tck=sysconf(_SC_CLK_TCK);
   m_begin=times(&m_begin_t);
   m_end=m_begin;
   now_time();
}

Time::~Time(){
  delete(time_manager);
  time_manager=nullptr;
}

void Time::now_time(){
  std::unique_lock<std::mutex> lock(m_mutex);
  m_computer_time=std::make_shared<time_s>();
  m_local_date=std::make_shared<date_s>();
  struct timeval time_val;
  struct timezone time_zone;
  gettimeofday(&time_val,&time_zone);
  m_computer_time->second=time_val.tv_sec;
  m_computer_time->msecond=time_val.tv_usec;
  m_computer_time->gmtofff=(time_zone.tz_minuteswest)*60;
  //获取计算机时间戳
  time_t data_value=timetype::to_time_t(std::chrono::system_clock::now());
  struct tm* data=localtime(&data_value);
  m_local_date->second=data->tm_sec;
  m_local_date->minute=data->tm_min;
  m_local_date->hour=data->tm_hour+16;
  m_local_date->weekday=data->tm_wday;
  m_local_date->monthday=data->tm_mday;
  m_local_date->month=data->tm_mon+1;
  m_local_date->year=data->tm_year+1900;
  //获取本地系统时间
  m_update=true; 
}

void Time::update(){
  std::unique_lock<std::mutex> lock(m_mutex);
  time_t data_value=timetype::to_time_t(std::chrono::system_clock::now());
  struct tm* data=localtime(&data_value);
  m_local_date->second=data->tm_sec;
  m_local_date->minute=data->tm_min;
  m_local_date->hour=data->tm_hour+15;
  m_local_date->weekday=data->tm_wday;
  m_local_date->monthday=data->tm_mday;
  m_local_date->month=data->tm_mon+1;
  m_local_date->year=data->tm_year+1900;
  m_update=true;
  if(m_local_date->hour>=24){
    m_local_date->hour-=24;
    m_local_date->weekday += 1;
    m_local_date->monthday += 1;
  }

}

uint Time::get_second(){
   update();
   return (uint)m_local_date->second;    
}
uint Time::get_minute(){
  update();
  return (uint)m_local_date->minute;
}
uint Time::get_hour(){
  update();
  return (uint)m_local_date->hour;
}
uint Time::get_monthday(){
  update();
  return (uint)m_local_date->monthday; 
}
uint Time::get_weekday(){
   update();
   return (uint)m_local_date->weekday;
}
uint Time::get_month(){
   update();
   return (uint)m_local_date->month;
}
uint Time::get_year(){
  update();
  return (uint)m_local_date->year;
}

int Time::get_date(){
  char date[64];
  bzero(date,sizeof(date));
  update();
  sprintf(date,"%04d%02d%02d",m_local_date->year,m_local_date->month,m_local_date->monthday);  
  return atoi(date);
}

std::string Time::get_formate_date(){
   char date[64];
   bzero(date,sizeof(date));
   update();
   sprintf(date,"%04d-%02d-%02d-%02d:%02d:%02d",m_local_date->year,m_local_date->month,m_local_date->monthday,m_local_date->hour,m_local_date->minute,m_local_date->second);
   std::string _date=date;
   return _date;   
}


uint Time::get_lasttime(struct time_s* &lasttime){
   update();
   return (m_computer_time->second+m_computer_time->msecond/1000000)-(lasttime->second+lasttime->msecond/1000000);
}

uint Time::get_elapsetime(){
  m_end=times(&m_end_t);
  return (m_end-m_begin)/(double)m_tck; 
}

uint64_t Time::GetElapseMs(std::function<void()> test){
  std::chrono::steady_clock::time_point begin_time;
  std::chrono::steady_clock::time_point end_time;
  begin_time=std::chrono::steady_clock::now();
  test();  
  end_time=std::chrono::steady_clock::now();
  uint elapse;
  elapse=std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(end_time-begin_time).count();
  return elapse;
}

uint64_t Time::GetCurrentMs(){
  struct timeval time_val;
  gettimeofday(&time_val,NULL);
  return time_val.tv_sec*1000ul+time_val.tv_usec/1000;   
}