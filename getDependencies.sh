# Downloads all dependencies which are needed for the engine

download () { #dos2unix 
	if [[ $2 == *.zip ]]; then
		folderName=${2::-4}
	elif [[ $2 == *.tar.gz ]]; then
		folderName=${2::-7}
	else
		folderName=${2}
	fi
	
	if [ ! -d $folderName ]; then
		echo "Donwnloading $2" 
		#wget "https://github.com/g-truc/glm/releases/download/0.9.7.1/glm-0.9.7.1.zip" --no-check-certificate
		#curl -LOk "$1" -o $2
		curl -# "$1" -o $2 -LOk
		if [ -f $2 ]; then
			if [[ $2 == *.zip ]]; then
				unzip -q $2 
			elif [[ $2 == *.tar.gz ]]; then
				tar -f $2 --extract
			else
				echo "Unknown achive format!" >&2
			fi
			rm $2
		else 
			echo "Download failed: file not found!"	
		fi
	fi
}

downloadToFolder () {
	if [ ! -d $3 ]; then
		echo "Donwnloading $2" 
		curl -# "$1" -o $2 -LOk
		if [ -f $2 ]; then
			if [[ $2 == *.zip ]]; then
				unzip -q $2 -d ./$3
			elif [[ $2 == *.tar.gz ]]; then
				tar -f $2 --extract
			else
				echo "Unknown achive format!" >&2
			fi
			rm $2
		else 
			echo "Download failed: file not found!"	
		fi
	fi
}

mkdir -p dependencies
cd dependencies
if [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	echo "Getting Linux dependencies"
	download "https://github.com/g-truc/glm/releases/download/0.9.7.1/glm-0.9.7.1.zip" "glm.zip"
	
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ] || [ "$(expr substr $(uname -s) 1 10)" == "MSYS_NT-10" ]; then
	echo "Getting Windows dependencies"
	download "https://github.com/g-truc/glm/releases/download/0.9.7.1/glm-0.9.7.1.zip" "glm.zip"
else
	echo "Unsupported System"
fi
