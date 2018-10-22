#include<iostream>
#include<signal.h>
#include<unistd.h>
#include <sys/types.h>

void get_info(int qid, struct msgbuf *exitmsg, int size, long mtype);
