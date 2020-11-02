#include "files.h"


// lists all the files of the specified directory
List_t listFiles(DIR *directory) {
    List_t list = listCreate();
    struct dirent *ent;
    while ((ent = readdir(directory)) != NULL) {
        if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
            char *filename = (char*)(malloc((FILE_NAME_SIZE + 5) * sizeof(char)));
            memset(filename, FILE_NAME_SIZE + 5, '\0');
            char *extension = strchr(ent->d_name, '.');
            size_t len = strlen(ent->d_name);

            strncpy(filename, ent->d_name, FILE_NAME_SIZE);
            if (len > FILE_NAME_SIZE)
                strncpy(filename + FILE_NAME_SIZE, extension, 4);

            filename[FILE_NAME_SIZE + 4] = '\0';
            listInsert(list, filename);
        }
    }
    
    return list;        
}