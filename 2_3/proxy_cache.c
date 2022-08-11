#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdlib.h>

// libraries for getHomeDir function
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>

// libraries for making logfile
#include <time.h>
#include <dirent.h>

// wait
#include <sys/wait.h>

// server (socket, netinet..)
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define URLSIZE 1024
#define PORTNO 39999


char *sha1_hash(char *input_url, char *hashed_url){
	unsigned char hashed_160bits[20];
	char hashed_hex[41];
	int i;

	// SHA1(data you want to hashing, length of hashing data, destination);
	SHA1(input_url, strlen(input_url), hashed_160bits);
	
	// hashed_160bits -> hashed_hex transformation
	for(i = 0; i < sizeof(hashed_160bits); i++){
		sprintf(hashed_hex + i * 2, "%02x", hashed_160bits[i]);
	}
	
	// copy result to hashed_url
	strcpy(hashed_url, hashed_hex);

	return hashed_url;
}

char *getHomeDir(char *home){
	// get user information with using getpwuid
	struct passwd *usr_info = getpwuid(getuid());
	strcpy(home, usr_info->pw_dir);
	
	return home;
}

//////////////////////////////////////////////////////////////////////////
// handler								//
// =====================================================================//
// Input: void			                                        //
//        				                                //
//                            			                        //
// Output: static void 							//
// Output parameter Description : return nothing			//
//  									//
// Purpose : when this function called wait for child process's 	//
// 	     termination and receive termination status			//
//////////////////////////////////////////////////////////////////////////
static void handler(){
	pid_t pid;
	int status;
	while((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

int main(){	
	char cacheDirPath[20];
	int hit_num = 0, miss_num = 0;
	int subProcessNum = 0;
	int opt = 1;
	
	int i, j;
	
	/* variables for saving socket information */
	int socket_fd, client_fd;
	struct sockaddr_in server_addr, client_addr;
	
	int len, len_out;
	int state;
	/* variable for saving socket information */

	/* get home directory path */
	char *home = (char*)malloc(sizeof(char)*10);
	home = getHomeDir(home);
	
	/* cacheDirPath = /your home dir/cache */
	strcpy(cacheDirPath, home);
	strcat(cacheDirPath, "/cache");
	/* make cache directory */
	umask(000);
	mkdir(cacheDirPath, 0777);
	//////////////////////////
	
	if((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		printf("Server: Can't open the stream socket!\n");
		return 0;			
	}
	
	// use setsockopt to avoid bind TIME_WAIT error
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	// fill the server socket's information
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNO);
	// fill the server socket's information
	
	if(bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("Server: Can't bind the local address with stream socket!\n");
		close(socket_fd);
		return 0;
	}

	// wait until the requests are set.
	listen(socket_fd, 5);
	// enroll the signal handler.
	signal(SIGCHLD, (void *)handler);
	
	/* get the url from standard input */	
	while(1){
		struct in_addr inet_client_address;
		
		// initialize client_addr with zeros
		bzero((char*)&client_addr, sizeof(client_addr));
		
		len = sizeof(client_addr);
		// accepts client's connection request
		client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &len);
		
		// error handler.
		if(client_fd < 0){
			printf("Server: Accept failed!	%d\n", getpid());
			close(socket_fd);
			return 0;
		}
		
		inet_client_address.s_addr = client_addr.sin_addr.s_addr;
		// IPv4 based network address, port number
		printf("[%s : %d] client was connected.\n", inet_ntoa(inet_client_address), client_addr.sin_port);
		
		// make new process for handling new connection
		pid_t pid = fork();
		int status;
		
		// parent process: wait child process
		if(pid < 0){
			printf("fork error!\n");
		}
		// child process: proxy server start
		else if(pid == 0){
			char buf[URLSIZE];
			
			char response_header[URLSIZE];
			char response_message[URLSIZE];

			char tmpBuf[URLSIZE];
			char method[20];
			char input_domain[URLSIZE];
			char *input_url;
			char tmp_url[URLSIZE];	
			char *tok = NULL;
			
			read(client_fd, buf, URLSIZE);
			strcpy(tmpBuf, buf);
			
			puts("=========================================");
			printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			puts(buf);
			puts("=========================================");
			
			tok = strtok(tmpBuf, " ");
			strcpy(method, tok);
			
			// if method is "GET"
			if(strcmp(method, "GET") == 0){
				tok = strtok(NULL, " ");
				strcpy(input_domain, tok);
			}
			// write response header
			sprintf(response_header,
				"HTTP/1.0 200 OK\r\n"
				"Server:2022 simple web server\r\n"
				"Content-length:%lu\r\n"
				"Content-type:text/html\r\n\r\n", strlen(response_message));
			//input_url = strtok(input_domain, "/");
			for(i = 7; i <= strlen(input_domain); i++){
				tmp_url[i - 7] = input_domain[i];
			}tmp_url[i - 7] = '\0';			
			
			input_url = strtok(tmp_url, "/");

			printf("input url: %s\n", input_url);				

			/* get current time information with localtime systemcall */
			time_t now;
			time(&now);
			struct tm *ltp;
			ltp = localtime(&now);
		
			/* calculate hashed url with input url */
			char *hashed_url = (char*)malloc(sizeof(char) * 100);
			hashed_url = sha1_hash(input_url, hashed_url);
	
			//printf("%s\n", hashed_url);
			//////////// start of making directory name ///////////
			/* set directory path and name */
			char dirPath[30];
			strcpy(dirPath, cacheDirPath);
			strcat(dirPath, "/");
			char dirName[4];
			strncpy(dirName, hashed_url, 3);
			dirName[3] = '\0';
			strcat(dirPath, dirName);
			//printf("%s\n", dirPath);
			/////////// end of making directory name ///////////
		
			/* open cache directory for checking cached data */
			DIR *dp = opendir(cacheDirPath);
			struct dirent *pFile;
	
			if(dp == NULL){
				printf("%s\n", cacheDirPath);
				printf("Directory read error!!\n");
				break;
			}
			bool judge = false;
		
			/////////// start point of making file name ////////////
			char filePath[30];
			strcpy(filePath, dirPath);
			strcat(filePath, "/");

			char tmp[40];
			for(i = 3; i < strlen(hashed_url); i++){
				tmp[i-3] = hashed_url[i];
			}
			tmp[i-3] = '\0';
			
			strcat(filePath, tmp);		
			/////////// end point of making file name ////////////
			for(pFile = readdir(dp); pFile; pFile = readdir(dp)){
				// compare each directory name in cache directory
				if(strcmp(pFile->d_name, dirName) == 0)
					judge = true;
			}
			// hit
			if(judge){
				sprintf(response_message, "<h1>HIT</h1><br>%s", input_url);
				hit_num++;
			}
			// miss
			else{
				sprintf(response_message, "<h1>MISS</h1><br>%s", input_url);
				miss_num++;
			}

			/////////// start point of making file and directory ///////////
		
			//printf("%s\n", filePath);
			//printf("%s\n", dirPath);
			/* make new directory with hashed url */
			umask(000);
			mkdir(dirPath, 0777);
			/* make new file with hashed url */
			creat(filePath, 0644);
			/////////// end point of making file and directory /////////////
				
			/* free hashed url */
			free(hashed_url);

			////////////////// start of sending response message to client ////////////////
			write(client_fd, response_header, strlen(response_header));
			write(client_fd, response_message, strlen(response_message));
			////////////////// end of sending response msg to client ////////////////////

			printf("[%s : %d] client was disconnected.\n", inet_ntoa(inet_client_address), client_addr.sin_port);

			/* end of the child process */
			exit(0);			
		}
		/* close the client socket */
		close(client_fd);
		/* close the client socket */
		subProcessNum++;
	}
	/* close the socket */
	close(socket_fd);
	/* close the socket */
	return 0;
}
