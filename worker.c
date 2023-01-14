#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h> 
#include <semaphore.h>
#include <fcntl.h>      
#include <string.h>  
#include <ctype.h> 
#include <sys/types.h>
#include <sys/wait.h>

#define SEM_PRODUCER_FNAME "/sem_reader"
#define SEM_CONSUMER_FNAME "/sem_writer" 


struct myMemory {
    int askingLine;
    char gl[];     
};   

void worker (int X, int N) {  
    
    const char* name = "Shared Memory";
    int shm_fd;
    struct myMemory* shmptr;
    int running = 1; 
    int askforline, counter=0; 
 
    shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("Failed to open shared memory in child");
        exit(EXIT_FAILURE); 
    }
    
    sem_t *sem_reader = sem_open(SEM_PRODUCER_FNAME,0); 
    if (sem_reader == SEM_FAILED) {
        perror("sem open failed on producer2"); 
        exit(EXIT_FAILURE); 
    }
    

    sem_t *sem_writer = sem_open(SEM_CONSUMER_FNAME,0);
    if (sem_writer == SEM_FAILED) {
        perror("sem open failed on consumer2"); 
        exit(EXIT_FAILURE); 
    }
    

   shmptr = mmap(0, sizeof(struct myMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
     
   srand(time(0)); 

   clock_t start_t=0, end_t=0, total_t=0;

   for (int i = 0; i < N; i++) {
       sem_wait(sem_writer);
       askforline = (rand() % (X) + 1);
       printf("Child %d wants line %d\n", getpid(), askforline); 
       start_t = clock(); 
       shmptr->askingLine = askforline;
       sem_post(sem_reader);
       sleep(1); 
       printf("child %d got line %s\n", getpid(), shmptr->gl);
       end_t = clock(); 
       total_t += (double)(end_t - start_t) / CLOCKS_PER_SEC;
   }
   double avg = 0.0;
   avg = (total_t) / N;
   printf("Child %d average time taken for a transcation is %f\n", getpid(), avg);
   exit(0);


    
    sem_close(sem_reader);
    sem_close(sem_writer); 
     
    shm_unlink(name);


}