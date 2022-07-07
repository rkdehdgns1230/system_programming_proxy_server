#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFSIZE 1024
#define PORTNO 40000

int main(){
	int socket_fd, len;
	struct sockaddr_in server_addr;
	
	char haddr[] = "127.0.0.1";
	char buf[BUFFSIZE];
	
	
	if((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		printf("can't create socket.\n");
		return -1;
	}
	
	bzero(buf, sizeof(buf));
	bzero((char*)&server_addr, sizeof(server_addr));

	// socket information
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(haddr);
	server_addr.sin_port = htons(PORTNO);
	
	// if connect fail!
	if(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		printf("can't connect.\n");
		return -1;
	}
	// file des, buf, nbytes
	write(STDOUT_FILENO, "> ", 2);
	// read from standard input and write it to the socket and standard output
	while((len = read(STDIN_FILENO, buf, sizeof(buf))) > 0){
		if(!strncmp(buf, "bye", 3)){
			break;
		}
		// file des, buf, nbytes
		if(write(socket_fd, buf, strlen(buf)) > 0){
			// send message to server.
			// file des, buf, nbytes
			write(STDOUT_FILENO, buf, len);
			bzero(buf, sizeof(buf));
		}
		
		// file des, buf, nbytes
		write(STDOUT_FILENO, "> ", 2);
	}
	// close socket with file descriptor.
	close(socket_fd);
	return 0;
}
