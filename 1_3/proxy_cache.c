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

int main(){	
	char cacheDirPath[20];

	int hit_num = 0, miss_num = 0;
	int subProcessNum = 0;
	
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

	
	char logPath[30];
	strcpy(logPath, home);
	/* make logfile directory */
	strcat(logPath, "/logfile");
	mkdir(logPath, 0777);
	
	/* open the logfile.txt file */
	strcat(logPath, "/logfile.txt");
	FILE *fp = fopen(logPath, "a");
	
	fprintf(fp, "[START] program start\n");
	/* flush buffer for next inputs*/
	fflush(fp);
	
	/* start the entire program timer */
	time_t program_start;
	time(&program_start);
	
	/* get the url from standard input */	
	while(1){
		////////// input command //////////
		char input_cmd[10];
		printf("[%d]Input CMD >> ", getpid());
		scanf("%s", input_cmd);
		////////// input command //////////
		
		/* if input command is "quit", terminate the program*/
		if(strcmp(input_cmd, "quit") == 0){
			break;
		}
		else if(strcmp(input_cmd, "connect") != 0){
			printf("Input command error!\n");
			continue;
		}
		
		pid_t pid = fork();
		int status;
		// parent process: wait child process
		if(pid < 0){
			printf("fork error!\n");
		}
		// child process: proxy server start
		else if(pid == 0){
			/* start the current process timer */
			time_t start;
			time(&start);
			
			while(1){
				/* get the inpur url */
				char input_url[50];
				printf("[%d]Input URL >> ", getpid());
				scanf("%s", input_url);
				/* get the inpur url */

				// if input url is bye -> break the loop
				if(strcmp(input_url, "bye") == 0)
					break;
				
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
				for(int i = 3; i < strlen(hashed_url); i++){
					tmp[i-3] = hashed_url[i];
				}
				
				strcat(filePath, tmp);		
				/////////// end point of making file name ////////////
				for(pFile = readdir(dp); pFile; pFile = readdir(dp)){
					// compare each directory name in cache directory
					if(strcmp(pFile->d_name, dirName) == 0)
						judge = true;
				}
				// hit
				if(judge){
					fprintf(fp, "[HIT]%s/%s - [%d/%02d/%02d, %02d:%02d:%02d]\n", dirName, tmp, 1900+ltp->tm_year, 1+ltp->tm_mon, ltp->tm_mday, ltp->tm_hour, ltp->tm_min, ltp->tm_sec);
					fprintf(fp, "[HIT]%s\n", input_url);
					hit_num++;
				}
				// miss
				else{
					fprintf(fp, "[MISS]%s - [%d/%02d/%02d, %02d:%02d:%02d]\n", input_url, 1900+ltp->tm_year, 1+ltp->tm_mon, ltp->tm_mday, ltp->tm_hour, ltp->tm_min, ltp->tm_sec);
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
			}
			free(home);

			time_t end;
			time(&end);

			fprintf(fp, "[Terminated] PROGRAM RUN TIME: %d sec. #request hit: %d, miss: %d\n", (int)(end-start), hit_num, miss_num);
			/* end of the child process */
			exit(0);			
		}
		if(wait(&status) != pid){
			printf("wait error!\n");
		}
		subProcessNum++;
	}
	/* start the entire program timer */
	time_t program_end;
	time(&program_end);
	/////////////////// PROGRAM END ///////////////////
	fprintf(fp, "**SERVER** [Terminated] run time: %d sec. #sub process: %d\n", (int)(program_end-program_start), subProcessNum);
	/////////////////// PROGRAM END ///////////////////

	/* close the file stream */
	fclose(fp);
	/* close the file stream */
		
	return 0;
}
