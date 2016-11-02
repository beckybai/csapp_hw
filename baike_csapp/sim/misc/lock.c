#include "lock.h"

int fd;

void init_lock() {
	fd = open(LOCK_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
}

int lock(mem_t m) {
	printf("--- try to lock %s\n", LOCK_FILE); 
//	while (flock(fd, LOCK_EX | LOCK_NB) != 0) {
//		response(m);
//		usleep(50);
//	}
	if(flock(fd,LOCK_EX | LOCK_NB)!=0)
		printf("--- lock succeed\n");
	return 1;
}

void unlock() {
	int res = (flock(fd, LOCK_UN) == 0);
	printf("--- unlock %s\n", res ? "succeed" : "failed");
}
