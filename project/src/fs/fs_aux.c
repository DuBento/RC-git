#include "fs_aux.h"

/*! \brief Fill the base information of the user request.
 *
 *  Checks and fills the UID and TID and sets the variables for the timeouts.
 * 
 * 	\param  userRequest		the pointer to the user request structure.
 * 	\param 	uid				the uid of the request.
 * 	\param	tid				the tid of the request.
 * 	\return TRUE if the fill was successfull, FALSE otherwise.
 */
static bool_t _fillBaseRequest(userRequest_t *userRequest, const char *uid, const char *tid) {
    if (!isUIDValid(uid) || !isTIDValid(tid))
		return FALSE;

    static int fsidgen = 0;
    userRequest->fsid = fsidgen++;
    strcpy(userRequest->uid, uid);
	strcpy(userRequest->tid, tid);
    return TRUE;
}


// prepares a list request from the user.
bool_t fillListRequest(userRequest_t *userRequest, const char* uid, const char *tid) {
    if (!_fillBaseRequest(userRequest, uid, tid)) {
        tcpSendMessage(userRequest->tcpConnection, RESP_LST " ERR\n", 8);
        return FALSE;
    }
    
    userRequest->fop = FOP_L;
    userRequest->exeRequest = listRequest;
    strcpy(userRequest->replyHeader, RESP_LST);
    return TRUE;
}


// prepares a retreive request from the user
bool_t fillRetreiveRequest(userRequest_t *userRequest, const char* uid, const char *tid, const char *fname) {
    if (!_fillBaseRequest(userRequest, uid, tid)) {
        tcpSendMessage(userRequest->tcpConnection, RESP_RTV " ERR\n", 8);
        return FALSE;
    }

    userRequest->fop = FOP_R;
    userRequest->exeRequest = retreiveRequest;
    strcpy(userRequest->replyHeader, RESP_RTV);
    
    userRequest->fileName = (char*)malloc((strlen(fname) + 1) * sizeof(char));
    if (userRequest->fileName == NULL) {
        tcpSendMessage(userRequest->tcpConnection, RESP_RTV " ERR\n", 8);
        return FALSE;
    }   

    strcpy(userRequest->fileName, fname);
    return TRUE;
}


// prepares a upload request from the user
bool_t fillUploadRequest(userRequest_t *userRequest, const char* uid, const char *tid, const char *fname, const char *fsize) {
    if (!_fillBaseRequest(userRequest, uid, tid) || !isStringValid(fsize, STR_DIGIT, STR_ALLLEN)) {
        tcpSendMessage(userRequest->tcpConnection, RESP_UPL " ERR\n", 8);
        return FALSE;
    }
    
    userRequest->fop = FOP_U;
    userRequest->exeRequest = uploadRequest;
    strcpy(userRequest->replyHeader, RESP_UPL);
    
    userRequest->fileName = (char*)malloc((strlen(fname) + 1) * sizeof(char));
    if (userRequest->fileName == NULL) {
        tcpSendMessage(userRequest->tcpConnection, RESP_UPL " ERR\n", 8);
        return FALSE;
    }
    strcpy(userRequest->fileName, fname);

    return  TRUE;   
}


// prepares a delete request from the user
bool_t fillDeleteRequest(userRequest_t *userRequest, const char* uid, const char *tid, const char *fname) {
     if (!_fillBaseRequest(userRequest, uid, tid)) {
        tcpSendMessage(userRequest->tcpConnection, RESP_DEL " ERR\n", 8);
        return FALSE;
    }

    userRequest->fop = FOP_D;
    userRequest->exeRequest = deleteRequest;
    strcpy(userRequest->replyHeader, RESP_DEL);

    userRequest->fileName = (char*)malloc((strlen(fname) + 1) * sizeof(char));
    if (userRequest->fileName == NULL) {
        tcpSendMessage(userRequest->tcpConnection, RESP_DEL " ERR\n", 8);
        return FALSE;
    }   
    
    strcpy(userRequest->fileName, fname);
    return TRUE;
}


// prepares a remove request from the user
bool_t fillRemoveRequest(userRequest_t *userRequest, const char* uid, const char *tid) {
    if (!_fillBaseRequest(userRequest, uid, tid)) {
        tcpSendMessage(userRequest->tcpConnection, RESP_REM " ERR\n", 8);
        return FALSE;
    }

    userRequest->fop = FOP_X;
    userRequest->exeRequest = removeRequest;
    strcpy(userRequest->replyHeader, RESP_REM);
    return  TRUE;
}





