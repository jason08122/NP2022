#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <bits/stdc++.h>
#include "np_multi_proc.h"
#include <sys/shm.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
using namespace std;


int main ()
{
    char folder[] = "/user_pipe/1";
    char a[] = "hello world";
    char b[20];
    int wfd = open(folder, O_WRONLY);
    int rfd = open(folder, O_RDONLY);

    write(wfd,&a[0], sizeof(&a[0])+1);

    read(rfd,b,sizeof(b));

    printf("b is %s\n", b);

    return 0;
}
