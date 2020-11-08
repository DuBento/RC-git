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
	
	//if (userInfo->asConnected) {
	//	_WARN("A session is already on! Operation ignored.\n\t - Current uid: %s\n\t"
	//	"Unregister first if you wish to use another user.\n", userInfo->uid);
	//	return FALSE;
	//}

	msgSize = sprintf(msgBuffer, "%s %s %s\n", REQ_LOG, uid, pass);

	//sizeSent = tcpSendMessage(asConnection,  msgBuffer, msgSize);
	//if (msgSize != sizeSent) {
	//	WARN("A problem may have occured while sending the registration request \
	//	because the whole message was not sent!");
	//	return FALSE;
	//}
	if (!sendUserMessage(asConnection, msgBuffer, msgSize)) return FALSE;


	// Adjust size.
	userInfo->uid = (char*)(malloc((strlen(uid) + 1) * sizeof(char)));
	userInfo->pass = (char*)(malloc((strlen(pass) + 1) * sizeof(char)));
	
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
	//sizeSent = tcpSendMessage(asConnection, mssgBuffer, mssgSize);

	//if (mssgSize != sizeSent) {
	//	WARN("A problem may have occured while sending the request request!");
	//	return FALSE;
	//}

	if (sendUserMessage(asConnection, mssgBuffer, mssgSize)) {
		return rid;
	} else {
		return FALSE;
	}
	//return rid;
}


bool_t req_val(TCPConnection_t *asConnection, const userInfo_t *userInfo, 
		const char *vc, int rid) {

	int mssgSize, sizeSent;
	mssgSize = sizeSent = 0;
	char mssgBuffer[BUFFER_SIZE];

//_LOG("At req val user %s %s", userInfo->uid, userInfo->pass);

	if (rid == RID_INVALID) {
		printf("Make a request first in order to send a "MSG_VC".\n");
		return FALSE;
	}

	mssgSize = sprintf(mssgBuffer, "%s %s %.4d %s\n", REQ_AUT, 
	userInfo->uid, rid, vc);
	
	if (mssgSize < 0)
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
	if (msgSize == SSCANF_FAILURE) {
		printf(MSG_FLD"prepare list message to "MSG_FS".\n");
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

	if (msgSize == SSCANF_FAILURE) {
		printf(MSG_FLD_SSCANF MSG_FS ".\n");
	}


	//sizeSent = tcpSendMessage(fsconnection,  msgBuffer, msgSize);
	//if (msgSize != sizeSent) {
	//	printf("A problem may have occured while sending the list " 
	//	"request because the whole message was not sent!");
	//	return FALSE;
	//}

	if (!sendUserMessage(fsconnection,  msgBuffer, msgSize))
		return FALSE;
	
	// Store fname so it can be used to download file.
	*filename = (char*) malloc ((strlen(fname)+1)*sizeof(char));
	if (*filename == NULL) {
		printf(MSG_FLD"allocate space to store fname in retreive request.\n");
	}
	strcpy(*filename, fname);
	return TRUE;
}


bool_t req_upload(TCPConnection_t **fsConnection, const userInfo_t *userInfo, 
			const int tid, const char *filename) {

	char *msgBuffer, *data;
	int msgSize, sizeSent, filenameSize;
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
	//todo how to check this function?

	// Send message to FS: UPL UID TID Fname Fsize data
	msgBuffer = (char*) malloc((3+1+UID_SIZE+1+TID_SIZE+1+strlen(filename)+1
		+fileSize+1+fileSize)*sizeof(char));

	if (msgBuffer == NULL) {
		printf(MSG_FLD "allocate buffer to send upload message.\n");
		free(data);
		return TRUE; // because connection is on - tbd change FALSE
	}
	
	msgSize = sprintf(msgBuffer, "%s %s %.4d %s %d %s\n", REQ_UPL, userInfo->uid, 
	tid, filename, fileSize, data);

	if (msgSize == SSCANF_FAILURE) {
		printf(MSG_FLD"prepare upload message to "MSG_FS".\n.");
	}

	sendUserMessage(fsconnection, msgBuffer, msgSize);
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


bool_t resp_login(char *status) {

	if (!strcmp(status,STATUS_OK)) 
		printf(MSG_SUC_LOG"\n");
	else if (!strcmp(status, STATUS_NOK))
		printf(MSG_FLD_LOG_PSW"\n"
			MSG_HELP_REGPD"\n");
	else if (!strcmp(status, SERVER_ERR))
		printf(MSG_FLD_AUT "\n"
		MSG_HELP_VLDUID"\n"
		MSG_HELP_VLDPSW"\n");
	return TRUE;
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
		" asked for is %.4d\n.", tid);
		return tid;
	}
}


//-------------------------------------FS---------------------------------------


