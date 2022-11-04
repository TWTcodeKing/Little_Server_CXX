#ifndef LOCK_H
#define LOCK_H
#include <exception>
#include <pthread.h>
#include <semaphore.h>

class sem{
private:

public:
    sem(){
        if (sem_init(&num,0,0) != 0){
            throw std::exception();
        }
    }   

    sem(int sem_num){
        if (sem_init(&num,0,sem_num) != 0){
            throw std::exception();
        }
    } 

    bool wait(){
        return sem_wait(&num) == 0;

    }

    bool post(){
        return sem_post(&num) == 0;
    }
    ~sem(){
        sem_destroy(&num);
    }

private:
    sem_t num;
};


class lock{

public:
    lock(){
        if (pthread_mutex_init(&mutex_lock,NULL) != 0){
            throw std::exception();
        }
    }

    ~lock(){
        pthread_mutex_destroy(&mutex_lock);
    }

    bool get(){
        return pthread_mutex_lock(&mutex_lock) == 0;
    }

    bool release(){
        return pthread_mutex_unlock(&mutex_lock) == 0;
    }

    pthread_mutex_t * get_value(){
        return &mutex_lock;
    }

private:
    pthread_mutex_t mutex_lock;
};


class cond{
public:
    cond(){
        if (pthread_cond_init(&m_cond,NULL) != 0){
            throw std::exception();
        }
    }

    ~cond(){
        pthread_cond_destroy(&m_cond);
    }

    bool wait(pthread_mutex_t * mutex){
        int ret;
        ret = pthread_cond_wait(&m_cond,mutex);
        return ret == 0;
    }

    bool time_wait(pthread_mutex_t* mutex,struct timespec *t){
        int ret;
        ret = pthread_cond_timedwait(&m_cond,mutex,t);
        return ret == 0;
    }

    bool signal(){
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool broadcast(){
        return pthread_cond_broadcast(&m_cond) == 0;
    }
private:
    pthread_cond_t m_cond;
};
#endif