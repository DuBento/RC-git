#include "files.h"



// lists all the files of the specified directory
List_t listFiles(const char *filesPath, const char *dirname) {
	List_t list = listCreate();
	DIR *directory = initDir(filesPath, dirname, NULL);
	struct dirent *ent;

	while ((ent = readdir(directory)) != NULL) {
		if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
			char *filename = (char*)malloc((FILE_NAME_SIZE + 1) * sizeof(char));
			memset(filename, FILE_NAME_SIZE + 1, '\0');
			char *extension = strchr(ent->d_name, '.');
			size_t len = strlen(ent->d_name);

			strncpy(filename, ent->d_name, FILE_NAME_SIZE);
			if (len > FILE_NAME_SIZE)
				strncpy(filename + FILE_NAME_SIZE - 4, extension, 4);

			filename[FILE_NAME_SIZE] = '\0';
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
	return read == len;
}


// deletes the specified file
bool_t deleteFile(const char *filesPath, const char *dirname, const char *filename) {
	char filePath[PATH_MAX];
	sprintf(filePath, "%s/%s/%s", filesPath, dirname, filename);
	return remove(filePath) == 0;
}


// deletes the specified directory.
void deleteDirectory(const char *filesPath, const char *dirname) {
	char dirPath[PATH_MAX];
	DIR *directory = initDir(filesPath, dirname, dirPath);
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
}