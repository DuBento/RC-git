#ifndef FILES_H
#define FILES_H

#include "../common.h"
#include "../list.h"


/*! Lists all the files in the specified directory.
 *
 *  Stores up to FILE_NAME_SIZE characters of each file of the specifiied directory
 *  in the specified buffer and returns the number of characters of the buffer.
 *  The buffer will be dynamicly allocated by this function.
 * 
 *  \param  filesPath           the path of the main files directory.
 *  \param  dirname             the name of the directory.
 *  \return a list with all the file names of the directory.
 */
List_t listFiles(const char *directory, const char *dirname);


/*! Returns the specified file's content.
 *
 *  Returns a pointer to a dynamically allocated buffer with the content of the specified
 *  file (null terminated string).
 * 
 *  \param  filesPath           the path of the main files directory.
 *  \param  dirname             the name of the directory of the file.
 *  \param  filename            the name of the file.
 *  \param  contents            a pointer to store the contents of the file.
 *  \return the number of bytes read to the contents buffer.
 */
size_t getFile(const char *directory, const char *dirname, const char *filename, char**contents);


#endif 	/* FILES */