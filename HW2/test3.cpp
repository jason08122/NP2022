#include <sys/shm.h>
#include <bits/stdc++.h>

int* shm_num;

int main()
{
    key_t key = ftok("/myshm/num", 0);
    int tmp_1 = shmget(key, sizeof(int), 0666);
    int* p = (int*) shmat(tmp_1, NULL, 0);

    int n = *p;

    printf("%d\n", n);

    return 0;
}