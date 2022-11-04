#include "logger.h"
#include <cstring>
#include <pthread.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

log::log(){
    lines_count = 0;
    is_async = false;
}

log::~log(){
    if(!m_log_fp){
        fclose(m_log_fp);
    }
}

bool log::init(const char*file_path,int max_log_lines,int close_log,int max_buf_size,int max_queue_size){
    if (max_queue_size >= 1){
        //if block queue size if bigger/eq 1, then it's an async log
        is_async = true;
        m_log_queue = new block_queue<std::string>(max_queue_size);
        pthread_t tid;
        pthread_create(&tid,NULL,flush_log_thread,NULL);
    }

    max_log_lines = max_log_lines;
    m_log_close = close_log;
    max_buf_size = max_buf_size;
    m_log_buf = new char[max_buf_size];
    memset(m_log_buf,'\0',sizeof(char)*sizeof(m_log_buf));

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    const char* p = strrchr(file_path,'/');
    char log_full_name[256] = {'\0'};
    if (p == NULL){
        //wrong file_path,just name it the time
        //strncpy(file_name,sizeof(char)*strlen(file_path));
        snprintf(log_full_name,255,"%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_path);

    }
    else{
        //right file_path,name it plus time
        strncpy(file_name,p+1,sizeof(char)*strlen(p+1));
        strncpy(dir_name,file_path,sizeof(char)*(p-file_name+1));
        snprintf(log_full_name,255,"%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,file_name);        
    }

    m_log_fp = fopen(log_full_name,"a");
    if (m_log_fp == NULL){
        return false;
    }
    m_today = my_tm.tm_mday;
    return true;

}
void log::write_into_log(int level,const char* format, ...){
    //first we get the time here
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {'\0'};
    switch (level)
    {
    case 0:
        strcpy(s,"[debug:]");
        break;
    case 1:
        strcpy(s,"[info:]");
        break;
    case 2:
        strcpy(s,"[warning:]");
        break;
    case 3:
        strcpy(s,"[error:]");
        break;
    default:
        strcpy(s,"[info:]");
        break;
    }
    m_mutex.get();
    lines_count++;

    //if it passes a day or exceed the max_log_line, we switch the output stream to another file(redirect m_log_ptr)
    if(m_today != my_tm.tm_mday || lines_count % max_log_lines==0){
        char new_log[255] = {0};
        fflush(m_log_fp);
        fclose(m_log_fp);
        char tail[16] = {0};
        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
        if(m_today != my_tm.tm_mday){
            snprintf(new_log,255,"%s%s%s",dir_name,tail,file_name);
            m_today = my_tm.tm_mday;
            lines_count = 0;
        }
        else{
            snprintf(new_log,255,"%s%s%s.%lld",dir_name,tail,file_name,lines_count / max_log_lines);
        }
        m_log_fp = fopen(new_log,"a");
    }
    m_mutex.release();
    //we do not need any thing about mode
    va_list valst;
    va_start(valst, format);

    std::string log_str;
    m_mutex.get();

    //写入的具体时间内容格式
    int n = snprintf(m_log_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    
    int m = vsnprintf(m_log_buf + n, max_buf_size, format, valst);
    m_log_buf[m+n] = '\n';
    m_log_buf[m+n+1] = '\0';
    log_str = m_log_buf;
    m_mutex.release();

    if(is_async && !m_log_queue->full()){
        m_log_queue->push(log_str);
    }
    else{
        m_mutex.get();
        fputs(log_str.c_str(),m_log_fp);
        m_mutex.release();
    }
    va_end(valst);
}
void log::flush(void* args){
     //instantly write into the file 
    m_mutex.get();
    fflush(m_log_fp);
    m_mutex.release();
}