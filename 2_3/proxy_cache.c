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

// gethostbyname
#include <netdb.h>

#define URLSIZE 1024
#define BUFSIZE 40960
#define PORTNO 39999

FILE* fp; // logfile's file pointer
time_t server_start, server_end;
int subProcessNum = 0; // the number of sub processes
pid_t ppid; // parent process id

// get hashed url
char* sha1_hash(char* , char*);

// get the home directory name
char* getHomeDir(char*);

// signal handler
static void handler();
static void sigInt(int);
static void sigAlrm(int);

// change host name to dotted decimal
char* getIPAddr(char*);
//////////////////////////////////////////////////////////////////////////
// sha1_hash								//
// =====================================================================//
// Input: char* -> input_url                                            //
//        char* -> hashed_url		                                //
// Input parameter Description : get input url and hashed url		//
//                            			                        //
// Output: char* -> hashed_url						//
// Output parameter Description : return hashed url mapped by SHA1 func //
//  									//
// Purpose : mapping url to hash value			                //
//////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////
// getHomeDir								//
// =====================================================================//
// Input: char* -> home		                                        //
//        				                                //
// Input parameter Description : get the storage of path to the home dir//
//                            			                        //
// Output: char* -> home						//
// Output parameter Description : return home path of current user	//
//  									//
// Purpose : get the home dir's path with this function	                //
//////////////////////////////////////////////////////////////////////////
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

char* getIPAddr(char* url) {
	struct hostent* hent;
	char* haddr;
	int len = strlen(url);

	if ((hent = (struct hostent*)gethostbyname(url)) != NULL) {
		// 32 bits big-endian ip address -> dotted decimal
		haddr = inet_ntoa(*((struct in_addr*)hent->h_addr_list[0]));
	}
	else {
		printf("get IP address error!!\n");
	}
	return haddr;
}

