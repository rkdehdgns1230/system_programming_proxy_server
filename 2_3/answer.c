//////////////////////////////////////////////////////////////////////////
// File Name    : Main.c                                                //
// Date         : 2021/05/15                                            //
// OS           : Ubuntu 16.04 LTS 64bits                               //
// Author       : Kang Dong Hun                                         //
// Student ID   : 2017202097                                            //
//----------------------------------------------------------------------//
// Title        : System Programming Assignment #2-2 (proxy server)     //
// Description  : make server process handling HTTP request from web	//
//		  browser and make response header and resonse message	//
//		  of HIT/MISS						//
//////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>

#define URLSIZE 40960
#define PORTNO 40000

int sub_pro_num = 0;
time_t server_start, server_end;
FILE *fp;	//logfile's file pointer
pid_t ppid;
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
char *sha1_hash(char* input_url, char *hashed_url){
	unsigned char hashed_160bits[20];
	char hashed_hex[41];
	int i;

	SHA1(input_url, strlen(input_url), hashed_160bits);
	//////////////////// transfer 160bit binary data to 40bit hexadecimal data /////////// 
	for(i = 0; i < sizeof(hashed_160bits); i++)
		sprintf(hashed_hex + i * 2, "%02x", hashed_160bits[i]);

	strcpy(hashed_url, hashed_hex);
	//////////////////// end of transformation ////////////////////////
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
//////////////////////////////////////////////////////////////////////////
// getIPAddr								//
// =====================================================================//
// Input: char *		                                        //
//        				                                //
//                            			                        //
// Output: char * 							//
// Output parameter Description : return dotted decimal fo input address//
//  									//
// Purpose : this function reuturns dotted decimal of input url string	//
//////////////////////////////////////////////////////////////////////////
char* getIPAddr(char* addr){
	struct hostent* hent;
	char* haddr;
	int len = strlen(addr);
	
	//domain name -> 32 bits ip address
	if((hent = (struct hostent*)gethostbyname(addr)) != NULL){
		//32 bits ip address -> dotted decimal
		haddr = inet_ntoa(*((struct in_addr*)hent->h_addr_list[0]));
	}
	else{
		printf("get IP addr error!!\n");
	}
	return haddr;
}
//////////////////////////////////////////////////////////////////////////
// sig_alrm								//
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
void sig_alrm(int signo){
	printf("-------------No Response -----------------\n");
	exit(0);
}
//////////////////////////////////////////////////////////////////////////
// sig_int								//
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
void sig_int(int signo){
	// current pid == ppid then terminate current process and print termination string
	if(getpid() == ppid){
		time(&server_end);
		fprintf(fp, "**SERVER** [Terminated] run time: %d sec. #sub process: %d\n", server_end-server_start, sub_pro_num);
		exit(0);
	}
	return;
}

//////////////////////////////////////////////////////////////////////////
// main									//
// =====================================================================//
// Input: void			                                        //
//        				                                //
//                            			                        //
// Output: int -> 0 							//
// Output parameter Description : end of program			//
//  									//
// Purpose : get url from standard input and make hash value from url	//
// 	     and make new cache file getting the name of hash.		//
//	     and make child process connected with web browsers		//
//////////////////////////////////////////////////////////////////////////
int main(){
	struct sockaddr_in server_addr, client_addr;
	int socket_fd, client_fd;
	int len, len_out;
	int state;

	char input_url[URLSIZE], input_command[100];
	char *home = (char*)malloc(sizeof(char)*50);
	home = getHomeDir(home);

	char cachePath[60];
	strcpy(cachePath, home);
	strcat(cachePath,"/cache");
	mkdir(cachePath, 0777);
	strcat(cachePath, "/");
	
	char logPath[60];
	strcpy(logPath, home);
	strcat(logPath, "/logfile");
	mkdir(logPath, 0777);
	strcat(logPath, "/logfile.txt");
	fp = fopen(logPath, "a");

	time(&server_start);

	pid_t pid;
	ppid = getpid();
	if((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		printf("Server : Can't open stream socket\n");
		return 0;
	}
	////////////////////// to avoid bind error //////////////////////////
	int opt = 1;	
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNO);
	
	if(bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		printf("Server : Can't bind local address\n");
		close(socket_fd);
		return 0;
	}
	
	listen(socket_fd, 5);
	/////////////////// install signal handlers ///////////////////////
	signal(SIGCHLD, (void *)handler);
	signal(SIGINT, (void *)sig_int);
	signal(SIGALRM, (void *)sig_alrm);
	/////////////////// install signal handlers ///////////////////////
	while(1){
		struct in_addr inet_client_address;
				
		bzero((char*)&client_addr, sizeof(client_addr));
		
		len = sizeof(client_addr);
		client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len);
		
		if(client_fd < 0){
			printf("Server : accept failed %d\n", getpid());
			close(socket_fd);
			return 0;
		}
		////////////////// start of making new sub server process /////////////////////
		
		pid = fork();
		////////////////// end of making new sub server process /////////////////////
		if(pid == -1){
			/////////////////// error handling /////////////////////
			close(client_fd);
			close(socket_fd);
			continue;
		}
		if(pid == 0){
			////////////////// start executing sub server process //////////////////				
			char content[URLSIZE];
			char *IPAddr;

			char tmp[URLSIZE] = {0, };
			char method[20] = {0, };
			
			char *tok = NULL;
			inet_client_address.s_addr = client_addr.sin_addr.s_addr;

			printf("[%s : %d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
	
			read(client_fd, content, URLSIZE);
			strcpy(tmp, content);
			
			puts("=========================================");
			printf("Request from [%s : %d]\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			puts(content);
			puts("=========================================");
			
			tok = strtok(tmp, " ");
			//////////////////// get host name //////////////////////
			strcpy(method, tok);
			//////////////////// repeat until get "HOST:" //////////////////
			
			if(strcmp(method, "GET") == 0){
				tok = strtok(NULL, " ");
				char buf[URLSIZE];
				strcpy(buf, tok);
				
				int i;
				char domain_name[URLSIZE];				
				char *tok2;			
				for(i = 7; i < strlen(buf); i++){
					domain_name[i - 7] = buf[i];
				}domain_name[i] = '\0';
				
				tok2 = strtok(domain_name, "/");
				strcpy(input_url, tok2);
				
				printf("* Hostname: %s\n", input_url);
				IPAddr = getIPAddr(input_url);
				printf("* IP adress : %s\n", IPAddr);
			}
						
			
			////////////////// Check the commands and get hash value //////////////
			char *hashed_url = (char*)malloc(sizeof(char) * 100);
			hashed_url  = sha1_hash(input_url, hashed_url);
			////////////////// End of getting hash value ///////////
			
			////////////////// get the path of new directory and make dir ///////////////
			char makeDir_path[80];
			strcpy(makeDir_path, cachePath);
			char dir_name[4];
			strncpy(dir_name, hashed_url, 3);
			dir_name[3] = '\0';
			strcat(makeDir_path, dir_name); 
			////////////////// get file name ///////////////////
			char buf[40]; int j;
			for(j = 3; j < strlen(hashed_url); j++){
				buf[j-3] = hashed_url[j];
			}buf[j-3] = '\0';
			////////////////// end of get file name /////////////////
			
			////////////////// check if hit or miss /////////////////
			DIR* dp = opendir(cachePath);
			struct dirent *cFile;
			bool judge = false;	
			
			char response_msg[URLSIZE];
			
			////////////////// get current time status ////////////////
			time_t now;
			time(&now);		
			struct tm *ltp;
			ltp = localtime(&now);
			////////////////// get current time status ////////////////
			
			////////////////// get the path of new file in new directory ///////////////
			char makeFile_path[90];
			strcpy(makeFile_path, makeDir_path);
			strcat(makeFile_path, "/");		
				
			strcat(makeFile_path, buf);
			////////////////// end of getting the path of new file in new directory /////
			for(cFile = readdir(dp); cFile; cFile = readdir(dp)){
				if(strcmp(cFile->d_name, dir_name) == 0) judge = true;
			}
			////////////////// check if hit or miss /////////////////
			if(judge == false){ // MISS
				fprintf(fp, "[MISS]%s-[%d/%02d/%02d, %02d:%02d:%02d]\n", input_url, 1900+ltp->tm_year, ltp->tm_mon, ltp->tm_mday, ltp->tm_hour,ltp->tm_min, ltp->tm_sec);
				//////////////////// alarm start //////////////////////////
				alarm(20);
				struct sockaddr_in web_server_addr;
				int client_socket_fd;
				
				umask(000);	// set umask 000	
				mkdir(makeDir_path, 0777);
				////////////////// end of making directory ///////////////////
				////////////////// create file and write reponse message to cache file ////////////
				int file_fd = creat(makeFile_path, 0644);

				//////////////////// start to making new socket (connect to web server) /////////////
				if((client_socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
					printf("can't create socket.\n");
					return -1;
				}
				bzero((char*)&web_server_addr, sizeof(web_server_addr));				
				web_server_addr.sin_family = AF_INET;
				web_server_addr.sin_addr.s_addr = inet_addr(IPAddr);//web server's ip address	
				web_server_addr.sin_port = htons(80);//HTML's well known port # (80)
			
				if(connect(client_socket_fd, (struct sockaddr*)&web_server_addr, sizeof(web_server_addr)) < 0){
					printf("can't connect.\n");
					return 0;
				}
				//////////////////// end of making new socket ///////////////////////
				//////////////////// start to send requst message to web server /////////////////
				
				int read_len;
				if(write(client_socket_fd, content, strlen(content)) > 0){
					if((read_len = read(client_socket_fd, response_msg, sizeof(response_msg))) > 0){
						//write(STDOUT_FILENO, response_msg, read_len);
						write(client_fd, response_msg, read_len);
						// write cache data to cache file
						write(file_fd, response_msg, strlen(response_msg));
						bzero(response_msg, sizeof(response_msg));
					}
				}
				//////////////////// initialize alarm with 0 ////////////////
				alarm(0);
				//////////////////// end to send requst message to web server /////////////////
				close(client_socket_fd);
				//////////////////// close the proxy server's socket //////////////////	
				
				////////////////// end of making new file in new directory //////////////////
				free(hashed_url);
				////////////////// initialize input_url with 0 ///////////////////
				bzero((char*)&input_url, sizeof(input_url));
			}
			else if(judge){ // HIT
				////////////////// find and open cache file /////////////////
				FILE *hit_file = fopen(makeFile_path, "r");
				////////////////// read until end of file ////////////////
				while(feof(hit_file) == 0){
					char hit_buf[URLSIZE];
					fgets(hit_buf, sizeof(hit_buf), hit_file);
					write(STDOUT_FILENO, hit_buf, strlen(hit_buf));
					write(client_fd, hit_buf, strlen(hit_buf));
				}
				////////////////// close cache file stream /////////////////
				fclose(hit_file);
			}
			printf("[%s : %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port);
			////////////////// end of child process ////////////////
			exit(0);
			////////////////// end of child process ////////////////
		}
		
		close(client_fd);
		sub_pro_num++;
	}
	close(socket_fd);
	fclose(fp);
	/////////////// end main process ///////////////
	return 0;
}

