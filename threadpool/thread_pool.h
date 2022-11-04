#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H
#include "../lock/lock.h"
#include<exception>
#include<list>
#include<cstdio>
#include<iostream>
template<typename T>
class thread_pool{
private:
    int           max_thread_num; //max number can be used
    int           max_request_num; //max request can be processed
    int           used_volumn;    // in use 
    int           free_volumn;    // free
    sem           query_string_stat; // query queue
    lock          query_lock;      // lock control for query queue visit
    std::list<T> query_string_queue; // list store the query string
    std::list<T> query_string_debug;
    pthread_t*    pool;             //pointer to pool

public:
    thread_pool(int max_thread_num,int max_request_num);
    ~thread_pool();
    bool put(T query_string);
    std::list<T> & debug_list();

private:
    static void * work(void* args);
    void * run();

};
template<typename T>
std::list<T>& thread_pool<T>::debug_list(){
    return query_string_debug;
}
template<typename T>
thread_pool<T>::thread_pool(int max_thread_num,int max_request_num):query_string_stat(0),free_volumn(max_thread_num),used_volumn(0),max_request_num(max_request_num){
        if (max_thread_num <= 0 || max_request_num <= 0 ){
            throw std::exception();
        }
        pool = new pthread_t[max_thread_num];
        if (!pool) throw std::exception();
        for (int i = 0; i < max_thread_num;++i){
            if (pthread_create(pool+i,NULL,work,this) != 0){
                delete [] pool;
                throw std::exception();
            }
            if (pthread_detach(pool[i]) != 0){
                delete[] pool;
                throw std::exception();
            }
        }
}

template<typename T>
thread_pool<T>::~thread_pool(){
    delete [] pool;
}
template<typename T>
void* thread_pool<T>::work(void* args){
    thread_pool* pool = (thread_pool*) args;
    pool->run();
    return pool;
}

template<typename T>
void* thread_pool<T>::run(){
    // get one query from list
    while(true){
        query_string_stat.wait();
        query_lock.get();
        if (query_string_queue.empty()){
            query_lock.release();
            continue;
        }
        T request = query_string_queue.front();
        //if (request == NULL) continue; this line of code is not necessary to use now
        query_string_queue.pop_front();
        query_lock.release();
        //debug
        //std::cout << request << std::endl;
        query_string_debug.push_back(std::move(request));
        //process


    }
}

template<typename T>
bool thread_pool<T>::put(T query_string){
    query_lock.get();
    if (query_string_queue.size() >= max_request_num){
        query_lock.release();
        return false;
    }
    query_string_queue.push_back(std::move(query_string));
    query_lock.release();
    query_string_stat.post();
    return true;
}
#endif 