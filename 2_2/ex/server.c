#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFSIZE 1024
#define PORTNO 39999

int main(){
	struct sockaddr_in server_addr, client_addr;
	int socket_fd, client_fd;
	int len, len_out;
	int opt = 1;
	
	if((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		printf("Server:Can't open the stream socket.\n");
		return 0;
	}
	// avoid TIME_WAIT
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNO);
	
	// sockaddr_in -> sockaddr transformation
	if(bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("Server: Can't bind local address.\n");
		return 0;
	}
	// socket descriptor, queue length
	listen(socket_fd, 5);
	
	while(1){
		struct in_addr inet_client_address;
		
		char buf[BUFFSIZE];

		char response_header[BUFFSIZE] = {0, };
		char response_message[BUFFSIZE] = {0, };
		
		
		char tmp[BUFFSIZE] = {0, };
		char method[20] = {0, };
		char url[BUFFSIZE] = {0, };
		
		char *tok = NULL;
		
		len = sizeof(client_addr);
		
		client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
		
		if(client_fd < 0){
			printf("Server: accept failed!!\n");
			return 0;
		}
		
		inet_client_address.s_addr = client_addr.sin_addr.s_addr;
		
		printf("[%s : %d] client was connected!\n", inet_ntoa(inet_client_address), client_addr.sin_port);
		read(client_fd, buf, BUFFSIZE);
		
		strcpy(tmp, buf);
		puts("==========================================");
		printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
		puts(buf);
		puts("==========================================\n");

		tok = strtok(tmp, " ");
		strcpy(method, tok);
		if(strcmp(method, "GET") == 0){
			tok = strtok(NULL, " ");
			strcpy(url, tok);
		}

		sprintf(response_message, 
			"<h1>RESPONSE</h1><br>"
			"Hello %s:%d<br>"
			"%s", inet_ntoa(inet_client_address), client_addr.sin_port, url);
		sprintf(response_header,
			"HTTP/1.0 200 OK\r\n"
			"Server:2022 simple web server\r\n"
			"Content-length:%lu\r\n"
			"Content-type:text/html\r\n\r\n", strlen(response_message));
		
		write(client_fd, response_header, strlen(response_header));
		write(client_fd, response_message, strlen(response_message));
		
		
		printf("[%s : %d] client was disconnected!\n", inet_ntoa(inet_client_address), client_addr.sin_port);
		// close the connection
		close(client_fd);
	}
	// close the socket
	close(socket_fd);
	return 0;
}
