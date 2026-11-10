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
    bool stop = false;   

    const void* data = nullptr;

    Monitor(const Monitor& other) = delete;
    Monitor& operator=(const Monitor& other) = delete;
};


void* producer(void* arg){
    Monitor* m = static_cast<Monitor*>(arg);
    static const char event[] = "event";

    for (int i = 1; i <= 5; i++){
        sleep(1);
        pthread_mutex_lock(&m->mutex);
        m->data = event;
        m->ready = true;
        printf("[producer] событие %d отправлено\n", i);
        fflush(stdout);
        pthread_cond_signal(&m->cond);
        pthread_mutex_unlock(&m->mutex);
    }

    pthread_mutex_lock(&m->mutex);
    m->stop = true;
    pthread_cond_broadcast(&m->cond);
    pthread_mutex_unlock(&m->mutex);
    printf("[producer] завершён\n");

    return nullptr;
}

void* consumer(void* arg){ 
    Monitor* m = static_cast<Monitor*>(arg);
    pthread_mutex_lock(&m->mutex);
    int count = 0;
    while(true){
        while(!m->ready && !m->stop){
            pthread_cond_wait(&m->cond, &m->mutex);
        }
        
        if (m->stop && !m->ready){
            break;
        }

        const void* payload = m->data;
        m->ready = false;
        m->data = nullptr;
        ++count;
        printf("[consumer] событие %d обработано, данные[%p]\n", count, payload);
        fflush(stdout);
    }
    pthread_mutex_unlock(&m->mutex);
    printf("[consumer] завершён, всего обработано: %d\n", count);
    return nullptr;
}

int main(){
    Monitor m;
    pthread_t tp, tc;

    pthread_create(&tc, nullptr, consumer, &m);
    pthread_create(&tp, nullptr, producer, &m);

    pthread_join(tp, nullptr);
    pthread_join(tc, nullptr);

    return 0;
}