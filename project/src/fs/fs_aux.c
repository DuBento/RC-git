#include "fs_aux.h"

// prepares a list request from the user.
bool_t fillListRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid) {
    if (!strcmp(opcode, REQ_LST) && isUIDValid(uid) && isTIDValid(tid)) {
        userRequest->timeExpired = TIMEOUT + 1.0;
        userRequest->nTries = 0;
        userRequest->fop = FOP_L;
        userRequest->exeRequest = NULL;     // TO BE SET
        strcpy(userRequest->uid, uid);
        strcpy(userRequest->tid, tid);
        return  TRUE;
    }

    return FALSE;
}


// prepares a retreive request from the user
bool_t fillRetreiveRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid, 
	const char *fname)
{
    if (!strcmp(opcode, REQ_RTV) && isUIDValid(uid) && isTIDValid(tid) && isFileNameValid(fname)) {
        userRequest->timeExpired = TIMEOUT + 1.0;
        userRequest->nTries = 0;
        userRequest->fop = FOP_R;
        userRequest->exeRequest = NULL;     // TO BE SET
        strcpy(userRequest->uid, uid);
        strcpy(userRequest->tid, tid);
        userRequest->fileName = (char*)malloc((strlen(fname) + 1) * sizeof(char));
        if (userRequest->fileName == NULL)
            return FALSE;
        strcpy(userRequest->fileName, fname);
        return  TRUE;
    }

    return FALSE;
}


// prepares a upload request from the user
bool_t fillUploadRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid, 
	const char *fname, const char *fsize)
{
    if (!strcmp(opcode, REQ_UPL) && isUIDValid(uid) && isTIDValid(tid) && isFileNameValid(fname) && isStringValid(fsize, STR_DIGIT, 0)) {
        userRequest->timeExpired = TIMEOUT + 1.0;
        userRequest->nTries = 0;
        userRequest->fop = FOP_U;
        userRequest->exeRequest = NULL;     // TO BE SET
        strcpy(userRequest->uid, uid);
        strcpy(userRequest->tid, tid);
        userRequest->fileName = (char*)malloc((strlen(fname) + 1) * sizeof(char));
        if (userRequest->fileName == NULL)
            return FALSE;
        strcpy(userRequest->fileName, fname);
        userRequest->fileSize = atoi(fsize);
        userRequest->data = (char*)malloc((userRequest->fileSize + 1) * sizeof(char));
        if (userRequest->data == NULL)
            return FALSE;
        for (int i = 0; i < userRequest->fileSize; i++)
            userRequest->data[i] = '\0';
        return  TRUE;
    }

    return FALSE;
}


// prepares a delete request from the user
bool_t fillDeleteRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid, 
	const char *fname)
{
        if (!strcmp(opcode, REQ_DEL) && isUIDValid(uid) && isTIDValid(tid) && isFileNameValid(fname)) {
        userRequest->timeExpired = TIMEOUT + 1.0;
        userRequest->nTries = 0;
        userRequest->fop = FOP_D;
        userRequest->exeRequest = NULL;     // TO BE SET
        strcpy(userRequest->uid, uid);
        strcpy(userRequest->tid, tid);
        userRequest->fileName = (char*)malloc((strlen(fname) + 1) * sizeof(char));
        if (userRequest->fileName == NULL)
            return FALSE;
        strcpy(userRequest->fileName, fname);
        return  TRUE;
    }

    return FALSE;
}


// prepares a remove request from the user
bool_t fillRemoveRequest(userRequest_t *userRequest, const char *opcode, const char *uid, const char *tid) {
    if (!strcmp(opcode, REQ_REM) && isUIDValid(uid) && isTIDValid(tid)) {
        userRequest->timeExpired = TIMEOUT + 1.0;
        userRequest->nTries = 0;
        userRequest->fop = FOP_X;
        userRequest->exeRequest = NULL;     // TO BE SET
        strcpy(userRequest->uid, uid);
        strcpy(userRequest->tid, tid);
        return  TRUE;
    }

    return FALSE;
}