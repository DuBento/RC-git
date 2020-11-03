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
    
    return list;        
}


// returns the specified file's content
size_t getFile(const char *filesPath, const char *dirname, const char *filename, char **contents) {
    char filePath[PATH_MAX];
    sprintf(filePath, "%s/%s/%s", filesPath, dirname, filename);  

    FILE *file = fopen(filePath, "r");
    fseek(file, 0L, SEEK_END);
    size_t len = ftell(file);
    fseek(file, 0L, SEEK_SET);

    *contents = (char*)malloc((len + 1) * sizeof(char));
    //char *tempContents = contents, *readPos = fgets(contents, len, file);
    //while (*(readPos = fgets(tempContents, len, file)) != EOF)
      //  tempContents = readPos + 1;

    size_t read = fread(*contents, sizeof(char), len, file);
    fclose(file);
    return read;
}
