#include <stdio.h>
#include <unistd.h>	
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#define LOCK_FILE "/tmp/y86-lock-file1.txt"

int main(){
	FILE *fd;
	int i;
	char ch ;
	int fd_lock = open(LOCK_FILE, O_RDWR | O_CREAT |O_TRUNC, S_IRUSR | S_IWUSR);
	if(fd_lock!=-1){
		printf("open success");
		fork();
		if(flock(fd_lock,LOCK_EX | LOCK_NB)!=0)
			
			printf("it isn't been locked");
		else 
			printf("it was locked\n");
//		int res = (flock(fd_lock,LOCK_UN)==0);
//			printf("--- unlock %s\n", res ? "succeed" : "failed");
	}
	return 0;
}
