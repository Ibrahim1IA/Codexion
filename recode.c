#include <pthread.h>
#include <stdio.h>
int station =  2000;
pthread_mutex_t  mutex;
void *print_name(void *)
{
    pthread_mutex_lock(&mutex);
    station -= 1000;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{

    pthread_t t1;
    pthread_t t2;
    pthread_mutex_init(&mutex, NULL);
    printf("station avant les thread: %d\n", station);
    pthread_create(&t1, NULL, print_name, NULL);
    pthread_create(&t2, NULL, print_name, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("station apres les thread: %d\n", station);

}
