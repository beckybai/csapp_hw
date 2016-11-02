#ifndef LOCK_H
#define LOCK_H
#include <sys/mman.h>
#include <sys/file.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "isa.h"

#define LOCK_FILE "/tmp/y86-lock-file"

void init_lock();
int lock(mem_t m);
void unlock();

#endif

