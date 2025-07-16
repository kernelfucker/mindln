#!/bin/sh

current="$(dirname "$(realpath "$0")")"
decryptor="$current/decryptor"

custom_key="mindln"
suffix=".mindln"

targets=(
        "/root/"
        "/etc/"
        "/mnt/"
        "/opt/"
        "/srv/"
        "/tmp/"
        "/var/"
        "/media/"
        "/usr/share/"
        "/home/"
)

echo "starting decryption"

for directory in "${targets[@]}"; do
	if [ -d "$directory" ]; then
		echo "scanning: $directory"
		mapfile -t files < <(find "$directory" -type f -name "*$suffix" ! -path "$current/*" 2>/dev/null)
		for file in "${files[@]}"; do
			if [ -f "$file" ]; then
				if [ "$file" = "$current/login.orig" ]; then
					continue
				fi

				echo "decrypting: $file"
				"$decryptor" -k "$custom_key" -f "$file" >/dev/null 2>&1
			fi
		done
	fi
done

echo "decryption completed"

while true; do
	echo -n "do you want to restore the old /bin/login file? [y/n] "
	read -r yn
	case "$yn" in
		[Yy]* )
			if [ -f "$current/login.orig" ]; then
				echo "login.orig is being restored"
				cp -f "$current/login.orig" /bin/login
				if [ $? -eq 0 ]; then
					chmod +x /bin/login
					echo "/bin/login successfully restored"
				else
					echo "/bin/login could not be restored, the file may be in use"
				fi
			else
				echo "login.orig not found, $current/login.orig"
			fi

			break
			;;

		[Nn]* )
			break
			;;
		* )
			echo "enter 'y' or 'n'"
			;;
	esac
done
