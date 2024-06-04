#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>

#define THREADS 4
#define N 3000

int primes[N];
int pflag[N];
int total = 0;
sem_t sem;	//semaphore 선언

int is_prime(int v)
{
    int i;
    int bound = floor(sqrt ((double)v)) + 1;
    for (i = 2; i < bound; i++) {
        /* No need to check against known composites */ 
        if (!pflag[i])
            continue;
        if (v % i == 0) {
	    sem_wait(&sem); //P()로 wait
            pflag[v] = 0;
	    sem_post(&sem); //V()로 post
            return 0;
        }
    }
    return (v > 1); 
}

void *work(void *arg)
{
    int start;
    int end;
    int i;
    start = (N/THREADS) * (*(int *)arg);
    end = start + N/THREADS;
    for (i = start; i < end; i++) {
        if (is_prime(i)) {
	    sem_wait(&sem); //P()
            primes[total] = i;	//Critical Section
            total++;
	    sem_post(&sem); //V()
        }
    }
    return NULL;
}

int main(int argn, char **argv)
{
    int i;
    pthread_t tids[THREADS];
    int thread_ids[THREADS];

    sem_init(&sem, 0, 1);   //value를 1로 설정하여 Binary semaphore로 초기화
    
    for (i = 0; i < N; i++) {
        pflag[i] = 1; 
    }
    
    for (i = 0; i < THREADS; i++) {
        thread_ids[i] = i;	//thread_id 배열에 저장
        pthread_create(&tids[i], NULL, work, (void *)&thread_ids[i]);
    }

    for (i = 0; i < THREADS; i++) {
	pthread_join(tids[i], NULL);    //스레드 종료 기다림
    }
    
    printf("Number of prime numbers between 2 and %d: %d\n",
           N, total);
    for (i = 0; i < total; i++) {
        printf("%d\n", primes[i]);
    }

    sem_destroy(&sem);  //세마포어 자원 반환

    return 0;
}

