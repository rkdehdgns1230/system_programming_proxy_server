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

// use semaphore
#include <sys/sem.h>
#include <sys/ipc.h>

// use POSIX Thread
#include <pthread.h>

#define URLSIZE 1024
#define BUFSIZE 40960
#define PORTNO 39999

FILE* logfile_fp; // logfile's file pointer
time_t server_start, server_end;
int sub_process_num = 0; // the number of sub processes
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

void p(int);
void v(int);

//////////////////////////////////////////////////////////////////////////
// p operation															//
// =====================================================================//
// Input: int -> semid (semaphore id)	                                //
// Input parameter Description : get semaphore id						//
//                            			                        		//
// Output: nothing														//
// Output parameter Description : none									//
//  																	//
// Purpose : set semaphore to obtain a shared resources	                //
//////////////////////////////////////////////////////////////////////////
void p(int semid){
	// set semaphore operation information
	struct sembuf pbuf;
	pbuf.sem_num = 0; // Semaphore number
	pbuf.sem_op = -1; // Semaphore operation
	pbuf.sem_flg = SEM_UNDO;

	// semop: atomically perform series of semaphore(semid) operations set.
	if(semop(semid, &pbuf, 1) == -1){
		perror("p: semop failed");
		exit(1);
	}
}

//////////////////////////////////////////////////////////////////////////
// v operation															//
// =====================================================================//
// Input: int -> semid (semaphore id)	                                //
// Input parameter Description : get semaphore id						//
//                            			                        		//
// Output: nothing														//
// Output parameter Description : none									//
//  																	//
// Purpose : set semaphore to release a shared resources	        	//
//////////////////////////////////////////////////////////////////////////
void v(int semid){
	// set semaphore operation information
	struct sembuf pbuf;
	pbuf.sem_num = 0; // Semaphore number
	pbuf.sem_op = 1;  // Semaphore operation
	pbuf.sem_flg = SEM_UNDO;

	if(semop(semid, &pbuf, 1) == -1){
		perror("v: semop failed");
		exit(1);
	}
}


//////////////////////////////////////////////////////////////////////////
// sha1_hash															//
// =====================================================================//
// Input: char* -> input_url                                            //
//        char* -> hashed_url		                                	//
// Input parameter Description : get input url and hashed url			//
//                            			                        		//
// Output: char* -> hashed_url											//
// Output parameter Description : return hashed url mapped by SHA1 func //
//  																	//
// Purpose : mapping url to hash value			              			//
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
// getHomeDir															//
// =====================================================================//
// Input: char* -> home		                                      		//
//        				                                				//
// Input parameter Description : get the storage of path to the home dir//
//                            			                        		//
// Output: char* -> home												//
// Output parameter Description : return home path of current user		//
//  																	//
// Purpose : get the home dir's path with this function	                //
//////////////////////////////////////////////////////////////////////////
char *getHomeDir(char *home){
	// get user information with using getpwuid
	struct passwd *usr_info = getpwuid(getuid());
	strcpy(home, usr_info->pw_dir);
	
	return home;
}

//////////////////////////////////////////////////////////////////////////
// handler																//
// =====================================================================//
// Input: void			                                        		//
//        				                                				//
//                            			                        		//
// Output: static void 													//
// Output parameter Description : return nothing						//
//  																	//
// Purpose : when this function called wait for child process's 		//
// 	     termination and receive termination status						//
//////////////////////////////////////////////////////////////////////////
static void handler(){
	pid_t pid;
	int status;
	while((pid = waitpid(-1, &status, WNOHANG)) > 0);
}

