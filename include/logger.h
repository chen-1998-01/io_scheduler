#ifndef _LOGGER_H_
#define _LOGGER_H_

//**日志事件的设计:主要包含事件发生的时间戳，文件地址，事件等级
                 //所处的线程号、线程名称和协程号，采用事件包裹器进行包裹
                 //每个日志事件都关联一个日志器来记录** 

//**日志器的设计:主要包含日志记录的输出地（包含控制台输出和文本输出）、
               //采用的记录的格式，需要对日志格式进行解释，生成不同的条目。**




#include<iostream>
#include<fstream>
#include<sstream>
#include<cstdint>
#include<cstdarg>
#include<cstring>
#include<cstdio>
#include<cstdlib>
#include<cassert>
#include<string>
#include<vector>
#include<list>
#include<map>
#include<functional>
#include<memory>
#include<mutex>
#include"times.h"
#include"file.h"


#define logger_eventwrap(logger,level)\
if(level>=logger->get_level())\
    logevent_wrap::GetLogeventWrap()->wrap(logevent::_event(new logevent(__FILE__,__LINE__,\
Thread::get_this_threadname(),Thread::get_thread_id(),coroutine::get_coroutine_id(),__func__,\
Time::GetTimeManager()->get_elapsetime(),Time::GetTimeManager()->get_formate_date(),level,logger)))->get_stream()

#define logger_debug_eventstream(logger) logger_eventwrap(logger,logger_level::debug)

#define logger_warn_eventstream(logger) logger_eventwrap(logger,logger_level::warn)

#define logger_inform_eventstream(logger) logger_eventwrap(logger,logger_level::inform)

#define logger_error_eventstream(logger) logger_eventwrap(logger,logger_level::error)

#define logger_crash_eventstream(logger) logger_eventwrap(logger,logger_level::crash)

#define root_logger (logger_manager::GetLoggerManager()->get_root_logger()) 

#define files_logger(name) (logger_manager::GetLoggerManager()->get_logger(#name))

#define ConsoleLog() (root_logger->log(logger_level::debug,logevent_wrap::GetLogeventWrap()->get_event()))

class logger;

  //日志等级
	class logger_level {
	  public:
		  enum level {
			unknow = 0,
			debug = 1,
			inform = 2,
			warn = 3,
			error = 4,
			crash = 5,
		};
		  static std::string tostring(logger_level::level);
	};

//日志事件
class logevent:public std::enable_shared_from_this<logevent>{
	public:  
		logevent(const std::string&,uint32_t,const std::string&,uint32_t,uint32_t,const char*,uint32_t,const std::string&,logger_level::level,std::shared_ptr<logger>);
        //文件名，行号，线程名，线程号，协程号，程序运行时间，当地时间，日志
		typedef std::shared_ptr<logevent> _event;
	private:
		std::string m_file_name;
		uint32_t m_line_number;
		std::string m_thread_name;
		uint32_t m_thread_id;
		uint32_t m_coroutine_id;
		char* m_func;
		uint32_t m_elapse_second;
		std::string m_localtime;
		logger_level::level m_level;
		std::stringstream m_stream;
        std::shared_ptr<logger>m_logger;//记录日志的日志器
	public:
		void set_filename(const std::string&);
		void set_threadname(const std::string&);
		void set_threadid(uint32_t);
		void set_coroutineid(uint32_t);
		void set_elapsesecond(uint32_t);
		void set_time(const std::string&);
		void set_level(logger_level::level _level);
		std::string get_filename()const {
			return m_file_name;
		}
		uint16_t get_linenumber()const {
			return m_line_number;
		}
		std::string get_threadname()const {
			return m_thread_name;
		}
		uint32_t get_threadid()const {
			return m_thread_id;
		}
		uint32_t get_coroutineid()const {
			return m_coroutine_id;
		}
		uint32_t get_elapse_second()const {
			return m_elapse_second;
		}
		std::string get_func(){
			std::string _func(m_func);
			return _func;
		}
		std::string get_localtime()const {
			return m_localtime;
		}
		logger_level::level get_level()const {
			return m_level;
		}
		std::shared_ptr<logger> get_logger()const {
			return m_logger;
		}
		std::string get_content()const {
			return m_stream.str();
		}
		std::stringstream& get_stringstream(){
			return m_stream;
		}
		void log();
	};


  //日志事件包裹器
