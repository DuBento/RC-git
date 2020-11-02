#ifndef FILES_H
#define FILES_H

#include "../common.h"
#include "../list.h"

#define FILES_DIR "files/"


/*! Lists all the files in the specified directory.
 *
 *  Stores up to FILE_NAME_SIZE characters of each file of the specifiied directory
 *  in the specified buffer and returns the number of characters of the buffer.
 *  The buffer will be dynamicly allocated by this function.
 * 
 *  \param  directory            the directory from where to list.
 *  \return a list with all the file names of the directory.
 */
List_t listFiles(DIR *directory);



#endif 	/* FILES */