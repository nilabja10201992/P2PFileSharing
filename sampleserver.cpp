#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <pthread.h>
#include <bits/stdc++.h>
#include <iostream>
using namespace std;

#define PORT 22000 
#define BACKLOG 5
#define LENGTH 512

/*void error(const char *msg){
	perror(msg);
	exit(1);
}*/
//pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
void senddata(string fs_name,int nsockfd){
	//pthread_mutex_lock(&lock);
	char sdbuf[LENGTH];
	printf("[Server] Sending %s to the Client...\n", fs_name);
	FILE *fs = fopen(fs_name.c_str(), "r");
	if(fs == NULL){
	    fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name, errno);
		pthread_exit(NULL);
		exit(1);
	}

	bzero(sdbuf, LENGTH); 
	int fs_block_sz; 
	while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0){
		if(send(nsockfd, sdbuf, fs_block_sz, 0) < 0)
		{
		    fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
			pthread_exit(NULL);
		    exit(1);
		}
		bzero(sdbuf, LENGTH);
	}
	printf("Ok sent to client!\n");
	close(nsockfd);
	printf("[Server] Connection with Client closed. Server will wait now...\n");
	//pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}

void connection1(){
	int sockfd; 
	int nsockfd; 
	int num;
	int sin_size; 
	struct sockaddr_in addr_local; 
	struct sockaddr_in addr_remote, remote_addr; 
	char revbuf[LENGTH]; 

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
		pthread_exit(NULL);
		//exit(1);
	}
	else 
		printf("[Server] Obtaining socket descriptor successfully.\n");
	addr_local.sin_family = AF_INET; 
	addr_local.sin_port = htons(PORT); 
	addr_local.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(addr_local.sin_zero), 8); 

	/*if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
		fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
		pthread_exit(NULL);
		//exit(1);
	}
	else 
		printf("[Server] Binded tcp port %d in addr 127.0.0.1 sucessfully.\n",PORT);

	if(listen(sockfd,BACKLOG) == -1){
		fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
		pthread_exit(NULL);
		//exit(1);
	}*/
	if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
	{
		fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",errno);
		pthread_exit(NULL);
		//exit(1);
	}
	else
		printf ("[Server] Listening the port %d successfully.\n", PORT);

	int success = 0;
	std::vector<std::thread> threads;
	while(success == 0){
		sin_size = sizeof(struct sockaddr_in);
		if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, (socklen_t *)&sin_size)) == -1){
		    fprintf(stderr, "ERROR: Obtaining new Socket Despcritor. (errno = %d)\n", errno);
			pthread_exit(NULL);
			//exit(1);
		}
		else 
			printf("[Server] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));
		/*string sock = "";
		sock.append(inet_ntoa(addr_remote.sin_addr));
        sock.append(":");
		sock.append(to_string(ntohs(addr_remote.sin_port)));
		printf("%s",sock.c_str());*/
		string fs_name = "f5.png";
        threads.push_back(std::thread(senddata, fs_name,nsockfd));
		success = 0;
	}
	for (auto& th : threads) th.join();  
}

int servercon(){
	connection1();
	return 1;
}
int main(){
	servercon();
	return 0;
}