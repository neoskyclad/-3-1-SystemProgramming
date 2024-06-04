#include <stdio.h>
#include <math.h>
#include <pthread.h>

#define THREADS 4
#define N 3000

int primes[N];
int pflag[N];
int total = 0;

int cs_access = 0;	//Critical Section에 접근 중인 스레드 개수
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;     //condition value 생성 및 초기화
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	//mutex 생성 및 초기화

int is_prime(int v)
{
    int i;
    int bound = floor(sqrt ((double)v)) + 1;
    for (i = 2; i < bound; i++) {
        /* No need to check against known composites */ 
        if (!pflag[i])
            continue;
        if (v % i == 0) {
	    pthread_mutex_lock(&mutex);	//lock
	    while(cs_access > 0) {  //만약 Critical Section에 하나 이상의 스레드가 접근해있다면
		    pthread_cond_wait(&cond, &mutex);   //wait을 통해 스레드 대기
	    }
	    cs_access++;    //CS에 접근 중인 상태를 나타내는 변수
            pflag[v] = 0;
	    cs_access--;    //CS에 접근 해제
	    pthread_cond_signal(&cond); //signal을 통해 스레드 깨우기
	    pthread_mutex_unlock(&mutex);	//unlock
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
            pthread_mutex_lock(&mutex);  //mutex로 lock 수행
	    while(cs_access > 0) {
		    pthread_cond_wait(&cond, &mutex);
	    }
	    cs_access++;
            primes[total] = i;	//Critical Section
            total++;
	    cs_access--;
	    pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);  //mutex lock 해제
        }
    }
    return NULL;
}

int main(int argn, char **argv)
{
    int i;
    pthread_t tids[THREADS];
    int thread_ids[THREADS];
    
    for (i = 0; i < N; i++) {
        pflag[i] = 1; 
    }
    
    for (i = 0; i < THREADS; i++) {
        thread_ids[i] = i;	//thread_id 배열에 저장
        pthread_create(&tids[i], NULL, work, (void *)&thread_ids[i]);
    }

    for (i = 0; i < THREADS; i++) {
        pthread_join(tids[i], NULL);
    }
    
    printf("Number of prime numbers between 2 and %d: %d\n",
           N, total);
    for (i = 0; i < total; i++) {
        printf("%d\n", primes[i]);
    }

    pthread_cond_destroy(&cond);    //조건 변수 자원 반환
    pthread_mutex_destroy(&mutex);  //스레드 자원 반환

    return 0;
}

