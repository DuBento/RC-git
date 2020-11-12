#ifndef FILES_H
#define FILES_H

#include "common.h"
#include "list.h"
#include "tcp.h"


/*! \brief Initialize a directory on the specified path.
 *
 *  Opens a directory on the specified path. The directory is created if it does not exist.
 *  Returns a pointer to the directory and stores its path on the 'path' variable
 * 
 * 	\param  path			the path to the main directory.
 * 	\param  dirname			the name of the directory (relative to the path).
 * 	\param  outPath			buffer to hold directory path (can be set to NULL).
 * 	\return the directory pointer.
 */ 
DIR* initDir(const char* path, const char* dirname, char* outPath);


/*! \brief Checks if the specified file is whitin the given directory.
 *
 *  Iterates through the directory and checks in any file matches the specified filename.
 *  If so, returns TRUE, otherwise returns FALSE.
 * 
 * 	\param  dir			the directory where to search.
 * 	\param  filename		the name of the file.
 * 	\return TRUE if the file is whitin the directory, FALSE otherwise.
 */ 
bool_t inDir(DIR* dir, char* filename);


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
List_t listFiles(const char *filesPath, const char *dirname);


/*! Returns the specified file's content.
 *
 *  Returns a pointer to a dynamically allocated buffer with the content of the specified
 *  file (null terminated string).
 * 
 *  \param  filesPath           the path of the main files directory.
 *  \param  dirname             the name of the directory of the file.
 *  \param  filename            the name of the file that is going to be retreived.
 *  \param  contents            a pointer to store the contents of the file (NULL if the file doesn't exit).
 *  \return the number of bytes read to the contents buffer.
 */
size_t retreiveFile(const char *filesPath, const char *dirname, const char *filename, char**contents);


/*! Creates a new file on the directory with the specified contents.
 *
 *  Creates a new file on the specified directory and stores the contents in it. 
 *  If there is already a file in the directory, it will be overwritten.
 * 
 *  \param  filesPath           the path of the main files directory.
 *  \param  dirname             the name of the directory of the file.
 *  \param  filename            the name of the file that is going to be created.
 *  \param  contents            the data to be stored.
 *  \param  len                 the size of the contents.
 *  \return TRUE if the file was written, FALSE otherwise.
 */
bool_t storeFile(const char *filesPath, const char *dirname, const char *filename, const char *contents, size_t len);


/*! Deletes the specified file.
 *
 *  Deletes the specified file from the specified directory.
 * 
 *  \param  filesPath           the path of the main files directory.
 *  \param  dirname             the name of the directory of the file.
 *  \param  filename            the name of the file that is going to be deleted.
 *  \return TRUE if the file was deleted, FALSE if the file didn't exist.
 */
bool_t deleteFile(const char *filesPath, const char *dirname, const char *filename);


/*! Deletes the specified directory.
 *
 *  Deletes the specified directory and all the files stored within.
 * 
 *  \param  filesPath           the path of the main files directory.
 *  \param  dirname             the name of the directory of the file that is going to be deleted.
 *  \return TRUE if the directory was deleted, FALSE if it didn't exist
 */
bool_t deleteDirectory(const char *filesPath, const char *dirname);


bool_t storeFileFromTCP(TCPConnection_t *tcpConnection, const char *filePath, int fileSize, const char *fdata, int size);

bool_t sendFileThroughTCP(TCPConnection_t *tcpConnection, const char *filePath, const char *header, int headerSize);

#endif 	/* FILES */