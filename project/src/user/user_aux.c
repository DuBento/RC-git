#include "user_aux.h"

/******************************************************************************/
/*									       /
/*			Connections					       /
/*									       /
/******************************************************************************/
bool_t sendUserMessage(TCPConnection_t *tcpConnection, char *msgBuffer, int msgSize) {
	
	int sizeSent;

	if (msgBuffer == NULL) return FALSE;

	sizeSent = tcpSendMessage(tcpConnection, msgBuffer, msgSize);
	if (msgSize != sizeSent) {
		printf("A problem may have occured while sending the  " 
		"message because the whole message was not sent!");
		return FALSE;
	}
	return TRUE;
}
	
bool_t terminateContact(TCPConnection_t *tcpConnection) {
	return TRUE;
}



/******************************************************************************/
/*									       /
/*				Request					       /
/*									       /
/******************************************************************************/


bool_t req_login(TCPConnection_t *asConnection, userInfo_t *userInfo, 
		const char *uid, const char *pass) {
	/* User application sends to the AS the user’s ID UID and a password */
	int sizeSent, msgSize;
	char msgBuffer[BUFFER_SIZE * 2];
	
	//if (userInfo->loggedIn) {
	//	_WARN("A session is already on! " MSG_OP_IGN "\n"
	//	"Current " MSG_UID " %s\n"
	//	"Unregister first if you wish to use another user.", userInfo->uid);
	//	strcpy(userInfo->uid, uid);
	//	strcpy(userInfo->pass, pass);
	//	return TRUE;
	//}

	msgSize = sprintf(msgBuffer, "%s %s %s\n", REQ_LOG, uid, pass);

	//sizeSent = tcpSendMessage(asConnection,  msgBuffer, msgSize);
	//if (msgSize != sizeSent) {
	//	WARN("A problem may have occured while sending the registration request \
	//	because the whole message was not sent!");
	//	return FALSE;
	//}
	if (!sendUserMessage(asConnection, msgBuffer, msgSize)) return TRUE;


	// Adjust size.
	if (!userInfo->loggedIn /*&& userInfo->uid == NULL && userInfo->pass == NULL*/) {
		userInfo->uid = (char*)(malloc((strlen(uid) + 1) * sizeof(char)));
		userInfo->pass = (char*)(malloc((strlen(pass) + 1) * sizeof(char)));
	}
	
	strcpy(userInfo->uid, uid);
	strcpy(userInfo->pass, pass);

	return TRUE;
}


int req_request(TCPConnection_t *asConnection, const userInfo_t *userInfo, 
		const char *fop, const char *fname) {

	int mssgSize, rid, sizeSent;
	char mssgBuffer[2 * BUFFER_SIZE];
	
	// A random natural number of 4 digits is added as a request identifier RID.
	rid = randomNumber(RAND_NUM_MIN, RAND_NUM_MAX);

	//If the operation is retrieve (R), upload (U) or delete (D) also the file name Fname is sent. 
	if ((!strcmp(fop, FOP_STR_R))|| (!strcmp(fop, FOP_STR_U))|| 
	(!strcmp(fop, FOP_STR_D)) ) {
		// what if R is written without fname?
		mssgSize = sprintf(mssgBuffer, "%s %s %.4d %s %s\n", REQ_REQ, 
		userInfo->uid, rid, fop, fname);
	} else {
		mssgSize = sprintf(mssgBuffer, "%s %s %.4d %s\n", REQ_REQ, 
		userInfo->uid, rid, fop);
	}
		
	// Send message = REQ UID RID Fop [Fname] to AS requesting TID.
	if (sendUserMessage(asConnection, mssgBuffer, mssgSize)) {
		return rid;
	} else {
		return FALSE;
	}
}


