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
	printf("\rdecrypting: [");
	for(int i = 0; i < width; ++i){
		if(i < pos) printf("#");
		else printf(" ");
	}

	printf("] %d file(s)", cn);
	fflush(stdout);
}

void derive_key(const char *passwd, unsigned char *key){
	PKCS5_PBKDF2_HMAC(passwd, strlen(passwd), (unsigned char *)"saltsalt", 8, pbkdf2_iteration, EVP_sha256(), aes_key_size, key);
}

void decrypt(const char *path, const unsigned char *key){
	FILE *in = fopen(path, "rb");
	if(!in){
		fprintf(stderr, "cannot open: %s\n", path);
		return;
	}

	unsigned char salt[salt_size], iv[aes_iv_size];
	fread(salt, 1, salt_size, in);
	fread(iv, 1, aes_iv_size, in);
	EVP_CIPHER_CTX *c = EVP_CIPHER_CTX_new();
	EVP_DecryptInit_ex(c, EVP_aes_256_cbc(), NULL, key, iv);
	char last_path[1024];
	snprintf(last_path, sizeof(last_path), "%.*s", (int)(strlen(path) - 7), path);
	FILE *out = fopen(last_path, "wb");
	if(!out){
		fclose(in);
		EVP_CIPHER_CTX_free(c);
		fprintf(stderr, "cannot write: %s\n", last_path);
		return;
	}

	unsigned char inbuf[4096], outbuf[4096 + EVP_MAX_BLOCK_LENGTH];
	int bytes_read, out_ln;
	while((bytes_read = fread(inbuf, 1, sizeof(inbuf), in)) > 0){
		if(!EVP_DecryptUpdate(c, outbuf, &out_ln, inbuf, bytes_read)){
			fprintf(stderr, "decryption error in %s\n", path);
			fclose(in);
			fclose(out);
			remove(last_path);
			EVP_CIPHER_CTX_free(c);
			return;
		}

		fwrite(outbuf, 1, out_ln, out);
	}

	if(!EVP_DecryptFinal_ex(c, outbuf, &out_ln)){
		fprintf(stderr, "wrong key or corrupted file: %s\n", path);
		fclose(in);
		fclose(out);
		remove(last_path);
		EVP_CIPHER_CTX_free(c);
		return;
	}

	fwrite(outbuf, 1, out_ln, out);

	EVP_CIPHER_CTX_free(c);
	fclose(in);
	fclose(out);
	remove(path);
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
		if(!S_ISDIR(s.st_mode)){
			size_t len = strlen(path);
			if(len > strlen(file_suffix) && strcmp(path + len - strlen(file_suffix), file_suffix) == 0){
				decrypt(path, key);
			}
		}
	}

	closedir(dr);
}

void show_version(){
	printf("decryptor-%s\n", version);
}

void help(){
	printf("usage: decryptor [options]..\n");
	printf("options:\n");
	printf("  -f	decrypt a file\n");
	printf("  -p	decrypt all files in given path\n");
	printf("  -k	custom decryption passwd\n");
	printf("  -v	show version information\n");
	printf("  -h	display this\n");
}

int main(int argc, char *argv[]){
	char *target = NULL;
	char *passwd = last_passwd;
	char *a_file = NULL;
	int opt;
	while((opt = getopt(argc, argv, "p:f:k:vh")) != -1){
		switch(opt){
			case 'p': target = optarg; break;
			case 'f': a_file = optarg; break;
			case 'k': passwd = optarg; break;
			case 'v': show_version(); return 0;
			case 'h': help(); return 0;
			default: help(); return 1;
		}
	}

	if(!passwd){
		fprintf(stderr, "password required, use -k\n");
		return 1;
	}

	if(!target && !a_file){
		help();
		return 1;
	}

	unsigned char key[aes_key_size];
	derive_key(passwd, key);
	if(a_file){
		struct stat s;
		if(stat(a_file, &s) != 0 || !S_ISREG(s.st_mode)){
			fprintf(stderr, "'%s' is not a valid file\n", a_file);
			return 1;
		}

		printf("decrypting file: %s\n", a_file);
		decrypt(a_file, key);
		printf("\ndecrypted\n");
	} else if(target){
		struct stat s;
		if(stat(target, &s) != 0 || !S_ISDIR(s.st_mode)){
			fprintf(stderr, "'%s' is not a valid directory\n", target);
			return 1;
		}

		printf("decrypting directory: %s\n", target);
		p_directory(target, key);
		printf("\ndecryption completed\n");
	}

	return 0;
}
