#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Buffer.h"

using namespace std;
using namespace Forest;

#define MAXLINE 80
#define SERV_PORT 6666
int main()
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    int i, n;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	listen(listenfd, 20);

	printf("Accepting connections ...\n");
	Forest::Buffer myBuffer;
    while (1) {
		cliaddr_len = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
		//n = read(connfd, buf, MAXLINE);
		n = myBuffer.readDataFromFd(connfd);
		printf("received from %s at PORT %d\n",
		inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
		ntohs(cliaddr.sin_port));
		//write(connfd, buf, n);
		myBuffer.writeDataToFd(connfd);
		close(connfd);
	}

    return 0;
}