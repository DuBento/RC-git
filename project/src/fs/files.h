#ifndef FILES_H
#define FILES_H

#include "../common.h"
#include "../list.h"

extern char filesPath[PATH_MAX];


/*! Lists all the files in the specified directory.
 *
 *  Stores up to FILE_NAME_SIZE characters of each file of the specifiied directory
 *  in the specified buffer and returns the number of characters of the buffer.
 *  The buffer will be dynamicly allocated by this function.
 * 
 *  \param  directory            the directory from where to list.
 *  \return a list with all the file names of the directory.
 */
List_t listFiles(const char *directory);


/*! Returns the specified file's content.
 *
 *  Returns a pointer to a dynamically allocated buffer with the content of the specified
 *  file (null terminated string).
 * 
 *  \param  directory           the name of the directory.
 *  \param  filename            the name of the file.
 *  \return a buffer with the file's content
 */
char* getFile(const char *directory, const char *filename);


#endif 	/* FILES */