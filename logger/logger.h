#ifndef _LOGGER_H
#define _LOGGER_H
#include "block_queue.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdarg.h>
class log{

public:
    static log* get_instance(){
        static log instance;
        return &instance;
    }
    static void* flush_log_thread(void* args){
        log::get_instance()->async_write_log();
    }
    bool init(const char*file_path,int max_log_lines,int close_log,int max_buf_size,int max_queue_size);
    void flush(void* args);
    void write_into_log(int level,const char* format, ...);
private:
    char                      dir_name[128]; //dir name
    char                      file_name[128]; //file name
    int                       max_log_lines; //max log lines can be used
    long long                 lines_count; //how many lines are used
    bool                      is_async; // is the log async or not
    block_queue<std::string>* m_log_queue; //string source queue
    char*                     m_log_buf; // log buf
    int                       max_buf_size; //max_buf_size
    std::FILE*                m_log_fp; //ptr of log file
    int                       m_log_close; //is log closed
    lock                      m_mutex; // mutex for async writing
    int                       m_today; //today
    log();
    virtual ~log();
    void* async_write_log(){
        std::string format;
        while(m_log_queue -> pop(format)){
            m_mutex.get();
            fputs(format.c_str(),m_log_fp);
            m_mutex.release();
        }
    }
};


#endif