#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // STDOUT_FILENO

#define BUFFSIZE 1024
#define PORTNO 40000

int main(){
	struct sockaddr_in server_addr, client_addr;
	int socket_fd, client_fd;
	int len, len_out;
	
	char buf[BUFFSIZE];
	
	if((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		printf("Server: Can't open stream socket.");
		return 0;
	}
	// fill zeros in server_addr
	bzero((char *)&server_addr, sizeof(server_addr));
	
	// address system definition
	server_addr.sin_family = AF_INET;
	// long data to network byte order.
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// short data to network byte order.
	server_addr.sin_port = htons(PORTNO);
	
	if(bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("Server: Can't bind local address.\n");
		return 0;
	}
	// socket descriptor, length of the queue
	listen(socket_fd, 5);
	
	// repeat until the ctrl+C
	while(1){
		len = sizeof(client_addr);
		
		client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
		
		if(client_fd < 0){
			printf("Server: accept failed!!\n");
			return 0;
		}
		
		// connect message
		printf("[%d:%d] client was connected.\n", client_addr.sin_addr.s_addr, client_addr.sin_port);
		
		// repeat until the client send the requests.
		while((len_out = read(client_fd, buf, BUFFSIZE)) > 0){
			write(STDOUT_FILENO, "	- Messages : ", 15);
			write(STDOUT_FILENO, buf, len_out);
			write(client_fd, buf, len_out);
			write(STDOUT_FILENO, "\n", 1);
		}
		// disconnect message (client stop to send requests)
		printf("[%d:%d] client was disconnected.\n", client_addr.sin_addr.s_addr, client_addr.sin_port);
		// close the connection.
		close(client_fd);
	}
	// close the socket with socket descriptor.
	close(socket_fd);
	return 0;
}