//////////////////////////////////////////////////////////////////////////
// sigInt								//
// =====================================================================//
// Input: int signo		                                        //
//        				                                //
//                            			                        //
// Output: void 							//
// Output parameter Description : return nothing			//
//  									//
// Purpose : this function is SIGINT's signal handler function 		//
//	     executing when process create SIGINT signal		//
//////////////////////////////////////////////////////////////////////////
void sigInt(int signo) {
	// current pid == ppid then terminate current process and print termination message
	if (getpid() == ppid) {
		time(&server_end);
		fprintf(fp, "**SERVER** [Terminated] run time %d sec. #sub process: %d\n", (int)(server_end - server_start), subProcessNum);
		exit(0);
	}
	return;
}
//////////////////////////////////////////////////////////////////////////
// sigAlrm								//
// =====================================================================//
// Input: int signo		                                        //
//        				                                //
//                            			                        //
// Output: void 							//
// Output parameter Description : return nothing			//
//  									//
// Purpose : this function is SIGALRM's signal handler function 	//
//	     executing when process create SIGALRM signal		//
//////////////////////////////////////////////////////////////////////////
void sigAlrm(int signo) {
	// print alarm msg
	printf("-------------No Response -----------------\n");
	exit(0);
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

	ppid = getpid(); // saving current process id
	time(&server_start);

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

	/* make logfile directory and file */
	char logPath[60];
	strcpy(logPath, home);
	strcat(logPath, "/logfile");
	mkdir(logPath, 0777);

	strcat(logPath, "/logfile.txt");
	fp = fopen(logPath, "a");

	if((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		printf("Server: Can't open the stream socket!\n");
		return 0;			
	}
	/* make logfile directory and file */


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
	signal(SIGINT, (void*)sigInt);
	signal(SIGALRM, (void*)sigAlrm);
	// enroll the signal handler.
	
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
			char buf[BUFSIZE];
			char *IPAddr;
			char response_header[BUFSIZE];
			char response_message[BUFSIZE];

			char tmpBuf[BUFSIZE];
			char method[20];
			char input_domain[URLSIZE];
			char *input_url;
			char tmp_url[URLSIZE];	
			char *tok = NULL;
			// print address and port number of client
			printf("[%s:%d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			// read request message from the client
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
				char* tok2;

				tok = strtok(NULL, " ");
				strcpy(input_domain, tok);

				for (i = 7; i <= strlen(input_domain); i++) {
					tmp_url[i - 7] = input_domain[i];
				}tmp_url[i - 7] = '\0';

				input_url = strtok(tmp_url, "/");

				// get IP address(dotted decimal) with input url
				printf("* Hostname: %s\n", input_url);
				IPAddr = getIPAddr(input_url);
				printf("* IP address: %s\n", IPAddr);
				// get IP address with input url
			}				

			/* get current time information with localtime systemcall */
			time_t now;
			time(&now);
			struct tm *ltp;
			ltp = localtime(&now);
		
			/* calculate hashed url with input url */
			char *hashed_url = (char*)malloc(sizeof(char) * 100);
			hashed_url = sha1_hash(input_url, hashed_url);
	
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
					judge = true; // found the cache file
			}

			// hit
			if(judge){
				/*
				여기서 구현해야할 내용
				1. cache에 작성된 내용 web browser로 전송
				2. logfile에 hit message 작성
				*/
				// find and open the cache file.
				FILE* hitFile = fopen(filePath, "r");

				// write the whole cache file to the client(web browser)
				while (feof(hitFile) == 0) {
					char cacheBuf[BUFSIZE];
					// read the cache file and write it to the terminal and web browser.
					fgets(cacheBuf, sizeof(cacheBuf), hitFile);
					write(STDOUT_FILENO, cacheBuf, sizeof(cacheBuf));
					write(client_fd, cacheBuf, sizeof(cacheBuf));
				}
				
				fprintf(fp,
					"[HIT]%s-[%d/%02d/%02d, %02d:%02d:%02d]\n",
					input_url, 1900 + ltp->tm_year, ltp->tm_mon, ltp->tm_mday, ltp->tm_hour, ltp->tm_min, ltp->tm_sec
				);
				fprintf(fp,
					"[HIT]%s\n",
					filePath);
				// close the file stream
				fclose(hitFile);
				hit_num++;
			}
			// miss
			else{
				/*
				여기서 구현해야할 내용
				1. web server로 request 전송
				2. web server response를 이용해 cache file 생성
				3. response를 web browser로 전송
				4. log file에 miss message를 작성 ㅇ
				*/
				// write miss message to logfile
				fprintf(fp,
					"[MISS]%s-[%d/%02d/%02d, %02d:%02d:%02d]\n",
					input_url, 1900 + ltp->tm_year, ltp->tm_mon, ltp->tm_mday, ltp->tm_hour, ltp->tm_min, ltp->tm_sec
				);
				// start alarm for 20 seconds
				alarm(20);

				struct sockaddr_in web_server_addr;
				int client_socket_fd;

				// set umask 000
				umask(000);
				// make new cache directory
				mkdir(dirPath, 0777);
				// make cache file in cache directory
				int file_fd = creat(filePath, 0644);

				//////////////////// making new socket for connection to server. ////////////
				if ((client_socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
					printf("Error: Can't create socket.\n");
					return -1;
				}

				bzero((char*)&web_server_addr, sizeof(web_server_addr));

				web_server_addr.sin_family = AF_INET;
				web_server_addr.sin_addr.s_addr = inet_addr(IPAddr);
				web_server_addr.sin_port = htons(80); // HTML's well known port #(80)

				if (connect(client_socket_fd, (struct sockaddr*)&web_server_addr, sizeof(web_server_addr)) < 0) {
					printf("socket connection fail!\n");
					return 0;
				}
				//////////////////// making new socket for connection to server. ////////////


				/* start point of sending request message to web server */
				int read_len; // the length of response message
				// send request message to web server.
				if (write(client_socket_fd, buf, strlen(buf)) > 0) {
					// read response message and save it to 'response_message'.
					if ((read_len = read(client_socket_fd, response_message, sizeof(response_message))) > 0) {
						// write response message to cache file and web browser.
						write(client_fd, response_message, read_len);
						write(file_fd, response_message, read_len);
						bzero(response_message, sizeof(response_message));
					}
				}
				// set alarm with 0
				alarm(0);
				// close socket
				close(client_socket_fd);
				// count the number of misses
				miss_num++;
			}
			/* free hashed url */
			free(hashed_url);
			
			printf("[%s : %d] client was disconnected.\n", inet_ntoa(inet_client_address), client_addr.sin_port);

			/* end of the child process */
			exit(0);
		}
		/* close the client socket */
		close(client_fd);
		/* close the client socket */
		subProcessNum++;
		fprintf(fp, "sub process number count! %d\n", subProcessNum);
	}
	/* close the socket (connection for web browser) */
	close(socket_fd);
	/* close the socket (connection for web browser) */
	fclose(fp); // close the logfile file stream

	return 0;
}
