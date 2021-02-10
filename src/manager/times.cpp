#include"times.h"
#include<utility>

Time::Time():
m_memory(nullptr),m_update(false),
m_local_date(nullptr),m_computer_time(nullptr){
   now_time();
}

Time::~Time(){
   m_memory->delete_class<time_s>(m_computer_time);
   m_memory->delete_class<date_s>(m_local_date);
   m_memory.reset();
   m_memory=NULL;
}

void Time::now_time(){
  std::unique_lock<std::mutex> lock(m_mutex);
  m_memory=class_memory_pool::_pool(new class_memory_pool);
  m_computer_time=m_memory->new_class<time_s>();
  m_local_date=m_memory->new_class<date_s>();
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
  m_local_date->hour=data->tm_hour;
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
  m_local_date->hour=data->tm_hour;
  m_local_date->weekday=data->tm_wday;
  m_local_date->monthday=data->tm_mday;
  m_local_date->month=data->tm_mon+1;
  m_local_date->year=data->tm_year+1900;
  m_update=true; 
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
   sprintf(date,"%04d-%02d-%02d-%02d-%02d-%02d-weekday:%1d\n",\
   m_local_date->year,m_local_date->month,m_local_date->monthday,m_local_date->hour,m_local_date->minute,m_local_date->second,m_local_date->weekday);
   return std::move(std::string(date));  
}


uint Time::get_lasttime(struct time_s* &lasttime){
   update();
   return (m_computer_time->second+m_computer_time->msecond/1000000)-(lasttime->second+lasttime->msecond/1000000);
}

uint Time::get_elapsetime(std::function<void()>& test){
  std::unique_lock<std::mutex> lock(m_mutex);
  std::chrono::steady_clock::time_point begin_time;
  std::chrono::steady_clock::time_point end_time;
  begin_time=std::chrono::steady_clock::now();
  test();  
  end_time=std::chrono::steady_clock::now();
  uint elapse;
  elapse=std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(end_time-begin_time).count();
  std::cout<<"in total it takes:"<<elapse<<" ms"<<std::endl;
  return elapse;
}