//////////////////////////////////////////////////////////////////////////
// getIPAddr															//
// =====================================================================//
// Input: char * (domain name)                                     		//
//        				                                				//
//                            			                        		//
// Output: char * 														//
// Output parameter Description : return dotted decimal fo input address//
//  																	//
// Purpose : this function returns dotted decimal of input url string	//
//////////////////////////////////////////////////////////////////////////
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
// sigInt																//
// =====================================================================//
// Input: int signo		                                        		//
//        				                                				//
//                            			                        		//
// Output: void 														//
// Output parameter Description : return nothing						//
//  																	//
// Purpose : this function is SIGINT's signal handler function 			//
//	     executing when process create SIGINT signal					//
//////////////////////////////////////////////////////////////////////////
void sigInt(int signo) {
	// current pid == ppid then terminate current process and print termination message
	if (getpid() == ppid) {
		time(&server_end);
		fprintf(logfile_fp, "**SERVER** [Terminated] run time %d sec. #sub process: %d\n", (int)(server_end - server_start), sub_process_num);
		exit(0);
	}
	return;
}
//////////////////////////////////////////////////////////////////////////
// sigAlrm																//
// =====================================================================//
// Input: int signo		                                        		//
//        				                                				//
//                            			                        		//
// Output: void 														//
// Output parameter Description : return nothing						//
//  																	//
// Purpose : this function is SIGALRM's signal handler function 		//
//	     executing when process create SIGALRM signal					//
//////////////////////////////////////////////////////////////////////////
void sigAlrm(int signo) {
	// print alarm msg
	printf("-------------No Response -----------------\n");
	exit(0);
}

//////////////////////////////////////////////////////////////////////////
// makeNewCacheFilePath													//
// =====================================================================//
// Input: char* dir_path, char* hashed_url                         		//
//        				                                				//
//                            			                        		//
// Output: char* new_file_path											//
//  																	//
// Purpose : this function make new cache file path with directory path	//
// and hashed url														//
//////////////////////////////////////////////////////////////////////////
char* makeNewCacheFilePath(char* dir_path, char* hashed_url){
	char* new_file_path = malloc(sizeof(char) * 30);
	strcpy(new_file_path, dir_path);
	strcat(new_file_path, "/");

	char tmp[40];
	int hashed_url_length = strlen(hashed_url);
	for(int i = 3; i < hashed_url_length; i++){
		tmp[i-3] = hashed_url[i];
	}
	tmp[hashed_url_length - 3] = '\0';
	
	strcat(new_file_path, tmp);	

	return new_file_path;
}

void* printMissMessage(void* input_url){
	time_t now;
	time(&now);

	struct tm *ltp;
	ltp = localtime(&now);

	fprintf(logfile_fp,
	"[MISS]%s-[%d/%02d/%02d, %02d:%02d:%02d]\n",
	(char*)input_url, 1900 + ltp->tm_year, ltp->tm_mon + 1, ltp->tm_mday, ltp->tm_hour, ltp->tm_min, ltp->tm_sec
	);
	fflush(logfile_fp);
	return;
}

struct hit_arg {
	char* input_url;
	char* dir_name;
	char* new_file_name;
};

void* printHitMessage(void* arg){
	// type casting to struct hit_arg
	struct hit_arg* args = (struct hit_arg *)arg;

	// print hit messages to logfile
	fprintf(logfile_fp, "[HIT]%s/%s\n", args->dir_name, args->new_file_name);
	fprintf(logfile_fp, "[HIT]%s\n", args->input_url);
	fflush(logfile_fp);

	return;
}

