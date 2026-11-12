#include <pthread.h>
#include <cstdio>
#include <unistd.h>
#include <cstring>   
#include <cstdlib>

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

int main() {
    Monitor m;
    pthread_t tp, tc;

    int tcCreated = pthread_create(&tc, nullptr, consumer, &m);
    if (tcCreated != 0) {
        fprintf(stderr, "[consumer]: pthread_create failed: %s\n", strerror(tcCreated));
        return EXIT_FAILURE; 
    }

    int tpCreated = pthread_create(&tp, nullptr, producer, &m);
    if (tpCreated != 0) {
        fprintf(stderr, "[producer]: pthread_create failed: %s\n", strerror(tpCreated));
        pthread_mutex_lock(&m.mutex);
        m.stop = true;
        pthread_cond_broadcast(&m.cond);
        pthread_mutex_unlock(&m.mutex);
        pthread_join(tc, nullptr);   
        return EXIT_FAILURE;
    }

    bool failed = false;
    int tpJoined = pthread_join(tp, nullptr);
    if (tpJoined != 0) {
        fprintf(stderr, "[producer]: pthread_join failed: %s\n", strerror(tpJoined));
        failed = true;
    }

    int tcJoined = pthread_join(tc, nullptr);
    if (tcJoined != 0) {
        fprintf(stderr, "[consumer]: pthread_join failed: %s\n", strerror(tcJoined));
        failed = true;
    }

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}