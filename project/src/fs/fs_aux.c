#include "fs_aux.h"

/*! \brief Fill the base information of the user request
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

	userRequest->timeExpired = TIMEOUT + 1.0;
    userRequest->nTries = 0;
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
        
    userRequest->fileName = (char*)malloc((strlen(fname) + 1) * sizeof(char));
    if (userRequest->fileName == NULL) {
        tcpSendMessage(userRequest->tcpConnection, RESP_RTV " NOK\n", 8);
        return FALSE;
    }   

    strcpy(userRequest->fileName, fname);
    return TRUE;
}


// prepares a upload request from the user
bool_t fillUploadRequest(userRequest_t *userRequest, const char* uid, const char *tid, const char *fname, const char *fsize, const char *fdata) {
    if (!_fillBaseRequest(userRequest, uid, tid) || !isStringValid(fsize, STR_DIGIT, 0)) {
        tcpSendMessage(userRequest->tcpConnection, RESP_UPL " ERR\n", 8);
        return FALSE;
    }
    
    userRequest->fop = FOP_U;
    userRequest->exeRequest = uploadRequest;
    userRequest->fileSize = atoi(fsize);

    int fdatalen = strlen(++fdata);
    userRequest->fileName = (char*)malloc((strlen(fname) + 1) * sizeof(char));
    userRequest->data = (char*)malloc((userRequest->fileSize + 2) * sizeof(char));
    if (userRequest->fileName == NULL || userRequest->data == NULL || fdatalen > userRequest->fileSize + 1 || 
        (fdata[fdatalen - 1] == '\n' && fdatalen <= userRequest->fileSize))
    {
        tcpSendMessage(userRequest->tcpConnection, RESP_UPL " NOK\n", 8);
        return FALSE;
    }

    strcpy(userRequest->fileName, fname);
    strncpy(userRequest->data, fdata, fdatalen);

    if (userRequest->fileSize >= fdatalen)
        tcpReceiveMessage(userRequest->tcpConnection, &userRequest->data[fdatalen], userRequest->fileSize + 2 - fdatalen);

    if (userRequest->data[userRequest->fileSize] != '\n') {
        tcpSendMessage(userRequest->tcpConnection, RESP_UPL " ERR\n", 8);
        return FALSE;
    }
    
    userRequest->data[userRequest->fileSize] = '\0';
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

    userRequest->fileName = (char*)malloc((strlen(fname) + 1) * sizeof(char));
    if (userRequest->fileName == NULL) {
        tcpSendMessage(userRequest->tcpConnection, RESP_DEL " NOK\n", 8);
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
    return  TRUE;
}





// executes the list request
void listRequest(userRequest_t *userRequest, const char *filesPath) {

}


// executes the retreive request
void retreiveRequest(userRequest_t *userRequest, const char *filesPath) {

}


// executes the upload request
void uploadRequest(userRequest_t *userRequest, const char *filesPath) {
    // checks if the file is already in the directory
    DIR *userDir = initDir(filesPath, userRequest->uid, NULL);
    if (inDir(userDir, userRequest->fileName)) {
        closedir(userDir);
        tcpSendMessage(userRequest->tcpConnection, REQ_UPL " DUP\n", 8);
        return;
    }
    

    // checks if the directory is full
    List_t userFiles = listFiles(filesPath, userRequest->uid);
    if (listSize(userFiles) >= MAX_FILES) {
        listDestroy(userFiles, free);
        tcpSendMessage(userRequest->tcpConnection, REQ_UPL " FULL\n", 9);
        return;
    }
    

    // cheks if the filename is valid
    if (!isFileNameValid(userRequest->fileName)) {
        tcpSendMessage(userRequest->tcpConnection, REQ_UPL " NOK\n", 8);
        return;
    }

    storeFile(filesPath, userRequest->uid, userRequest->fileName, userRequest->data, userRequest->fileSize);
    tcpSendMessage(userRequest->tcpConnection, REQ_UPL " OK\n", 7);
    closedir(userDir);
    listDestroy(userFiles, free);
}


// executes the delete request
void deleteRequest(userRequest_t *userRequest, const char *filesPath) {

}


// executes the remove request
void removeRequest(userRequest_t *userRequest, const char *filesPath) {

}