#include <pthread.h>
#include <cstdio>
#include <unistd.h>

struct Monitor { 
    pthread_mutex_t mutex;
    pthread_cond_t cond; 

    Monitor(){
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&cond, nullptr);
    }

    ~Monitor(){
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex); 
    } 

    bool ready = false;   
    bool stopping = false;   

    Monitor(const Monitor& other) = delete;
    Monitor& operator=(const Monitor& other) = delete;
};


