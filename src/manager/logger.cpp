#include"logger.h"



	logevent::logevent(const std::string& _filename, uint32_t _linenuber,const std::string& _threadname,
		uint32_t _Threadid, uint32_t _coroutineid, uint32_t _elapse, uint64_t _time,
		logger_level::level _level,std::shared_ptr<logger>_logger): 
		m_file_name(""),m_line_number(_linenuber),m_thread_name(""),
		m_thread_id(0), m_coroutine_id(0),
		m_elapse_second(0), m_localtime(0),
	    m_level(logger_level::debug),m_logger(nullptr){
		m_file_name = _filename;
		m_thread_name = _threadname;
		m_thread_id = _Threadid;
		m_coroutine_id = _coroutineid;
		m_elapse_second = _elapse;
		m_localtime = _time;
		m_level = _level;
		m_logger = _logger;
		m_stream.str("");
	};

	void logevent::set_filename(const std::string& _filename) {
		m_file_name = _filename;
	}

	void logevent::set_threadname(const std::string& _threadname) {
		m_thread_name = _threadname;
	}

	void logevent::set_threadid(uint32_t _id) {
		m_thread_id = _id;
	}

	void logevent::set_coroutineid(uint32_t _id) {
		m_coroutine_id = _id;
	}

	void logevent::set_elapsesecond(uint32_t _second) {
		m_elapse_second = _second;
	}

	void logevent::set_time(uint64_t _time) {
		m_localtime = _time;
	}
 
	void logevent::log(){
		m_logger->log(m_level, shared_from_this());
	}

  void logevent_wrap::updata(){
     std::unique_lock<std::mutex> lock(m_mutex);
     if(!m_localtime)
       m_localtime=new Time();
     if(m_localtime->get_month()!=m_month){
       if(!m_recordings.empty())
         m_recordings.clear();
       m_month=m_localtime->get_month();  
     }
     if(m_localtime->get_monthday()!=m_day){
       if(!m_events.empty()){
        std::pair<int,std::list<logevent*> >newpair(m_listnum,m_events);
        m_recordings.insert(newpair); 
        m_events.clear();
       }
       m_day=m_localtime->get_monthday(); 
     }        
  }
     
  void logevent_wrap::requiry(std::string& info){

  }

std::string logger_level::tostring(logger_level::level _level) {
		switch (_level){
#define xx(name)\
         case  logger_level::name:\
           return #name;
			break;
			xx(debug);
			xx(warn);
			xx(inform);
			xx(crash);
			xx(error);
#undef  xx
		default:
			return (std::string)"unknow";
		}
	}


  logger::logger():
  m_name(""),m_level(logger_level::level::unknow),m_format(nullptr){
		m_memory_pool=new class_memory_pool();
	};

	logger::logger(const std::string& name,logger_level::level _level,std::string _pattern):
	    m_name(name),m_level(_level){
      set_format(_pattern);              
	}

	void logger::set_format(const std::string& _pattern) {
    if(_pattern.empty()){
       set_default_format();
       return;
    }       
		logger_format::_format new_format(m_memory_pool->new_class<logger_format>(_pattern));
		if (new_format == NULL) {
			std::cout << "create format failed" << std::endl;
			return;
		}
		set_format(new_format);
	}

	void logger::set_format(logger_format::_format _format) {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_format = _format;
	}
	//f m_file_name;
	//l m_line_number;
	//N m_thread_name;
	//t m_thread_id;
	//c m_coroutine_id;
	//s m_elapse_second;
	//C m_time;
	//T tab
	//e enter

