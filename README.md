# mindln
x86_64 linux ransomware utility with false-login

# make
$ make

# usage
\# ./mindln-encryptor.sh

\# ./mindln-decryptor.sh

\# ./encryptor -p /etc/ -k mindln

\# ./decryptor -p /etc/ -k mindln

# encryptor options
```
-f    encrypt a file
-p    encrypt all files in given path
-k    custom encryption passwd
-u    encrypt user home directory
```

# decryptor options
```
-f    decrypt a file
-p    decrypt all files in given path
-k    custom decryption passwd
```

# features
aes-256 encryption in cbc mode

passwd-based key derivation using pbkdf2-hmac-sha256

recursive directory encryption

random iv and salt generation per file

# notes
make your own configuration in src/config.h

configure your own configuration on mindln-encryptor.sh and mindln-decryptor.sh

default suffix is .mindln

default passwd for false-login is mindln

default passwd is mindln, this is a key

# what was it tested on
arch linux 2025-07-01, x86_64

gentoo linux, x86_64

# examples
<img width="576" height="426" alt="image" src="https://github.com/user-attachments/assets/42ea47c8-f11d-4403-9a22-1494760e1447" />

<img width="522" height="312" alt="image" src="https://github.com/user-attachments/assets/f88347be-577c-4bbf-b189-eb147b1fac7b" />