bool_t req_val(TCPConnection_t *asConnection, const userInfo_t *userInfo, 
		const char *vc, int rid) {

	int mssgSize, sizeSent;
	mssgSize = sizeSent = 0;
	char mssgBuffer[BUFFER_SIZE];

//_LOG("At req val user %s %s", userInfo->uid, userInfo->pass);

	if (rid == RID_INVALID) {
		printf("Make sure you're logged in and make a request first in "
		"order to send a "MSG_VC".\n");
		return FALSE;
	}

	mssgSize = sprintf(mssgBuffer, "%s %s %.4d %s\n", REQ_AUT, 
	userInfo->uid, rid, vc);
	
	if (mssgSize < SPRINTF_THRSHLD)
		WARN(MSG_FLD"write val message to buffer.");
	
	//_LOG("Le buffer %s", mssgBuffer);
	//sizeSent = tcpSendMessage(asConnection, mssgBuffer, mssgSize);

	//if (mssgSize != sizeSent) {
	//	WARN("A problem may have occured while sending the request request!");
	//	return FALSE;
	//}
	//return TRUE;
	// Send message to AS: AUT UID RID VC 
	return sendUserMessage(asConnection, mssgBuffer, mssgSize);
}


bool_t req_list(TCPConnection_t **fsConnection, const userInfo_t *userInfo, 
		const int tid) {
	
	char msgBuffer[BUFFER_SIZE];
	int msgSize, sizeSent;
	TCPConnection_t *fsconnection;

	msgSize = sizeSent = 0;	
	// Establish TCP connection with FS (update ptr outside)
	*fsConnection = fsconnection = tcpCreateClient(connectionInfo.fsip, 
	connectionInfo.fsport);

	tcpConnect(fsconnection);	

	// Send message to FS: LST UID TID 
	msgSize = sprintf(msgBuffer, "%s %s %.4d\n", REQ_LST, userInfo->uid, 
	tid);
	if (msgSize < SPRINTF_THRSHLD) {
		printf(MSG_FLD_SSCANF MSG_FS ".\n");
	}

	return sendUserMessage(fsconnection, msgBuffer, msgSize);
//	sizeSent = tcpSendMessage (fsconnection,  msgBuffer, msgSize);
//	if (msgSize != sizeSent) {
//		printf("A problem may have occured while sending the list " 
//		"request because the whole message was not sent!");
//		return FALSE;
//	}
//	return TRUE;
}


bool_t req_retrieve(TCPConnection_t **fsConnection, const userInfo_t *userInfo, 
			const int tid, const char *fname, char **filename) {

	char msgBuffer[BUFFER_SIZE];
	int msgSize, sizeSent;
	TCPConnection_t *fsconnection;

	msgSize = sizeSent = 0;	
	// Establish TCP connection with FS (update ptr outside)
	*fsConnection = fsconnection = tcpCreateClient(connectionInfo.fsip, 
	connectionInfo.fsport);
	tcpConnect(fsconnection);		

	// Send message to FS: RTV UID TID Fname
	msgSize = sprintf(msgBuffer, "%s %s %.4d %s\n", REQ_RTV, userInfo->uid, 
	tid, fname);

	if (msgSize < SPRINTF_THRSHLD) {
		printf(MSG_FLD_SSCANF MSG_FS ".\n");
	}

	if (!sendUserMessage(fsconnection,  msgBuffer, msgSize))
		return FALSE;
	
	// Store fname so it can be used to download file.
	*filename = (char*) malloc ((strlen(fname)+1)*sizeof(char));
	if (*filename == NULL) {
		printf(MSG_FLD"allocate space to store fname in retrieve request.\n");
	}
	strcpy(*filename, fname);
	return TRUE;
}


