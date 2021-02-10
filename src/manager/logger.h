#ifndef _LOGGER_H_
#define _LOGGER_H_
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
#include"../manager/memory_pool.h"
#include"times.h"
#include"../stream/file_stream.h"

//宏定义，作为接口

#define logger_eventwrap(logger,level)\
if(level>=logger->get_level())\
    logevent_wrap::get_instance()->wrap(logevent::_event(new logevent(__FILE__,__LINE__,\
Thread::get_this_threadname(),Thread::get_this_threadid(),coroutine::get_this_coroutineid(),\
0,time(0),level,logger)))->get_stream()
//定义事件包裹器的宏,需要提供日志器和事件等级两个参数

#define logger_debug_eventstream(logger) logger_eventwrap(logger,logger_level::debug)

#define logger_warn_eventstream(logger) logger_eventwrap(logger,logger_level::warn)

#define logger_inform_eventstream(logger) logger_eventwrap(logger,logger_level::inform)

#define logger_error_eventstream(logger) logger_eventwrap(logger,logger_level::error)

#define logger_crash_eventstream(logger) logger_eventwrap(logger,logger_level::crash)

#define root_logger() (logger_manager::get_instance()->get_root_logger()) 

#define files_logger(name) (logger_manager::get_instance()->get_logger(#name))





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
		logevent(const std::string&,uint32_t,const std::string&,uint32_t,uint32_t,uint32_t,uint64_t,logger_level::level,std::shared_ptr<logger>);
		typedef std::shared_ptr<logevent> _event;
	private:
		std::string m_file_name;
		uint32_t m_line_number;
		std::string m_thread_name;
		uint32_t m_thread_id;
		uint32_t m_coroutine_id;
		uint32_t m_elapse_second;
		uint64_t m_localtime;
		logger_level::level m_level;
		std::stringstream m_stream;
    std::shared_ptr<logger>m_logger;//记录日志的日志器
    //包含事件发生的文件路径，文件所在的行号，事件执行的线程名称，线程号，协程号，事件发生时的时间戳，本地时间，
    //以及事件记录的内容
		class_memory_pool* m_memory_pool;
	public:
		void set_filename(const std::string&);
		void set_threadname(const std::string&);
		void set_threadid(uint32_t);
		void set_coroutineid(uint32_t);
		void set_elapsesecond(uint32_t);
		void set_time(uint64_t);
		void set_level(logger_level::level _level) {
			m_level = _level;
		}

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
		uint64_t get_localtime()const {
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
	class logevent_wrap:public mode::nonauto_singleclass<logevent_wrap> {
	public:
		typedef std::shared_ptr<logevent_wrap>_wrap;
		logevent_wrap(logevent::_event event=nullptr):
      m_listnum(0),m_event(event),m_localtime(nullptr),m_month(0),m_day(0){
        updata();
        if(event)
           m_events.push_back(event.get());
		 }
		logevent::_event get_event(){
			return m_event;
		}
		std::stringstream& get_stream() {
			return m_event->get_stringstream();
		}
		void wrap(logevent::_event event){
			m_event=event;
			updata();
			m_events.push_back(event.get());
		}//包裹事件
    void requiry(std::string&);//从备忘录获取信息
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
    void updata();//更新备忘录    
	};


  //日志格式
	class logger_format {
	public:
		logger_format(const std::string& _pattern);
		~logger_format(){
			if(m_memory_pool){
        delete(m_memory_pool);
				m_memory_pool=nullptr;
			}			 
		}
		typedef std::shared_ptr<logger_format> _format;
	private:
		std::string m_pattern;
		std::mutex m_mutex;
	protected:
	   class_memory_pool* m_memory_pool;
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
	public:
    virtual ~logger_appender(){};
		virtual void set_format(logger_format::_format)=0;
		bool isempty_format;
		virtual void log(std::shared_ptr<logger>,logger_level::level, logevent::_event)=0;
	};

	class console_appender :public logger_appender {
	public:
		console_appender();
    ~console_appender();
		void set_format(logger_format::_format)override;
		void log(std::shared_ptr<logger>,logger_level::level, logevent::_event)override;
	  std::mutex m_mutex;
	};

	class file_appender :public logger_appender {
	private:
    file* m_files;
    std::fstream* m_filestream;
		std::mutex m_mutex;
	public:
    file_appender();
		file_appender(const std::string& _filename);
    ~file_appender();
		void open(const std::string&);
		void log(std::shared_ptr<logger>,logger_level::level, logevent::_event)override;//记录日志
		void set_format(logger_format::_format)override;
	};


   //日志器,包含系统日志器和文件日志器 
	class logger: public std::enable_shared_from_this<logger> {
  friend class logger_manager;  
	protected:
		std::string m_name;//日志器名称
		logger_level::level m_level;
		logger_format::_format m_format;//输出格式
		std::list<logger_appender::_appender>m_appender;//输出区域
		std::mutex m_mutex;//设置互斥量
		class_memory_pool* m_memory_pool;
	protected:
		void set_format(logger_format::_format);
		void set_default_format();
		void set_level(logger_level::level _level) {
			m_mutex.lock();
			m_level = _level;
      m_mutex.unlock();
		}
	public:
		typedef std::shared_ptr<logger>_logger;
    logger();
		logger(const std::string& _name,logger_level::level _level,std::string _pattern="");
		virtual ~logger() {}
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
         system_logger(const std::string& _name,logger_level::level _level,std::string _pattern="");
         ~system_logger();
  };

   class file_logger:public logger{
      public:
          file_logger(const std::string& logger_name,logger_level::level _level,const std::string& file_name,std::string _pattern="");
          ~file_logger();
      private:
          int m_filenum;
   };



  //日志管理器，采用单例模式
	class logger_manager:public mode::nonauto_singleclass<logger_manager>{
	private:
		std::mutex m_mutex;
		std::map<std::string, logger::_logger>m_loggers;
		logger::_logger m_root_logger;//系统控制台日志
		int m_number;
	public:
		logger_manager();
		~logger_manager();
		void load_root_logger();
		void add_logger(logger::_logger);
		void delete_logger(const std::string&);
		void clean_logger();
		logger::_logger get_root_logger(){
			return m_root_logger;
		}
		logger::_logger get_logger(const std::string& name);
	};


#endif
