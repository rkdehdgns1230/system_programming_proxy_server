#include <stdio.h>
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
	char input_url[100];
	
	char cacheDirPath[20];
	
	/* get home directory path */
	char *home = (char*)malloc(sizeof(char)*10);
	home = getHomeDir(home);
	
	/* cacheDirPath = /your home dir/cache */
	strcpy(cacheDirPath, home);
	strcat(cacheDirPath, "/cache");
	/* make cache directory */
	umask(000);
	mkdir(cacheDirPath, 0777);
	
	strcat(cacheDirPath, "/");
	
	printf("Input URL >> ");
	/* get the url from standard input */	
	while(scanf("%s", input_url)){
		// if input url is bye -> break the loop
		if(strcmp(input_url, "bye") == 0)
			break;

		char *hashed_url = (char*)malloc(sizeof(char) * 100);
		hashed_url = sha1_hash(input_url, hashed_url);
		
		//printf("%s\n", hashed_url);

		//////////// start of making directory ///////////
		/* set directory path and name */
		char dirPath[30];
		strcpy(dirPath, cacheDirPath);

		char dirName[4];
		strncpy(dirName, hashed_url, 3);
		strcat(dirPath, dirName);
		/* set umask 000 */
		umask(000);
		/* make new directory with hashed url */
		mkdir(dirPath, 0777);
		//printf("%s\n", dirPath);
		/////////// end of making directory ///////////
		
		/////////// start of making file ///////////

		char filePath[30];
		strcpy(filePath, dirPath);
		strcat(filePath, "/");

		char tmp[40];
		for(int i = 3; i < strlen(hashed_url); i++){
			tmp[i-3] = hashed_url[i];
		}
		
		strcat(filePath, tmp);
		//printf("%s\n", filePath);
		creat(filePath, 0644);
		/////////// end of making file /////////////

		free(hashed_url);

		// print string for next url
		printf("Input URL >> ");
	}
	free(home);
	
	return 0;
}