int main(){	
	char cache_dir_path[60];
	int hit_num = 0, miss_num = 0;
	int sub_process_num = 0;
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
	
	/* cache_dir_path = /your home dir/cache */
	strcpy(cache_dir_path, home);
	strcat(cache_dir_path, "/cache");
	/* make cache directory */
	umask(000);
	mkdir(cache_dir_path, 0777);
	//////////////////////////

	/* make logfile directory and file */
	char log_file_path[60];
	strcpy(log_file_path, home);
	strcat(log_file_path, "/logfile");
	mkdir(log_file_path, 0777);

	strcat(log_file_path, "/logfile.txt");
	logfile_fp = fopen(log_file_path, "a");

	if((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		printf("Server: Can't open the stream socket!\n");
		return 0;
	}
	/* make logfile directory and file */


	// use setsockopt to avoid bind TIME_WAIT error
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	//////// fill the server socket's information ////////
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNO);
	//////// fill the server socket's information ////////
	
	//////// binding socket with port and ip address for server ////////
	if(bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		printf("Server: Can't bind the local address with stream socket!\n");
		close(socket_fd);
		return 0;
	}
	//////// binding socket with port and ip address for server ////////

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
		
		// create new process for handling new connection
		pid_t pid = fork();
		int status;
		
		// parent process: wait child process
		if(pid < 0){
			printf("fork error!\n");
		}
		// child process: send request to original server
		else if(pid == 0){
			char request_body_buf[BUFSIZE];
			char *IPAddr;
			// for response http message
			char response_header[BUFSIZE];
			char response_message[BUFSIZE];

			char tmp_buf[BUFSIZE];
			char method[20];
			char input_domain[URLSIZE];
			char *input_url;
			char tmp_url[URLSIZE];	
			char *tok = NULL;

			// print address and port number of client
			printf("[%s:%d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			// read request message from the client
			read(client_fd, request_body_buf, URLSIZE);
			strcpy(tmp_buf, request_body_buf);
			
			puts("=========================================");
			printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			puts(request_body_buf);
			puts("=========================================");
			
			tok = strtok(tmp_buf, " ");
			strcpy(method, tok);
			
			// if HTTP method is "GET"
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

			/* get current time information with localtime system call */
			time_t now;
			time(&now);
			struct tm *ltp;
			ltp = localtime(&now);

			/* calculate hashed url with input url */
			char *hashed_url = (char*)malloc(sizeof(char) * 100);
			// use sha1_hash function for hashing
			hashed_url = sha1_hash(input_url, hashed_url);
	
			//////////// start of making directory name ///////////
			char dir_path[60];

			strcpy(dir_path, cache_dir_path);
			strcat(dir_path, "/");

			///////// set cache directory name with hashed url /////////
			char dir_name[4];
			strncpy(dir_name, hashed_url, 3);
			dir_name[3] = '\0';
			///////// set cache directory name with hashed url /////////

			strcat(dir_path, dir_name);
			/////////// end of making directory name ///////////
		
			/* open cache directory for checking cached data */
			DIR *dp = opendir(cache_dir_path);
			struct dirent *pFile;

			/* failed to read cache directory */ 
			if(dp == NULL){
				printf("%s\n", cache_dir_path);
				printf("Directory read error!!\n");
				break;
			}

			/////////// start point of making new cache file name ////////////
			char new_file_path[90]; // failed to refactoring. I don't know what is the problem
			// occur malloc corruption, when using local function about this part
			strcpy(new_file_path, dir_path);
			strcat(new_file_path, "/");

			char new_file_name[40];
			for(i = 3; i < strlen(hashed_url); i++){
				new_file_name[i-3] = hashed_url[i];
			}
			new_file_name[i-3] = '\0';
			
			strcat(new_file_path, new_file_name);
			/////////// end point of making file name ////////////

			bool is_hit = false; // boolean type variable saving either cached or not

			for(pFile = readdir(dp); pFile; pFile = readdir(dp)){
				// compare each directory name in cache directory with newly created directory name
				if(strcmp(pFile->d_name, dir_name) == 0){
					// found the cache file => hit
					is_hit = true; 
				}
			}
			////////////////// setting semaphore /////////////
			// for semctl() function
			union semun{
				int val;
				struct semid_ds *buf;
				unsigned short int *array;
			} arg;

			int semkey = PORTNO;
			int semid;
			// make new semaphore and get semaphore id from semget function
			// semget(key_t key, int nsems(# of semaphore), int semflg)
			if((semid = semget((key_t)semkey, 1, IPC_CREAT|0666)) == -1){
				perror("semget failed!!");
				exit(1);
			}
			arg.val = 1;
			// semaphore initialization
			if((semctl(semid, 0, SETVAL, arg)) == -1){
				perror("semctl failed!!");
				exit(1);
			}
			////////////////// setting semaphore /////////////


			// hit
			if(is_hit){
				// find and open the cache file.
				FILE* cache_file = fopen(new_file_path, "r");

				// send the whole cache file to the client(web browser)
				while (feof(cache_file) == 0) {
					char cache_file_buf[BUFSIZE];
					// read the cache file and write it to the terminal and web browser.
					fgets(cache_file_buf, sizeof(cache_file_buf), cache_file);
					//write(STDOUT_FILENO, cache_file_buf, sizeof(cache_file_buf));
					write(client_fd, cache_file_buf, sizeof(cache_file_buf));
				}

				/////////// access to shared resource with semaphore ///////////
				printf("*PID# %u is waiting for the semaphore.\n", (unsigned int)getpid());
				p(semid);
				printf("*PID# %u is in the critical zone.\n", (unsigned int)getpid());
				
				////////// critical zone handling with thread //////////
				pthread_t tid; // thread id
				int err;
				void *thread_termination_code;
								
				struct hit_arg hit_args;
				hit_args.input_url = input_url;
				hit_args.dir_name = dir_name;
				hit_args.new_file_name = new_file_name;
				//pthread_create(thread id, thread attribute, particular function, function argument)
				err = pthread_create(&tid, NULL, printHitMessage, (void*)&hit_args);
				//error handling
				if(err != 0) {
					printf("pthread_create() error.\n");
					return 0;
				}
				printf("*PID# %u create the *TID# %u.\n", (unsigned int)getpid(), (unsigned int)tid);

				//wait until thread teminates
				pthread_join(tid, &thread_termination_code);
				printf("*TID# %u is exited.\n", (unsigned int)tid);

				////////// critical zone handling with thread //////////

				printf("*PID# %u exited the critical zone.\n", (unsigned int)getpid());
				v(semid);
				/////////// access to shared resource with semaphore ///////////

				fclose(cache_file);
				// count the hit num
				hit_num++;
			}
			// miss
			else{
				/////////// access to shared resource with semaphore ///////////
				printf("*PID# %u is wating for the semaphore.\n", (unsigned int)getpid());
				// wating until getting semaphore
				p(semid);
				printf("*PID# %u is in the critical zone.\n", (unsigned int)getpid());
				
				////////// critical zone handling with thread //////////
				pthread_t tid; // thread id
				int err;
				void *thread_termination_code;
				err = pthread_create(&tid, NULL, printMissMessage, (void*)input_url);
				//error handling
				if(err != 0) {
					printf("pthread_create() error.\n");
					return 0;
				}

				printf("*PID# %u create the *TID# %u.\n", (unsigned int)getpid(), (unsigned int)tid);

				//wait until thread teminates
				pthread_join(tid, &thread_termination_code);
				printf("*TID# %u is exited.\n", (unsigned int)tid);

				// release semaphore(shared resource)
				printf("*PID# %u exited the critical zone.\n", (unsigned int)getpid());
				v(semid);
				////////// critical zone handling with thread //////////

				/////////// access to shared resource with semaphore ///////////

				// start alarm for 30 seconds
				alarm(30);

				struct sockaddr_in web_server_addr;
				int client_socket_fd;

				// set umask 000
				umask(000);
				// make new cache directory
				mkdir(dir_path, 0777);
				// make cache file in cache directory
				int file_fd = creat(new_file_path, 0644);

				//////////////////// making new socket for connection to server. ////////////
				if ((client_socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
					printf("Error: Can't create socket.\n");
					return -1;
				}

				bzero((char*)&web_server_addr, sizeof(web_server_addr));

				web_server_addr.sin_family = AF_INET;
				web_server_addr.sin_addr.s_addr = inet_addr(IPAddr);
				web_server_addr.sin_port = htons(80); // HTML's well known port #(80)
				// connect to original server with socket
				if (connect(client_socket_fd, (struct sockaddr*)&web_server_addr, sizeof(web_server_addr)) < 0) {
					printf("socket connection fail!\n");
					return 0;
				}
				//////////////////// making new socket for connection to server. ////////////


				/* start point of sending request message to web server */
				int read_len; // the length of response message
				// send request message to web server.
				if (write(client_socket_fd, request_body_buf, strlen(request_body_buf)) > 0) {
					// read response message and save it to 'response_message'.
					if ((read_len = read(client_socket_fd, response_message, sizeof(response_message))) > 0) {
						// write response message to cache file and web browser(client).
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
			/* free memory allocation */
			//free(hashed_url);
			//free(new_file_path);
			/* free memory allocation */

			printf("[%s : %d] client was disconnected.\n", inet_ntoa(inet_client_address), client_addr.sin_port);

			/* end of the child process */
			exit(0);
		}
		/* close the client socket */
		close(client_fd);
		/* close the client socket */
		sub_process_num++;
	}
	/* close the socket (connection for web browser) */
	close(socket_fd);
	/* close the socket (connection for web browser) */

	// close the logfile file stream
	fclose(logfile_fp); 
	free(home);
	return 0;
}
