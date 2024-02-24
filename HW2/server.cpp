#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstring>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "npshell.h"

int server (int port )
{
    printf("Start server1 at port:%d\n", port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) printf("create socket failed\n");

    struct sockaddr_in serverInfo, clientInfo;
	socklen_t addrlen = sizeof(clientInfo);
	bzero(&serverInfo, sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
	serverInfo.sin_addr.s_addr = INADDR_ANY;
	serverInfo.sin_port = htons(port);
	bind(sockfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
	listen(sockfd, 5);


int main()
{


    return 0;
}