bool_t req_upload(TCPConnection_t **fsConnection, const userInfo_t *userInfo, 
			const int tid, const char *filename) {

	char *msgBuffer, *data, *writeData;
	int msgSize, sizeSent, filenameSize, expectedMsgSize;
	TCPConnection_t *fsconnection;
	ssize_t fileSize;

	if (filename == NULL) {
		printf("How the hell can I upload a file without a filename?\n");
		return FALSE;
	}

	msgSize = sizeSent = fileSize = filenameSize = 0;
	msgBuffer = data = NULL;

	// Establish TCP connection with FS (update ptr outside).
	*fsConnection = fsconnection = tcpCreateClient(connectionInfo.fsip, 
	connectionInfo.fsport);
	tcpConnect(fsconnection);

	// Get data
	fileSize = retreiveFile(".","\0",filename, &data);
	if (fileSize == 0 || data == NULL) {
		printf("Failed to retreive file from dir.\n");
		// Close FS TCP connection
		*fsConnection = tcpDestroySocket(fsconnection);
		return FALSE;
	}
	//todo how to check this function?
	expectedMsgSize = CMD_PRT_SIZE+SEPARATOR_SIZE+UID_SIZE+SEPARATOR_SIZE
			+TID_SIZE+SEPARATOR_SIZE+strlen(filename)
			+SEPARATOR_SIZE+nDigits(fileSize)+SEPARATOR_SIZE+fileSize
			+SEPARATOR_SIZE;


_LOG("upload buufer data %s le file size %d", data, fileSize);

	// Send message to FS: UPL UID TID Fname Fsize data
	msgBuffer = (char*) malloc((expectedMsgSize+1)*sizeof(char));

	if (msgBuffer == NULL) {
		printf(MSG_FLD "allocate buffer to send upload message.\n");
		free(data);
		// Close FS TCP connection
		*fsConnection = tcpDestroySocket(fsconnection);
		return TRUE; // because connection is on - tbd change FALSE
	}
	

	msgSize = sprintf(msgBuffer, "%s %s %.4d %s %d ", REQ_UPL, userInfo->uid, 
	tid, filename, fileSize);
_LOG("msg size %d", msgSize);
	memcpy(msgBuffer + msgSize, data, fileSize);

	// Insert end char to respect protocol.
	msgBuffer[expectedMsgSize-1] = PRT_TERM_CHAR;

	//if (expectedMsgSize > msgSize) {
	//	printf(MSG_FLD"prepare upload message to "MSG_FS".\n.");
	//}

	sendUserMessage(fsconnection, msgBuffer, expectedMsgSize);
	//sizeSent = tcpSendMessage(fsconnection, msgBuffer, msgSize);
	//if (msgSize != sizeSent) {
	//	printf("A problem may have occured while sending the upload " 
	//	"request because the whole message was not sent!");
	//	free(msgBuffer);
	//	free(data);
		// disconnect from fs here?
	//	return TRUE; // because connection is on - tbd change FALSE
	//}
	free(msgBuffer);
	free(data);
	return TRUE;
}


bool_t req_delete(TCPConnection_t **fsConnection, const userInfo_t *userInfo, 
			const int tid, const char *filename) {
	
	char msgBuffer[BUFFER_SIZE];
	int msgSize, sizeSent;
	TCPConnection_t *fsconnection;

	msgSize = sizeSent = 0;	

	// Establish TCP connection with FS (update ptr outside)
	*fsConnection = fsconnection = tcpCreateClient(connectionInfo.fsip, 
	connectionInfo.fsport);
	tcpConnect(fsconnection);	

	// Send message to FS: DEL UID TID Fname
	msgSize = sprintf(msgBuffer, "%s %s %.4d %s\n", REQ_DEL, userInfo->uid, 
	tid, filename);
	if (msgSize == SSCANF_FAILURE) {
		printf("Failed to prepare delete message to FS.");
	}
	
	return sendUserMessage(fsconnection, msgBuffer, msgSize);
	
	//sizeSent = tcpSendMessage(fsconnection,  msgBuffer, msgSize);
	//if (msgSize != sizeSent) {
	//	printf("A problem may have occured while sending the delete " 
	//	"request because the whole message was not sent!");
	//	return FALSE;
	//}
	//return TRUE;
}