class logevent_wrap {
  private:
      std::mutex m_mutex;
      int m_listnum;
	  logevent::_event m_event;  //当前事件
      std::list<logevent*> m_events;  //当天事件
      std::map<int,std::list<logevent*> > m_recordings;//事件备忘录
      Time* m_localtime;
      uint m_month;
      uint m_day;
  private:
      logevent_wrap(logevent::_event event=nullptr): 
	    m_listnum(0),m_event(event),m_localtime(nullptr),m_month(0),m_day(0)
	  {
        updata();
        if(event)
           m_events.push_back(event.get());
	  }  
      void updata();//更新备忘录 	
	public:
		typedef std::shared_ptr<logevent_wrap>_wrap;
		static logevent_wrap* m_wrap;
		static logevent_wrap* GetLogeventWrap();
		logevent::_event get_event(){
			return m_event;
		}
		std::stringstream& get_stream() {
			return m_event->get_stringstream();
		}
		logevent_wrap* wrap(logevent::_event event){
			m_event=event;
			updata();
			m_events.push_back(event.get());
			return this;
	}//包裹事件
    void requiry(std::string&);//从备忘录获取信息   
	};


  //日志格式
class logger_format {
	public:
		logger_format(const std::string& _pattern);
		~logger_format(){};		 
		typedef std::shared_ptr<logger_format> _format;
	private:
		std::string m_pattern;
		std::mutex m_mutex;
	public:
	    std::ostream& log(std::ostream&,std::shared_ptr<logger>,logger_level::level, logevent::_event);
	public:
        void explain();//根据不同的字符进行解码，生成不同的item，包含不同的信息，然后逐个输出各自所代表的信息
		class logger_item {
		  protected:
		     std::mutex m_mutex;	
		  public:
			  virtual ~logger_item() {};
			  typedef std::shared_ptr<logger_item> _item;
		      virtual void log(std::ostream&,std::shared_ptr<logger>,logger_level::level, logevent::_event) = 0;
		  };
	private:
		std::list<logger_item::_item> m_items;
};


//日志输出地，主要有系统日志输出和文本日志输出
class logger_appender{
	public:
		typedef std::shared_ptr<logger_appender> _appender;
	protected:
		logger_level::level m_level;
		logger_format::_format m_format;
		bool isempty_format;
		std::mutex m_mutex;
	public:
        virtual ~logger_appender(){};
		void set_format(logger_format::_format);	
		virtual void log(std::shared_ptr<logger>,logger_level::level, logevent::_event)=0;
	};

class console_appender :public logger_appender{
	public:
		console_appender();
        ~console_appender();
		void log(std::shared_ptr<logger>,logger_level::level, logevent::_event)override;
	};

class file_appender :public logger_appender {
	private:
        std::fstream* m_filestream;
	public:
        file_appender();
		file_appender(const std::string& _filename);
        ~file_appender();
		void open(const std::string&);
		void log(std::shared_ptr<logger>,logger_level::level, logevent::_event)override;//记录日志
	};


   //*日志器,包含系统日志器和文件日志器 
class logger: public std::enable_shared_from_this<logger> {
    friend class logger_manager;  
	protected:
		std::string m_name;
		logger_level::level m_level;
		logger_format::_format m_format;
		std::list<logger_appender::_appender>m_appender;//日志输出
		std::mutex m_mutex;
	protected:
		void set_level(logger_level::level _level) {
			m_level = _level;
		}
		logger();
	public:
		typedef std::shared_ptr<logger>_logger;
		logger(const std::string& _name,logger_level::level _level,std::string _pattern="");
		virtual ~logger(){};
		logger_level::level get_level()const { return m_level; }
		std::string get_name()const { return m_name; }
		void set_format(const std::string& _pattern);
		void log(logger_level::level, logevent::_event);//记录日志
		void add_appender(logger_appender::_appender);
		void delete_appender(logger_appender::_appender);
		void clean_appender();
	};
  
class system_logger:public logger{
     public:
         system_logger(const std::string& _name,logger_level::level _level,std::string _pattern="%C-%T-%f-%T-%l-%T-%F-%T-%s-%T-%e-%N-%T-%t-%T-%c-%T-%L-%T-%S-%T-%e");
         ~system_logger();
};

class file_logger:public logger{
      public:
          file_logger(const std::string& logger_name,logger_level::level _level,const std::string& file_name,std::string _pattern="%C-%T-%f-%T-%l-%T-%F-%T-%s-%e-%N-%T-%t-%T-%c-%T-%S-%e");
          ~file_logger();
      private:
          int m_filenum;
};


  //日志管理器，采用单例模式
class logger_manager{
	private:
        logger_manager();	
		std::mutex m_mutex;
		std::map<std::string, logger::_logger>m_loggers;
		logger::_logger m_root_logger;//系统控制台日志
		static logger_manager* m_logger_manager;
		void load_root_logger();//加载根目录下的日志器
	public:
		static logger_manager* GetLoggerManager();
		~logger_manager();
		void add_logger(logger::_logger);
		void delete_logger(const std::string&);
		void clean_logger();
		logger::_logger get_root_logger(){
			return m_root_logger;
		}
		logger::_logger get_logger(const std::string& name);
	};


#endif
