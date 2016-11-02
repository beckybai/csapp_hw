#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h>



#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "isa.h"

int main()
{
    int fd = open(SHARE_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    assert(fd != -1);
	// initialize shared memory
    ftruncate(fd, SHARE_SIZE);
    byte_t zeros[SHARE_SIZE] = {0};
    write(fd, zeros, sizeof(zeros));
	
/*    
    byte_t *shared = mmap(NULL, SHARE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 0);
    assert(shared != MAP_FAILED);
    close(fd);
*/
	byte_t *shared;
        if (( shared = mmap(NULL, SHARE_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED| MAP_FILE, fd, 0)) == MAP_FAILED) 
	{
        printf("mmap error: %d\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    close(fd);
    int i;
    for (i = 0; i < SHARE_SIZE; i=i+4)
        shared[i] = 0;
    printf("Memory size: %.8x\n", SHARE_SIZE);
    printf("SWAP SIZE : %.8x\n",SWAP_SIZE);
	int *bus = (int *) (byte_t *) (shared + (SWAP_SIZE));
	    int *share = (int *)shared;

	for(i=0;i<SHARE_SIZE/4; i++){
        	if(share[i/4]!=0)
            		printf("0x%.4x: 0x%.8x\n",i,share[i]);
    	}

    int processnum = 2;
    while(bus[0] != processnum){
        if(bus[0]==1){
            printf("one process enter in!\n");
        }
       usleep(50);
    }

    printf("Clients ready...\n");

    while( bus[0] != 0) {
        int message = bus[1];
        if((bus[0])>processnum){
            printf("the clients number is too large! The position %x is visitd\n", SWAP_SIZE);
            exit(EXIT_FAILURE);
        }
        if (GETTYPE(message) != BUBBLE) {
    		if (bus[0] != 2)
    			bus[2] = 1;
                	while(bus[2] != 1)
                    	usleep(100);
                	bus[1] = BUBBLE;
        }
	usleep(100);
    }
    printf("Clients have already exited...\n");
    show_share_memory();
    munmap(shared, SHARE_SIZE);
    return 0;
}

void show_share_memory(){
    byte_t * newshare;
   
    int fd2 = open(SHARE_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (( newshare = mmap(NULL, SHARE_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED| MAP_FILE, fd2, 0)) == MAP_FAILED) 
    {
        printf("mmap error: %d\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int i;
    for(i=0;i<SHARE_SIZE;i++){
        if(newshare[i]!=0)
//            fprintf(pFile,"0x%.4x: 0x%.8x\n",i,share[i]);
              printf("0x%.4x: 0x%.8x\n",i,newshare[i]);
    }
    if (close(fd2) ==-1) {
            printf("close error: %d\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

}
