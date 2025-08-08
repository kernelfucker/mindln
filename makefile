cc=clang
cflags=-Wall -Wextra
ldflags=-O2 -s -lcrypto -lssl
ldflags_fl=-O2 -s -lcrypt

all: encryptor decryptor false-login

encryptor: src/encryptor.c
	$(cc) $(cflags) $< -o $@ $(ldflags)
decryptor: src/decryptor.c
	$(cc) $(cflags) $< -o $@ $(ldflags)
false-login: src/false-login.c
	$(cc) $(cflags) $< -o $@ $(ldflags_fl)