// executes the list request
void listRequest(userRequest_t *userRequest, const char *filesPath) {
    List_t files = listFiles(filesPath, userRequest->uid);
    size_t nFiles = listSize(files);
    if (nFiles == 0) {
        tcpSendMessage(userRequest->tcpConnection, RESP_LST " EOF\n", 8);
        listDestroy(files, free);
        return;
    }

    // find the size of the message
    size_t msgSize = 3 + 1 + nDigits(nFiles) + 1;
    ListIterator_t iterator = listIteratorCreate(files);
    while (!listIteratorEmpty(&iterator)) {
        char *fileEnt = (char*)listIteratorNext(&iterator);
        msgSize += strlen(fileEnt);
    }
    
    // alocates memory for the message
    char *msg = (char*)malloc((msgSize + 1) * sizeof(char));
    if (msg == NULL) {
        tcpSendMessage(userRequest->tcpConnection, RESP_RTV " ERR\n", 8);
        listDestroy(files, free);
        return;
    }

    // formats the message
    sprintf(msg, "%s %lu ", RESP_LST, nFiles);
    iterator = listIteratorCreate(files);
    while (!listIteratorEmpty(&iterator)) {
        char *fileEnt = (char*)listIteratorNext(&iterator);
        strcat(msg, fileEnt);
    }
    msg[msgSize - 1] = '\n';
    msg[msgSize] = '\0';

    // sends the message
    tcpSendMessage(userRequest->tcpConnection, msg, msgSize);
    listDestroy(files, free);
    free(msg);
}


// executes the retreive request
void retreiveRequest(userRequest_t *userRequest, const char *filesPath) {
    // checks if the user is registed on the FS
    char dirPath[PATH_MAX];
    sprintf(dirPath, "%s/%s", filesPath, userRequest->uid);
    DIR *directory = opendir(dirPath);
    if (directory == NULL ){
        tcpSendMessage(userRequest->tcpConnection, RESP_RTV " NOK\n", 8);
        return;
    }
    closedir(directory);
    char filePath[PATH_MAX];
    sprintf(filePath, "%s/%s/%s", filesPath, userRequest->uid, userRequest->fileName);

    // formats the message and sends it
    char header[BUFFER_SIZE] = { 0 };
    int headerSize = sprintf(header, "%s OK", RESP_RTV);
    if (!sendFileThroughTCP(userRequest->tcpConnection, filePath, header, headerSize)) {
        tcpSendMessage(userRequest->tcpConnection, RESP_RTV " EOF\n", 8);
        return;
    }
}


// executes the upload request
void uploadRequest(userRequest_t *userRequest, const char *filesPath) {
    char newFilePath[PATH_MAX], tempFilePath[PATH_MAX];
    sprintf(newFilePath, "%s/%s/%s", filesPath, userRequest->uid, userRequest->fileName);
    sprintf(tempFilePath, "%s/%s/~~temp_%d~~", filesPath, userRequest->uid, userRequest->fsid);

	DIR* directory = initDir(filesPath, userRequest->uid, NULL);
    // verifies duplicate files
    if (inDir(directory, userRequest->fileName)) {
        tcpSendMessage(userRequest->tcpConnection, RESP_UPL " DUP\n", 8);
        remove(tempFilePath);
        closedir(directory);
        return;
    }
    closedir(directory);

    // updates the name of the file to the final one
    if (rename(tempFilePath, newFilePath)) {
        tcpSendMessage(userRequest->tcpConnection, RESP_UPL " ERR\n", 8);
        remove(tempFilePath);
        return;
    }

    // checks if the directory is full
    List_t userFiles = listFiles(filesPath, userRequest->uid);
    if (listSize(userFiles) > MAX_FILES) {
        tcpSendMessage(userRequest->tcpConnection, RESP_UPL " FULL\n", 9);
        remove(newFilePath);
        listDestroy(userFiles, free);
        return;
    }
    listDestroy(userFiles, free);

    tcpSendMessage(userRequest->tcpConnection, RESP_UPL " OK\n", 7);
}


// executes the delete request
void deleteRequest(userRequest_t *userRequest, const char *filesPath) {
    // checks if the user is registed on the FS
    char dirPath[PATH_MAX];
    sprintf(dirPath, "%s/%s", filesPath, userRequest->uid);
    DIR *directory = opendir(dirPath);
    if (directory == NULL ){
        tcpSendMessage(userRequest->tcpConnection, RESP_DEL " NOK\n", 8);
        return;
    }
    closedir(directory);

    if (deleteFile(filesPath, userRequest->uid, userRequest->fileName))
        tcpSendMessage(userRequest->tcpConnection, RESP_DEL " OK\n", 7);
    else
        tcpSendMessage(userRequest->tcpConnection, RESP_DEL " EOF\n", 8);
}


// executes the remove request
void removeRequest(userRequest_t *userRequest, const char *filesPath) {
    if (deleteDirectory(filesPath, userRequest->uid))
        tcpSendMessage(userRequest->tcpConnection, RESP_REM " OK\n", 7);
    else
        tcpSendMessage(userRequest->tcpConnection, RESP_REM " NOK\n", 8);
}





// sends a validation request to the AS server.
void validateRequest(UDPConnection_t *asServer, userRequest_t *userRequest) {
    const size_t msgSize = 3 + 1 + UID_SIZE + 1 + TID_SIZE + 1 + 1;
    char msg[msgSize];
    sprintf(msg, "%s %s %s\n", REQ_VLD, userRequest->uid, userRequest->tid);
    udpSendMessage(asServer, msg, msgSize);
}