//设置默认格式
	void logger::set_default_format() {
		std::string _pattern = "%T-%f-%l-%N-%t-%c-%s-%C-%e";
		set_format(_pattern);
	}
	//{"f", 0}, { "N",1 }, { "t",2 }, { "c",3 }, { "s",4 }, { "C",5 }, { "T",6 }, { "e",7 }, { "L",8 }, { "R",9 }, { "l",10 }, { "E",11 }


	void logger::log(logger_level::level _level, logevent::_event _event) {
		if (_level < m_level)
			return;
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_appender.empty())
			return;
		for (auto& i:m_appender) {
			i->log(shared_from_this(),_level, _event);
		}//所有日志输出器均要记录
	}

	void logger::add_appender(logger_appender::_appender _appender) {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_appender.push_back(_appender);
		_appender->set_format(m_format);
	}

	void logger::delete_appender(logger_appender::_appender _appender) {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_appender.empty())
			return;
		for (auto i = m_appender.begin(); i != m_appender.end();i++) {
			if (*i == _appender) {
				m_appender.erase(i);
				break;
			}
		}
	}

	void logger::clean_appender() {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_appender.empty())
			return;
			m_appender.clear();
	}
	
   system_logger::system_logger(const std::string&  _name,logger_level::level _level,std::string _pattern ){
        m_name=_name;
        set_format(_pattern);
        logger_appender::_appender newappender(new console_appender());
        add_appender(newappender);
        }
    
    system_logger::~system_logger(){
        clean_appender();
    }

        
    file_logger::file_logger(const std::string& logger_name,logger_level::level _level, const std::string& file_name,std::string _pattern):
    m_filenum(0){
        m_name=logger_name;
        set_format(_pattern);
        logger_appender::_appender newappender(new file_appender(file_name));
        add_appender(newappender);
        m_filenum++;
    }

    file_logger::~file_logger(){
          clean_appender();
          m_filenum=0;
    }



  file_appender::file_appender():m_filestream(nullptr){
    m_files=file::get_instance();
  }

	file_appender::file_appender(const std::string& _filename) {
		m_level = logger_level::debug;
		isempty_format = true;
    if(!m_files)
      m_files=file::get_instance();
    m_files->add_file(_filename);  
		open(_filename);
    m_filestream=m_files->get_stream(_filename);
	}

  file_appender::~file_appender(){
     m_format.reset();
     m_level=logger_level::level::unknow;
     if(m_filestream)
       m_filestream=nullptr;
     if(m_files)
        m_files=nullptr;  
  }

	void file_appender::open(const std::string& _filename) {
		std::unique_lock<std::mutex> lock(m_mutex);
		if(!m_files)
      m_files=file::get_instance();
    m_files->open_file(_filename);  
	}

	void file_appender::log(std::shared_ptr<logger>_logger, logger_level::level _level, logevent::_event _event) {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (_level < m_level)
			return;
		if (!m_format->log(*m_filestream, _logger, _level, _event))
			std::cout << "error,log failed" << std::endl;
	}

	void file_appender::set_format(logger_format::_format _format) {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (isempty_format) {
			m_format = _format;
			isempty_format = false;
		}
	}

	console_appender::console_appender(){
		m_level = logger_level::debug;
		isempty_format = true;
	}

  console_appender::~console_appender(){
     m_format.reset();
     m_level=logger_level::level::unknow;
  }

	void console_appender::log(std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event) {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (_level < m_level)
			return;
		if (!m_format->log(std::cout, _logger, _level, _event)) {
			std::cout << "error,log failed" << std::endl;
		}
	}

	void console_appender::set_format(logger_format::_format _format) {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (isempty_format)
			m_format = _format;
		isempty_format = false;
	}



	class filename_item :public logger_format::logger_item {
	public:
		filename_item(const std::string&_str) { m_title = _str; }
		~filename_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);
			out << _event->get_filename()<<"     ";
		}
	private:
		std::string m_title;
	};//打印文件路径

	class linenumber_item :public logger_format::logger_item {
	public:
		linenumber_item(const std::string&_str) { m_title = _str; }
		~linenumber_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger, logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);
			out << _event->get_linenumber()<<"     ";
		}
	private:
		std::string m_title;
	};//打印所处行号

	class threadname_item :public logger_format::logger_item {
	public:
		threadname_item(const std::string&_str) { m_title = _str; }
		~threadname_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);	
			out << _event->get_threadname()<<"     ";
		}
	private:
		std::string m_title;
	};//打印线程名称

	class threadid_item :public logger_format::logger_item {
	public:
		threadid_item(const std::string&_str) { m_title = _str; }
		~threadid_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);	
			out << _event->get_threadid()<<"     ";
		}
	private:
		std::string m_title;
	};//打印线程号

	class corrouteid_item :public logger_format::logger_item {
	public:
		corrouteid_item(const std::string&_str) { m_title = _str; }
		~corrouteid_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);	
			out << _event->get_coroutineid()<<"    ";
		}
	private:
		std::string m_title;
	};//打印协程号

	class elapse_item :public logger_format::logger_item {
	public:
		elapse_item(const std::string&_str) { m_title = _str; }
		~elapse_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger, logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);	
			out << _event->get_elapse_second()<<"     ";
		}
	private:
		std::string m_title;
	};//打印计算机时间戳
   
	class time_item :public logger_format::logger_item {
	public:
		time_item(const std::string&_str) { m_title = _str; }
		~time_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);	
			out << _event->get_localtime()<<"     ";
		}
	private:
		std::string m_title;
	};//打印本地时间

	class tab_item :public logger_format::logger_item {
	public:
		tab_item(const std::string&_str) { m_title = _str; }
		~tab_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);	
			out <<"\t";
		}
	private:
		std::string m_title;
	};//制表符

	class newline_item :public logger_format::logger_item {
	public:
		newline_item(const std::string&_str) { m_title = _str; }
		~newline_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);	
			out << "\n";
		}
	private:
		std::string m_title;
	};//换行符

	class level_item :public logger_format::logger_item {
	public:
		level_item(const std::string&_str) { m_title = _str; }
		~level_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);	
			out << _event->get_level();
		}
	private:
		std::string m_title;
	};//打印日志事件等级

	class Logger_item :public logger_format::logger_item {
	public:
		Logger_item(const std::string&_str) { m_title = _str; }
		~Logger_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger, logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);
			out << _logger->get_name();
		}
	private:
		std::string m_title;
	};//打印日志器名称

	class content_item :public logger_format::logger_item {
	public:
		content_item(const std::string&_str) { m_title = _str; }
		~content_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger, logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);
			out << _event->get_content()<<"     ";
		}
	private:
		std::string m_title;
	};//打印日志器记录的内容

	class error_item :public logger_format::logger_item {
	public:
		error_item(const std::string&_str) { m_error = _str; }
		~error_item() {};
		void log(std::ostream& out, std::shared_ptr<logger>_logger, logger_level::level _level, logevent::_event _event)override {
			std::unique_lock<std::mutex> lock(m_mutex);
			out <<m_error;
		}
	private:
		std::string m_error;
	};//打印错误信息


	logger_format::logger_format(const std::string& _pattern) {
		m_memory_pool=new class_memory_pool();
		m_pattern = _pattern;
		explain();
	}

	std::ostream& logger_format::log(std::ostream& out,std::shared_ptr<logger>_logger,logger_level::level _level, logevent::_event _event) {
		std::unique_lock<std::mutex> lock(m_mutex);
		for (auto& i : m_items) {
			i->log(out,_logger,_level, _event);
		}
		return out;
	}


	void logger_format::explain() {
		//#xx(xx) , #xx
		std::unique_lock<std::mutex> lock(m_mutex);
		std::vector<std::tuple<char, std::string, int> >vectors;//
		size_t i=0;
		std::string unlegal;//存储错误格式命令
		unlegal.clear();
		while(i<m_pattern.size()){
			while (m_pattern[i] != '%') {
				unlegal.push_back(m_pattern[i]);
				i++;				
				continue;
			}
			size_t j = i+1;
			std::string effect_str;//存储有效命令
			std::string unmatch_str;//存储无效命令
			std::string temp_str;//保存临时命令 
			bool match = true; //括号是否匹配的标志
			while(j  < m_pattern.size()) {
				size_t _begin;
				if (!(isalpha(m_pattern[j])) && (m_pattern[j]) != '(' && (m_pattern[j] != ')')) {
					temp_str.append(m_pattern.substr(i + 1, j - i - 1));
					j++;
					break;
				}//当出现连接字符立马跳出内循环,并且将前面的有效字符保留
				if (match) {
					if (m_pattern[j] == '(') {
						match = false;
						temp_str.append(m_pattern.substr(i + 1, j - i - 1));
						_begin = j + 1;
					}//语句中出现括号
					if (j == m_pattern.size()-1) {
						if (effect_str.empty())
							effect_str = m_pattern.substr(i + 1, m_pattern.size() - i - 1);
					}//语句中从未出现括号
				}
				else if(!match) {
					if (m_pattern[j] == ')') {
						temp_str.append(m_pattern.substr(_begin, j - _begin));
						match = true;
						j++;
						break;
					}
				}
				j++;
			}//跳出内部while循环(遇到右括号，连接符或者字符完全读取完，才退出循环)

      if(j!=m_pattern.size())
			  if(match)
				  effect_str.append(temp_str);
     
			if (!match) {
				unmatch_str.append(temp_str);
				if (!unlegal.empty()) {
					for(int i=0;i<unlegal.size();i++)
					  vectors.push_back(std::make_tuple(unlegal[i], "<<lacking of # header>>", -1));
					unlegal.clear();		
				}
				for(int i=0;i<unmatch_str.size();i++)
				  vectors.push_back(std::make_tuple(unmatch_str[i], "<<formation matched error>>", -1));
			  break; 
			}
			else if (match) {
				if (!unlegal.empty()){
					for(int i=0;i<unlegal.size();i++)
					  vectors.push_back(std::make_tuple(unlegal[i], "<<lacking of # header>>", -1));
					unlegal.clear();
					break;
				}
				for(int i=0;i<effect_str.size();i++)
				  vectors.push_back(std::make_tuple(effect_str[i], "<<formation is legal>>", 0));
				effect_str.clear();
				i = j;
			}
		}//while外部循环结束

		std::map<char, int>maps = {
			{'f',0},{'N',1},{'t',2 }, { 'c',3 }, { 's',4 },{'C',5},{'T',6},{'e',7},{'L',8},{'R',9} ,{'l',10},{'S',11}, {'E',12}

		};
		for (auto& i : vectors) {
			if (std::get<2>(i) == 0) {
				auto _value = maps.find(std::get<0>(i));
				if (_value == maps.end()) {
					m_items.push_back(logger_format::logger_item::_item(m_memory_pool->new_class<error_item>((const std::string)"cmd unlegal!")));
				}
				else {
					int _order = _value->second;
					switch (_order) {
					case 0: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<filename_item>((const std::string)"")));
						break;}
					case 1: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<threadname_item>((const std::string)"")));
						break;}
					case 2: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<threadid_item>((const std::string)"")));
						break;}
					case 3: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<corrouteid_item>((const std::string)"")));
						break;}
					case 4: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<elapse_item>((const std::string)"")));
						break;}
					case 5: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<time_item>((const std::string)"")));
						break;}
					case 6: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<tab_item>((const std::string)"")));
						break;}
					case 7: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<newline_item>((const std::string)"")));
						break;}
					case 8: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<level_item>((const std::string)"")));
						break; }
					case 9: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<Logger_item>((const std::string)"")));
						break; }
					case 10: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<linenumber_item>((const std::string)"")));
						break; }
					case 11: {
						m_items.push_back(logger_item::_item(m_memory_pool->new_class<content_item>((const std::string)"")));
						break; }
				    }
			    }
		    }
			else if (std::get<2>(i) == -1) {
				m_items.push_back(logger_item::_item(m_memory_pool->new_class<error_item>("unknow")));}
		}
}



	logger_manager::logger_manager():m_number(0){
		load_root_logger();
	}

	void logger_manager::add_logger(logger::_logger _logger) {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_loggers.insert(std::pair<std::string,logger::_logger>(_logger->get_name(),_logger));
	}
	
	void logger_manager::load_root_logger() {
	  std::unique_lock<std::mutex> lock(m_mutex);
		m_root_logger= logger::_logger(new system_logger("#system",logger_level::level::debug));
		m_loggers.insert(std::pair<std::string, logger::_logger>("#system",m_root_logger));
		m_number++;
	}


	void logger_manager::delete_logger(const std::string& name) {
		std::unique_lock<std::mutex> lock(m_mutex);
	    auto iter=m_loggers.find(name);
	    if (iter != m_loggers.end()) {
		    m_loggers.erase(iter);
		    m_number--;
	      }
	}
	
	void logger_manager::clean_logger() {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_loggers.clear();
		m_number = 0;
	}
	
	logger::_logger logger_manager::get_logger(const std::string& name) {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_number == 0)
			return nullptr;
		auto iter=m_loggers.find(name);
		if (iter != m_loggers.end())
			return iter->second;
		return nullptr;
	}
  
  logger_manager::~logger_manager(){
     m_loggers.clear();
     m_root_logger.reset();
     m_number=0;

  }  