bool_t req_remove(TCPConnection_t **fsConnection, const userInfo_t *userInfo, const int tid) {
	
	char msgBuffer[BUFFER_SIZE];
	int msgSize, sizeSent;
	TCPConnection_t *fsconnection;

	msgSize = sizeSent = 0;	

	// Establish TCP connection with FS (update ptr outside)
	*fsConnection = fsconnection = tcpCreateClient(connectionInfo.fsip, 
	connectionInfo.fsport);
	tcpConnect(fsconnection);		

	// Send to FS: REM UID TID
	msgSize = sprintf(msgBuffer, "%s %s %.4d\n", REQ_REM, userInfo->uid, 
	tid);

	if (msgSize == SSCANF_FAILURE) {
		printf(MSG_FLD"prepare remove message to "MSG_FS".\n");
	}

	return sendUserMessage(fsconnection, msgBuffer, msgSize);
	//sizeSent = tcpSendMessage(fsconnection,  msgBuffer, msgSize);
	//if (msgSize != sizeSent) {
	//	printf("A problem may have occured while sending the remove " 
	//	"request because the whole message was not sent!");
	//	return FALSE;
	//}
	//return TRUE;
}


bool_t req_resendLastMessage() {
	LOG("Resending last user message...");
	return TRUE;
}


/******************************************************************************/
/*									       /
/*				Response				       /
/*									       /
/******************************************************************************/


bool_t resp_login(userInfo_t *userInfo, char *status) {

	if (!strcmp(status,STATUS_OK)) {
		printf(MSG_SUC_LOG"\n");
		return TRUE;
	} else if (!strcmp(status, STATUS_NOK)) {
		printf(MSG_FLD_LOG_PSW" "MSG_TRY_AGAIN"\n"
			MSG_HELP_REGPD"\n");
	} else if (!strcmp(status, SERVER_ERR)) {
		printf(MSG_FLD_AUT "\n"
			MSG_HELP_VLDUID"\n"
			MSG_HELP_VLDPSW"\n"); 
	}
	free(userInfo->uid);
	free(userInfo->pass);

	userInfo->uid = NULL;
	userInfo->pass = NULL;
	return FALSE;
}


bool_t resp_request(char *status) {

		if (!strcmp(status, STATUS_OK))
			printf(MSG_SUC_REQ"\n");
		else if (!strcmp(status, STATUS_ELOG))
			printf(MSG_FLD_LOG "\n"
			MSG_HELP_REGPD"\n");
		else if (!strcmp(status, STATUS_EPD))
			printf(MSG_AS MSG_FLD_CONTACT MSG_FLD_CONTACT MSG_PD"\n"
			MSG_HELP_REGPD"\n");
		else if (!strcmp(status, STATUS_EUSER))
			printf(MSG_FLD_UID".\n");
		else if (!strcmp(status, STATUS_EFOP))
			printf(MSG_ERR_INV_FOP"\n"
				MSG_HELP_UPCASE"\n"
				MSG_HELP_VLDFOP"\n");
		else if (!strcmp(status, SERVER_ERR))
			printf(MSG_ERR_INV_FMT "\n"
				MSG_HELP_PRVLOG"\n"
				MSG_HELP_FNAME"\n");	

	// DO NOT close connection to AS
	return TRUE;
}


int resp_val(char *tidStr) {

	int tid;
	if (sscanf(tidStr, "%d", &tid) == SSCANF_FAILURE) {
		printf(MSG_FLD"receive " MSG_TID" from "MSG_AS"."
		MSG_TRY_AGAIN"\n");
		return FALSE;
	}
	//todo check is digit?

	if (tid == RID_INVALID) {
		printf(MSG_FLD_AUT"\n"
		MSG_HELP_CORRVC"\n"
		MSG_HELP_DUPVC"\n"
		MSG_HELP_REGPD"\n"
		MSG_HELP_MSGPD"\n");
		return FALSE;
	} else {
		printf(MSG_SUC_AUT" The "MSG_TID" for the request you"
		" asked for is %.4d.\n", tid);
		return tid;
	}
}


//-------------------------------------FS---------------------------------------


