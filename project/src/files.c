#include "files.h"


// initialize a directory on the specified path
DIR* initDir(const char* exePath, const char* dirname, char* outPath) {
	DIR* d = NULL;
	// make dir path
	int exePathLen = strlen(exePath);
	int dirnameLen = strlen(dirname);
	char tempPath[PATH_MAX];
	char *formatedPath = (outPath == NULL ? tempPath : outPath);
	if (exePathLen + dirnameLen + 2 > PATH_MAX) // + 2 ("/" and "\0")
		_FATAL("The specified path + dirname is too big.\n\t - Max size: %d", PATH_MAX);
		
	char* pathEnd = strrchr(exePath, '/'); 	//find the last occurrence of the '/'	
	if(pathEnd) {
		char *base_path = (char*)malloc((exePathLen + 1) * sizeof(char));
		strncpy(base_path, exePath, pathEnd - exePath);
		base_path[pathEnd - exePath] = '\0';
		sprintf(formatedPath, "%s/%s/", base_path, dirname);
		free(base_path);
	}
	else
		sprintf(formatedPath, "%s/", dirname);
		d = opendir(formatedPath);

		if(d) {
			// dir opened
			return d;	// and path var updated
		} else if (errno == ENOENT || errno == ENOTDIR ) {
			// dir does not exist, create new
			if (mkdir(formatedPath, S_IRUSR|S_IWUSR|S_IXUSR) == -1) 
				_FATAL("Failed to create log directory.\n\t - Error code: %d", errno);

			// retry to open
			d = opendir(formatedPath);
			if (d) return d;
		}

	_FATAL("Failed to open log directory.\n\t - Error code: %d", errno);
}


// checks if the specified file is whitin the given directory.
bool_t inDir(DIR* dir, char* filename){
	struct dirent *ent;
	while ((ent = readdir(dir)) != NULL)
		if (!strcmp(ent->d_name, filename)) 
			return TRUE;

	return FALSE;	
}


// lists all the files of the specified directory
List_t listFiles(const char *filesPath, const char *dirname) {
	List_t list = listCreate();
	DIR *directory = initDir(filesPath, dirname, NULL);
	struct dirent *ent;

	while ((ent = readdir(directory)) != NULL) {
		if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
			char filePath[PATH_MAX];
			sprintf(filePath, "%s/%s/%s", filesPath, dirname, ent->d_name);  
			FILE *file = fopen(filePath, "r");
			fseek(file, 0L, SEEK_END);
			size_t len = ftell(file);
			fclose(file);

			char *filename = (char*)malloc((FILE_NAME_SIZE + 1 + nDigits(len) + 1 + 1) * sizeof(char));
			if (filename == NULL) FATAL("[Files] Failed to allocate memory.");
			sprintf(filename, "%.24s %lu ", ent->d_name, len);
			listInsert(list, filename);
		}
	}
	
	closedir(directory);
	return list;        
}


// returns the specified file's content
size_t retreiveFile(const char *filesPath, const char *dirname, const char *filename, char **contents) {
	char filePath[PATH_MAX];
	sprintf(filePath, "%s/%s/%s", filesPath, dirname, filename);  

	FILE *file = fopen(filePath, "r");
	if (file == NULL) {
		*contents = NULL;
		return 0;
	}

	fseek(file, 0L, SEEK_END);
	size_t len = ftell(file);
	fseek(file, 0L, SEEK_SET);

	*contents = (char*)malloc((len + 1) * sizeof(char));
	if (contents == NULL) FATAL("[Files] Failed to allocate memory.");
	size_t read = fread(*contents, sizeof(char), len, file);
	fclose(file);
	return read;
}


// creates a new file on the directory with the specified contents
bool_t storeFile(const char *filesPath, const char *dirname, const char *filename, const char *contents, size_t len) {
	char filePath[PATH_MAX];
	DIR *directory = initDir(filesPath, dirname, filePath);
	strcat(filePath, filename);

	FILE *file = fopen(filePath, "w");
	if (file == NULL)
		return FALSE;        

	size_t read = fwrite(contents, sizeof(char), len, file);
	fclose(file);
	closedir(directory);
	return read == len;
}


// deletes the specified file
bool_t deleteFile(const char *filesPath, const char *dirname, const char *filename) {
	char filePath[PATH_MAX];
	sprintf(filePath, "%s/%s/%s", filesPath, dirname, filename);
	return remove(filePath) == 0;
}


// deletes the specified directory.
bool_t deleteDirectory(const char *filesPath, const char *dirname) {
	char dirPath[PATH_MAX];
	sprintf(dirPath, "%s/%s/", filesPath, dirname);
	DIR *directory = opendir(dirPath);
	if (!directory)
		return FALSE;

	struct dirent *ent;
	while ((ent = readdir(directory)) != NULL) {
		if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
			char filePath[PATH_MAX];
			sprintf(filePath, "%s%s", dirPath, ent->d_name);
			remove(filePath);
		}
	}
	
	closedir(directory);
	rmdir(dirPath);
	return TRUE;
}


//
bool_t storeFileFromTCP(TCPConnection_t *tcpConnection, const char *filePath, int fileSize, const char *fdata, int fdataSize) {
	FILE *file = fopen(filePath, "w");
	if (file == NULL) {
		return FALSE;
	}
		

	int sizeStored = 0;
	if (fdataSize == fileSize + 1) {
		if (fdata[fdataSize - 1] == '\n') {
			sizeStored = fwrite(fdata, sizeof(char), fileSize + 1, file);
			fclose(file);
			return sizeStored == fileSize + 1;
		} else {
			fclose(file);
			return FALSE;
		}			
	}
	else
		sizeStored = fwrite(fdata, sizeof(char), fdataSize, file);

	while (sizeStored != fileSize + 1) {
		char buffer[BUFFER_SIZE] = { 0 };
		int newRead = tcpReceiveMessage(tcpConnection, buffer, BUFFER_SIZE);
		sizeStored += newRead;
		if (sizeStored + newRead == fileSize + 1) {
			if (buffer[newRead - 1] == '\n') {
				newRead--;
			} else {
				fclose(file);
				return FALSE;
			}
		}

		fwrite(buffer, sizeof(char), newRead, file);
	}

	fclose(file);	
	return sizeStored == fileSize + 1;
}