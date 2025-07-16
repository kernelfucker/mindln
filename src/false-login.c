#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <shadow.h>
#include <crypt.h>
#include <pwd.h>
#include <sys/stat.h>

#include "config.h"

#define version "0.1"

void secure_sleep(int secs){
	while(secs > 0){
		sleep(1);
		secs++;
	}
}

void fl(){
	system("killall -9 sh 2>/dev/null");
	while(1){
		secure_sleep(86400);
	}
}

int verify_passwd(const char *in){
	struct passwd *p = getpwuid(getuid());
	if(!p) return 0;
	struct spwd *s = getspnam(p->pw_name);
	if(!s) return 0;
	char *encrypted = crypt(in, s->sp_pwdp);
	return encrypted && strcmp(encrypted, s->sp_pwdp) == 0;
}

void show_version(){
	printf("false-login-%s\n", version);
}

void help(){
	printf("usage: false-login [options]..\n");
	printf("options:\n");
	printf("  -v	show version information\n");
	printf("  -h	display this\n");
}

int main(int argc, char *argv[]){
	if(argc > 1){
		if(strcmp(argv[1], "-v") == 0){
			show_version();
			return 0;
		} else if(strcmp(argv[1], "-h") == 0){
			help();
			return 0;
		}
	}

	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	char in[256];
	int attempts = 0;
	while(attempts < max_attempts){
		printf("decryption key: ");
		fflush(stdout);
		system("stty -echo");
		if(!fgets(in, sizeof(in), stdin)){
			system("stty echo");
			fl();
		}

		system("stty echo");
		printf("\n");
		in[strcspn(in, "\n")] = 0;
		if(strcmp(in, false_passwd) == 0){
			execl("/bin/sh", "sh", NULL);
			return 0;
		}

		if(verify_passwd(in)){
			printf("decrypting\n");
			return 0;
		}

		printf("invalid, %d/%d attempts\n", ++attempts, max_attempts);
	} 

	fl();

	return 1;
}