bool_t resp_list(TCPConnection_t **fsConnection, char *data) {
	
	TCPConnection_t *fsconnection;
	int numFiles, fnameLen;
	char status[BUFFER_SIZE], fname[BUFFER_SIZE], fsize[BUFFER_SIZE];

	fsconnection = *fsConnection;

	sscanf(data, "%s", status);

_LOG("fkng status %s", status);

	if (!strcmp(status, FILE_NOT_AVAILABLE)) {
		printf(MSG_FILES_DNE " in the " MSG_FS".\n"
		"\n");
	} else if (!strcmp(status, STATUS_INV)) {
		printf(MSG_AS MSG_FLD_VLD MSG_TID".\n");
	} else if (!strcmp(status, SERVER_ERR)) {
		printf(MSG_ERR_INV_REQ"\n");
	} else {
		
		numFiles = (int) strtol(status, (char**)NULL, 10);
		if (numFiles == 0) {
			printf(MSG_FLD"get number of files on list operation.\n");
		}

		data += SEPARATOR_SIZE+nDigits(numFiles) ;	// shift pointer: space + numDigits(numFiles)

		// Display list of files to user.	
		printf(LST_TABLE_HDR);

		for(int i = 1; i <= numFiles; i++) {
			if (sscanf(data, "%s %s", fname, fsize) == SSCANF_FAILURE) {
				printf(MSG_FLD"get fname or fsize from list operation.\n");
			}
			// Display file to user.
			fnameLen = strlen(fname);
			printf("%d.\t%s%*c%s\n", i, fname,
				FILE_NAME_SIZE-fnameLen,' ',fsize);
			data += fnameLen +SEPARATOR_SIZE+strlen(fsize)+SEPARATOR_SIZE;
		}
	}
	//}
	// Close TCP connection with FS (update variable outside).
	*fsConnection = tcpDestroySocket(fsconnection);
	return TRUE;
}


bool_t resp_retrieve(TCPConnection_t **fsConnection, char *response, char **filename, int tcpMsgSize) {
/*RRT status [Fsize data] */
	TCPConnection_t *fsconnection;
	fsconnection = *fsConnection;
	char *data, status[BUFFER_SIZE], size[BUFFER_SIZE], *fname/*, *fdata*/;
	int fsize, datalen, statuslen, sizelen;

//todo confirm if \n comes at the end of the string
_LOG("retrv %s", response);

	fname = *filename;
	if (sscanf(response, "%s", status) == SSCANF_FAILURE) {
		printf(MSG_FLD"get response from retrieve.\n");
	}


	if (!strcmp(status, STATUS_OK)) {
		// shift pointer to reach fsize
		statuslen = strlen(status);
		data = response + statuslen + SEPARATOR_SIZE;
		
		if (sscanf(data, "%s", size) == SSCANF_FAILURE) {
			printf(MSG_FLD"get file size from retrieve.\n");
		}
		
		fsize = (int) strtol(size, (char**)NULL, 10);
		sizelen = strlen(size);
		// shift pointer to reach beginning of already received data
		data += sizelen + SEPARATOR_SIZE;
	//	datalen = strlen(data);
_LOG("xxxxxxxxxxxxxxxxxxxxxxxxxxxx%d",&data[tcpMsgSize-(PROTOCOL_MSSG_OFFSET+statuslen+SEPARATOR_SIZE+
		sizelen+SEPARATOR_SIZE)]-data);
		if (!storeFileFromTCP(fsconnection, fname, fsize, data, 
		&data[tcpMsgSize-(PROTOCOL_MSSG_OFFSET+statuslen+SEPARATOR_SIZE+
		sizelen+SEPARATOR_SIZE)]-data));
			printf(MSG_FLD "store file.\n");


		// Pointer that will contain *all* file data
		//fdata = (char*) malloc ((fsize+2*SEPARATOR_SIZE)*sizeof(char));
		
		// Copy the first part (what we've already captured in the buffer).
		//strcpy(fdata, data);

		// We still need to get the rest of the file, damnit
		//if (datalen < fsize) {
		//	tcpReceiveMessage(fsconnection, fdata+datalen, fsize-datalen+2);
		//}
		
		//if (!storeFile(CURRENT_DIR, CURRENT_DIR, fname, fdata, (ssize_t) fsize)) {
		//	printf(MSG_FLD "store file %s.\n", fname);
		//}

		printf("Retrieve request of file %s successeful.\n", *filename);
		
	} else if (!strcmp(status, FILE_NOT_AVAILABLE)) {
		printf("File %s not available.\n", fname);
	} else if (!strcmp(status, STATUS_NOK)) {
		printf("There is no content available in "MSG_FS
		" for the user with your "MSG_UID".\n");
	} else if (!strcmp(status, STATUS_INV)) {
		printf(MSG_AS MSG_FLD_VLD "retrieve request.\n");
	} else if (!strcmp(status, SERVER_ERR)) {
		printf(MSG_ERR_INV_REQ"\n");
	}

	// Close down FS TCP connection.
	*fsConnection = tcpDestroySocket(fsconnection);
	free(fname);
	//free(fdata);
	*filename = NULL;
	return TRUE;
}


