/*
block queue realized by rotated list
*/

#ifndef _BLOCK_QUEUE_H
#define _BLOCK_QUEUE_H
#include "../lock/lock.h"
#include <exception>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
template<typename T>
class block_queue{
public:
    block_queue(int max_size):max_size(max_size){
        if (max_size <= 0) throw std::exception();
        m_array = new T[max_size];
        if (!m_array) throw std::exception();
        m_front = -1;
        m_back = -1;
        m_size = 0;
    }
    ~block_queue(){
        delete [] m_array;
    }
    bool front(T& value){
        queue_mutex.get();
        if (m_size == 0){
            queue_mutex.release();
            return false;
        }
        value = m_array[m_front];
        queue_mutex.release();
        return true;
    }

    bool back(T& value){
        queue_mutex.get();
        if (m_size == 0){
            queue_mutex.release();
            return false;
        }
        value = m_array[m_back];
        return true;
    }

    bool push(T& value){
        queue_mutex.get();
        if (m_size >= max_size){
            queue_cond.broadcast();
            queue_mutex.release();
            return false;
        }
        m_back = (m_back+1) % max_size;
        m_array[m_back] = value;
        m_size++;
        queue_cond.broadcast();
        queue_mutex.release();
        return true;
    }

    bool pop(T& value){
        queue_mutex.get();
        while (m_size <= 0){
            if(!queue_cond.wait(queue_mutex.get_value())){
                queue_mutex.release();
                return false;
            }
        }
        value = m_array[m_front];
        m_front = (m_front + 1) % max_size;
        m_size--;
        queue_mutex.release();
        return true;
    }

    int size(){
        int tmp = 0;
        queue_mutex.get();
        tmp = m_size;
        queue_mutex.release();
        return tmp;
    }

    bool pop(T& value, int ms_timeout){
        struct timespec t= {0,0};
        struct timeval now = {0,0};
        gettimeofday(&now, NULL);
        queue_mutex.get();
        if (m_size < = 0){
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!queue_cond.time_wait(queue_mutex.get_value(),t)){
                queue_mutex.release();
                return false;
            }
        }

        if (m_size <= 0)
        {
            m_mutex.unlock();
            return false;
        }
        value = m_array[m_front];
        m_front = (m_front + 1) % max_size;
        m_size--;
        queue_mutex.release();

    }

    void clear(){
        queue_mutex.get();
        m_front = -1;
        m_back = -1;
        m_size = 0;
        queue_mutex.release();
    }

    bool full(){
        queue_mutex.get();
        if (m_size >= max_size) {
            queue_mutex.release();
            return false;
        }
        queue_mutex.release();
        return true;
    }

    bool empty(){
        queue_mutex.get();
        if (m_size > 0){
            queue_mutex.release();
            return false;
        }
        queue_mutex.release();
        return true;
    }

private:
    T*   m_array; //where store data
    int  m_front; //front ptr of the array
    int  m_back; // back  ptr of the array
    int  m_size; // the current of the array
    int  max_size; // max size of the array


    lock queue_mutex; // lock protect queue visited by thread
    cond queue_cond;  //cond corresponding with lock
};

#endif