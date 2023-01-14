#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>     
#include <string.h> 
#include <stdbool.h> 
#include <sys/types.h>
#include <sys/wait.h>



#define SEM_PRODUCER_FNAME "/sem_reader"
#define SEM_CONSUMER_FNAME "/sem_writer" 



struct myMemory {
    int askingLine;  
    char gl[];  
};  
 
void worker(int K, int N); 

int main(int argc, char** argv) 
{   
    
    if (argc < 4) {
        perror("Not enough input arguments given.");
        exit(EXIT_FAILURE);
    }

    char *filename = argv[1];
    
    FILE* fp;
    fp = fopen(filename, "r");
    char line[100]; 
    int linecounter = 0; 
    while (1) {
        if (fgets(line,150, fp) == NULL) break;
            linecounter++;
    }
    printf("File contains %d lines.\n",linecounter);
    
    int K = atoi(argv[2]); 
    int N = atoi(argv[3]); 
    int X = linecounter; 
    const char* name = "Shared Memory";
    int shm_fd;
    struct myMemory* shmptr;
    pid_t pid; 

    sem_unlink(SEM_PRODUCER_FNAME);
    sem_unlink(SEM_CONSUMER_FNAME); 
    
 
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        perror("Failed to create shared memory.");
        exit(EXIT_FAILURE);
    }
    printf("Succesfully created memory.\n");
    
    if (ftruncate(shm_fd,sizeof(struct myMemory)) == -1) {
        perror("Failed to set size of shared memory.");
        exit(EXIT_FAILURE); 
    }    


    sem_t *sem_reader = sem_open(SEM_PRODUCER_FNAME, O_CREAT , 0660, 0); 
    if (sem_reader == SEM_FAILED) {
        perror("sem open failed on producer1"); 
        exit(EXIT_FAILURE); 
    }
    int value; 
    printf("Succesfully created sem reader on parent.\n"); 
    sem_getvalue(sem_reader,&value); 
    printf("sem reader has value %d\n", value); 

    sem_t *sem_writer = sem_open(SEM_CONSUMER_FNAME, O_CREAT, 0660, 1);
    if (sem_writer == SEM_FAILED) {
        perror("sem open failed on consumer1"); 
        exit(EXIT_FAILURE);  
    }
    printf("Succesfully created sem writer on parent.\n");  
    int value2; 
    sem_getvalue(sem_writer,&value2);
    printf("sem writer has values %d\n", value2); 


    shmptr = mmap(0,sizeof(struct myMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    int k; 
    for (int i = 0; i < K; i++) { 
        if ( fork() == 0) {
            printf("Created child %d\n", getpid()); 
            worker(X,N); 
        } else if (fork > 0) {
            for (int j = 0; j < N; j++) { 
                sem_wait(sem_reader); 
                k = 0;  
                char line[100]; 
                char* str; 
                bool flag; 
                flag = true;  
                fp = fopen(filename,"r"); 
                while (flag == true) {
                    if (fgets(line,150, fp) == NULL) {
                        break;
                } else {  
                    str = strdup(line); 
                    k++; 
                    if (k == shmptr->askingLine) {
                        strcpy(shmptr->gl, line);   
                        sem_post(sem_writer); 
                        flag = false; 
                    }
                    } 
                }
            } 
        }
        else {
            perror("Error in fork."); 
        }
    }
    
    for(int j = 0; j < K; j++) {
        wait(NULL);  
    }
    
    
    
    sem_close(sem_reader);
    sem_close(sem_writer); 
    sem_unlink(SEM_PRODUCER_FNAME);
    sem_unlink(SEM_CONSUMER_FNAME);  

    munmap(shmptr,sizeof(struct myMemory));  
    close(shm_fd);
    fclose(fp); 
   
   return 0; 
}
 