bool_t resp_list(TCPConnection_t **fsConnection, char *data) {
	
	TCPConnection_t *fsconnection;
	int numFiles;
	char status[BUFFER_SIZE], fname[BUFFER_SIZE], fsize[BUFFER_SIZE];

	fsconnection = *fsConnection;

	sscanf(data, "%s", status);

	if (!strcmp(status, FILE_NOT_AVAILABLE)) {
		printf(MSG_FILES_DNE " in the " MSG_FS".\n");
	} else if (!strcmp(status, STATUS_INV)) {
		printf(MSG_AS MSG_FLD_VLD MSG_TID".\n");
	} else if (!strcmp(status, SERVER_ERR)) {
		printf(MSG_ERR_INV_REQ"\n");
	}
	
	numFiles = (int) strtol(status, (char**)NULL, 10);
	if (numFiles == 0) {
		printf(MSG_FLD"get number of files on list operation.\n");
	}
	
	data += 2+numFiles/10;	// shift pointer: space + numDigits(numFiles)
	
	// Display list of files to user.	
	printf(LST_TABLE_HDR);
	
	for(int i = 1; i <= numFiles; i++) {
		if (sscanf(data, "%s %s", fname, fsize) == SSCANF_FAILURE) {
			printf(MSG_FLD"get fname or fsize from list operation.\n");
		}
		// Display file to user.
		printf("%d.\t%s\t\t\t%s\n", i, fname, fsize);
		data += strlen(fname)+1+strlen(fsize);
	}

	// Close TCP connection with FS (update variable outside).
	*fsConnection = tcpDestroySocket(fsconnection);
	return TRUE;
}


bool_t resp_retrieve(TCPConnection_t **fsConnection, char *response, char **filename) {
/*RRT status [Fsize data] */
	TCPConnection_t *fsconnection;
	fsconnection = *fsConnection;
	char *data, status[BUFFER_SIZE], size[BUFFER_SIZE], *fname;
	int fsize;

_LOG("retrv %s", response);

	fname = *filename;
	if (sscanf(response, "%s", status) == SSCANF_FAILURE) {
		printf(MSG_FLD"get response from retrieve.\n");
	}


	if (!strcmp(status, STATUS_OK)) {
		// shift pointer to reach fsize
		data = response + strlen(status) + 1;
		
		if (sscanf(data, "%s", size) == SSCANF_FAILURE) {
			printf(MSG_FLD"get file size from retrieve.\n");
		}
		
		fsize = (int) strtol(size, (char**)NULL, 10);

		// shift pointer to reach data
		data = data + strlen(size) + 1;
		
		if (!storeFile(CURRENT_DIR, CURRENT_DIR, fname, data, (ssize_t) fsize)) {
			printf(MSG_FLD "store file %s.\n", fname);
		}

		printf("Retrieve request of file %s successeful.\n", *filename);
		
	} else if (!strcmp(status, FILE_NOT_AVAILABLE)) {
		printf("File %s not available.\n", fname);
	} else if (!strcmp(status, STATUS_NOK)) {
		printf("There is no content available in "MSG_FS
		" for the user with your "MSG_UID".\n");
	} else if (!strcmp(status, STATUS_INV)) {
		printf(MSG_AS MSG_FLD_VLD "retreive request.\n");
	} else if (!strcmp(status, SERVER_ERR)) {
		printf(MSG_ERR_INV_REQ);
	}
	// Close down FS TCP connection.
	*fsConnection = tcpDestroySocket(fsconnection);
	free(fname);
	*filename = NULL;
	return TRUE;
}


bool_t resp_upload(TCPConnection_t **fsConnection, char *status) {
/* RUP status */
	TCPConnection_t *fsconnection;
	fsconnection = *fsConnection;
	if (!strcmp(status, STATUS_OK))
		printf ("Upload successeful.\n");
	else if (!strcmp(status, STATUS_NOK))
		printf (MSG_UID MSG_DNE "in the " MSG_FS ".\n");
	else if (!strcmp(status, STATUS_DUP))
		printf ("The file already existed in the "MSG_FS".\n");
	else if (!strcmp(status, STATUS_FULL))
		printf("You have reached "MSG_MAXFILES" stored in the"MSG_FS".\n");
	else if (!strcmp(status, STATUS_INV))
		printf(MSG_FLD_AUT"\n");
	else if (!strcmp(status, SERVER_ERR))
		printf (MSG_ERR_INV_REQ);
	
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


bool_t resp_remove(TCPConnection_t **fsConnection, char *status) {

	TCPConnection_t *fsconnection;
	fsconnection = *fsConnection;
	if (!strcmp(status, STATUS_OK))
		printf(MSG_SUC_REM);
	else if (!strcmp(status, STATUS_NOK))
		printf(MSG_UID MSG_DNE"\n");
	else if (!strcmp(status, STATUS_INV))
		printf(MSG_AS MSG_FLD_VLD MSG_TID "\n");
	else if (!strcmp(status, SERVER_ERR))
		printf(MSG_ERR_INV_FMT "\n"
			MSG_HELP_PRVLOG"\n");
	
	// Close FS connection
	*fsConnection = tcpDestroySocket(fsconnection);
	return TRUE;
}