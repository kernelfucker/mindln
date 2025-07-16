cc=clang
cflags= -Wall -Wextra -O2 -s
ldflags=-lcrypto -lssl
ldflags_fl=-lcrypt

all: encryptor decryptor false-login

encryptor: src/encryptor.c
	$(cc) $(cflags) $< -o $@ $(ldflags)
decryptor: src/decryptor.c
	$(cc) $(cflags) $< -o $@ $(ldflags)
false-login: src/false-login.c
	$(cc) $(cflags) $< -o $@ $(ldflags_fl)
