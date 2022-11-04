#include "thread_pool.h"
#include <stdio.h>
#include <string>
#include <unistd.h>
#include<list>
#include<iostream>
int main(){
    thread_pool<std::string> test_pool(20,20);
    std::string origin = "request_NO.";
    for (int i = 0 ;i < 20;++i){
        //std::string new_one = std::move(origin+(char)(49+i));
        std::string new_one = origin+(char)(49+i);
        test_pool.put(std::move(new_one));
    }
    sleep(1);
    std::list<std::string> debug = test_pool.debug_list();
    std::list<std::string>::iterator index = debug.begin();
    std::cout << "length:" << debug.size() << std::endl;
    while(index!=debug.end()){
        std::cout << *index << std::endl;
        index++;
    }
    // while(true){
    //     ;
    // }
    return 0;
}