bool_t resp_upload(TCPConnection_t **fsConnection, char *status) {
/* RUP status */
	TCPConnection_t *fsconnection;
	fsconnection = *fsConnection;
	if (!strcmp(status, STATUS_OK))
		printf ("Upload successeful.\n");
	//else if (!strcmp(status, STATUS_NOK))
	//	printf (MSG_UID MSG_DNE " in the " MSG_FS ".\n"); does not make any sense
	else if (!strcmp(status, STATUS_DUP))
		printf ("The file already existed in the "MSG_FS".\n");
	else if (!strcmp(status, STATUS_FULL))
		printf("You have reached "MSG_MAXFILES" stored in the"MSG_FS".\n");
	else if (!strcmp(status, STATUS_INV))
		printf(MSG_FLD_AUT"\n");
	else if (!strcmp(status, SERVER_ERR))
		printf (MSG_ERR_INV_REQ"\n");
	
	// Close FS TCP connection
	*fsConnection = tcpDestroySocket(fsconnection);
	return TRUE;
}


bool_t resp_delete(TCPConnection_t **fsConnection, char *status) {
/*RDL status */
	TCPConnection_t *fsconnection;
	fsconnection = *fsConnection;

	if (!strcmp(status, STATUS_OK)) {
		printf("Successefully deleted the file.\n");
	} else if (!strcmp(status, FILE_NOT_AVAILABLE)) {
		printf("File not available in the "MSG_FS".\n");
	} else if (!strcmp(status, STATUS_NOK)) {
		printf(MSG_UID MSG_DNE " in the "MSG_FS".\n");
	} else if (!strcmp(status, STATUS_INV)) {
		printf(MSG_AS MSG_FLD_VLD MSG_TID".\n");
	} else if (!strcmp(status, SERVER_ERR)) {
		printf(MSG_ERR_INV_REQ"\n"
			MSG_HELP_DUPVC"\n");
	} else {
		printf("Error in communication on delete response.\n");
	}

	// Close FS TCP connection
	*fsConnection = tcpDestroySocket(fsconnection);
	return TRUE;
}


bool_t resp_remove(TCPConnection_t **fsConnection, const userInfo_t *userInfo, char *status) {

	TCPConnection_t *fsconnection;
	fsconnection = *fsConnection;
	if (!strcmp(status, STATUS_OK))
		printf(MSG_SUC_REM" "MSG_THANKS"\n");
	else if (!strcmp(status, STATUS_NOK))
		printf(MSG_UID" %s "MSG_DNE"\n", userInfo->uid);
	else if (!strcmp(status, STATUS_INV))
		printf(MSG_AS MSG_FLD_VLD MSG_TID".\n");
	else if (!strcmp(status, SERVER_ERR))
		printf(MSG_ERR_INV_FMT "\n"
			MSG_HELP_PRVLOG"\n");
	
	// Close FS connection
	*fsConnection = tcpDestroySocket(fsconnection);
	return TRUE;
}