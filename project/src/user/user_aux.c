#include "user_aux.h"


// logins a user on the authentication system.
bool_t req_login(int fd, userInfo_t *userInfo, const char *uid, const char *pass) {
	/* User application sends to the AS the user’s ID UID and a password */
	if (userInfo->connected) {
		_WARN("A session is already on! Operation ignored.\n\t - Current uid: %s\n\t"
		"Unregister first if you wish to use another user.\n", userInfo->uid);
		return FALSE;
	}

	char msgBuffer[BUFFER_SIZE * 2];
	int msgSize = sprintf(msgBuffer, "%s %s %s\n", REQ_LOG, uid, pass);

	int sizeSent = tcpSendMessage(fd, msgBuffer, msgSize);
	if (msgSize != sizeSent) {
		WARN("A problem may have occured while sending the registration request!");
		return FALSE;
	}

	// Adjust size.
	userInfo->uid = (char*)(malloc((strlen(uid) + 1) * sizeof(char)));
	userInfo->pass = (char*)(malloc((strlen(pass) + 1) * sizeof(char)));
	strcpy(userInfo->uid, uid);
	strcpy(userInfo->pass, pass);
	return TRUE;
}


bool_t req_request(int fd, const userInfo_t *userInfo, const char *fop, 
const char *fname, int *rid) {
	/* User sends a message to the AS requesting a transaction ID code (TID). 
	This request message includes the UID and the type of file operation desired
	 (Fop), either list (L), retrieve (R),
upload (U), delete (D) or remove (X), and if appropriate (when Fop is R, U or D)
also sends the Fname. */

/* User application sends a request to the AS to
inform of the user’s desire to perform the operation Fop (either L, R, U, D or X)
on the FS server. 
 Upon receipt of this message, the AS will send the VLC
message to the PD.*/
	_LOG("User request request, fop: %s fname %s", fop, fname);
	int mssgSize;

	// A random natural number of 4 digits is added as a request identifier RID.
	rid = (int*) malloc(sizeof(int));
	if (rid == NULL)
		FATAL("Failed to malloc rid");
	
	*rid = randomNumber(RAND_NUM_MIN, RAND_NUM_MAX);
	
	char mssgBuffer[2 * BUFFER_SIZE];
	//If the operation is retrieve (R), upload (U) or delete (D) also the file name Fname is sent. 
	if ((!strcmp(fop, FOP_STR_R))|| (!strcmp(fop, FOP_STR_U))|| 
	(!strcmp(fop, FOP_STR_D)) ) {
		// what if R is written without fname?
		mssgSize = sprintf(mssgBuffer, "%s %s %d %s %s\n", REQ_REQ, 
		userInfo->uid, *rid, fop, fname);
	} else {
		mssgSize = sprintf(mssgBuffer, "%s %s %d %s\n", REQ_REQ, 
		userInfo->uid, *rid, fop);
	}
		
	// Send message = REQ UID RID Fop [Fname] to AS requesting TID.
	_LOG("Message to be sent: %s", mssgBuffer);
	int sizeSent = tcpSendMessage(fd, mssgBuffer, mssgSize);

	if (mssgSize != sizeSent) {
		WARN("A problem may have occured while sending the request request!");
		return FALSE;
	}
	return TRUE;
}


bool_t req_val(int fd, const userInfo_t *userInfo, const char *vc, int *rid) {
	/*User has checked the VC on the PD
	User issues this command, sending a message to the AS with the VC. 
	*/
/*AUT UID RID VC
After the user checking the VC on the PD, the User application sends this
message to the AS with the UID and the VC, along with the request identifier
RID, to complete the second factor authentication. A recently generated VC will
be accepted by the AS only once.*/
	int mssgSize;
	mssgSize = 0;
	char mssgBuffer[BUFFER_SIZE];

	if (rid == NULL) {
		WARN("No no no, rid is null! You try again with rid no NULL");
		return FALSE;
	}

	mssgSize = sprintf(mssgBuffer, "%s %s %d %s\n", REQ_AUT, 
	userInfo->uid, *(rid), vc);
	printf("%d\n", mssgSize);
	
	if (mssgSize < 0)
		WARN("Failed to write val message to buffer.");
	
	_LOG("Le buffer %s", mssgBuffer);
	
	/* If rid were NULL then we wouldn't have reached here... 
	so be free to free! :) */
	free(rid);
	
	return TRUE;
}


bool_t req_list() {
	/* User establishes TCP session with FS
	asking for the list of files this user has previously
uploaded to the server. 
The message includes the UID, the TID and the type of file operation desired (Fop). 
The reply should be displayed as a numbered list of filenames and the respective sizes.*/
	
	// Establish TCP connection with FS

	// Send message to FS: LST UID TID 


	return TRUE;
}


bool_t req_retrieve() {
	/* User application establishes a TCP session with the FS server, 
	to retrieve the selected file filename. 
	The message includes the UID, the TID, the Fop and
Fname. 
The confirmation of successful transmission (or not) should be
displayed */
/*Following the retrieve command, the User application opens a TCP
connection with the FS server to retrieve the contents of the file with name
Fname from the FS server. The user ID (UID) and transaction ID (TID) are
also provided. Before replying, the FS sends a message to the AS to validate the
transaction (VLD).*/
/*) RTV UID TID Fname*/
	return TRUE;
}


bool_t req_upload() {
	/*  User application establishes a TCP session with the FS server, 
	to upload the file filename. The message includes the UID, the TID, the 
	Fop, Fname and the file size. 
The confirmation of successful transmission (or not) should be displayed.*/
/*UPL UID TID Fname Fsize data
Following the upload command, the User application opens a TCP connection
with the FS server and uploads to it the contents of the selected file (data),
with name Fname and size Fsize bytes. The user ID (UID) and transaction ID
(TID) are also provided. Before replying, the FS sends a message to the AS to
validate the transaction (VLD).*/

	// Open TCP connection with FS

	// Send to FS: UPL UID TID Fname Fsize data


	return TRUE;
}


bool_t req_delete() {
	/*following this command the User application establishes a TCP session 
	with the FS server, to delete the file
	filename. The message includes the UID, the TID, the Fop and Fname.
	The confirmation of successful deletion (or not) should be displayed.*/
	/* DEL UID TID Fname
Following the delete command, the User application opens a TCP connection
with the FS server and requests the deletion of the file with name Fname. The
user ID (UID) and transaction ID (TID) are also provided. Before replying, the
FS sends a message to the AS to validate the transaction (VLD).*/

	// Open TCP connection with FS.
	
	// Send to FS: DEL UID TID Fname

	return TRUE;
}


bool_t req_remove() {
	/*this command is used to request the FS to remove all files and
directories of this User, as well as to request the FS to instruct the AS to delete 
22/10/2020
this user’s login information. The result of the command should be displayed to
the user. The User application then closes all TCP connections and terminates*/

/*REM UID TID
Following the remove command, the User application opens a TCP connection
with the FS server and requests the removal of all its files and directories from
the FS server, as well as the deletion of the user information from the AS server.
The user ID (UID) and transaction ID (TID) are also provided. Before replying,
the FS sends a message to the AS to validate the transaction (VLD) and
requesting the AS to remove the user information.*/

	// Open TCP connection with FS

	// Send to FS: REM UID TID


	return TRUE;
}


bool_t req_resendLastMessage() {
	LOG("Resending last user message...");
	return TRUE;
}


bool_t resp_login(char *status) {
/*RLO status
In reply to a LOG request the AS server replies with the status of the login
request. 
If UID and pass are valid the status is OK; 
if the UID exists but the pass is incorrect the status is NOK; 
otherwise the status is ERR.*/

	if (!strcmp(status,STATUS_OK)) 
		printf("Login successeful. Congrats lad.\n");
	else if (!strcmp(status, STATUS_NOK))
		printf("Incorrect password.\n");
	else if (!strcmp(status, SERVER_ERR))
		printf("Smth went wrong.\n");
	return TRUE;
}


bool_t resp_request(char *status) {
/*RRQ status
The AS server replies informing if the REQ request could be processed (valid
UID), a message was sent to the PD and a successful RVC confirmation received.
In case of success the status is OK; 
if the REQ request was sent in a TCP connection where a successful login was not previously done,
 the status is ELOG; 
 if a message could not be sent by the AS to the PD the status is EPD;
if the UID is incorrect the status is EUSER; 
if the Fop is invalid the status is EFOP; 
otherwise (e.g. incorrectly formatted REQ message) the status is ERR.*/

		if (!strcmp(status, STATUS_OK))
			printf("Request successefully made, you smart ass.\n");
		else if (!strcmp(status, STATUS_ELOG))
			printf("A successeful login hasn't been made before. \
			Please login successefully before making requests.");
		else if (!strcmp(status, STATUS_EPD))
			printf("Authentication server (AS) failed to contact your \
			personal device (PD)\n");
		else if (!strcmp(status, STATUS_EUSER))
			printf("Incorrect UID, moron.\n");
		else if (!strcmp(status, STATUS_EFOP))
			printf("Invalid file operation, dumbass\n.");
		else if (!strcmp(status, SERVER_ERR))
			printf("Incorrectly formatted request, dear.\n");
	

	// DO NOT close connection to AS
	return TRUE;
}


bool_t resp_val(char *tid) {
/*In reply the AS should confirm (or not) the success of the two-factor authentication, 
	which should be displayed. The AS also sends the transaction ID TID.*/
	/*RAU TID
The AS confirms (or not) the success of the two-factor authentication, sending
the transaction identifier TID to use in the file operation with the FS. The TID
takes value 0 if the authentication failed.*/

	//
	/*
	if TID = 0
		printf Authentication near Authentication Server (AS) failed.
		return FALSE
	else
		printf Authentication successeful. The TID for the request you 
		asked for is %d, TID
		return TRUE
	*/
}


bool_t resp_list() {
	/* 
After receiving a message from the AS validating the transaction (CNF), the FS
reply to a User application LST request contains the number N of available files,
and for each file:
• the filename Fname, limited to a total of 24 alphanumerical characters (plus
‘-‘, ‘_’ and ‘.’), including the separating dot and the 3-letter extension:
“nnn…nnnn.xxx”;
• the file size Fsize, in bytes.
The filenames should be displayed by the User application as a numbered list.
In case of error the reply is in the form RLS status, with 
RLS N[ Fname Fsize]
RLS status:
 - status = EOF if no files are available, 
 - status = INV in case of an AS validation error of the provided TID,
 - status = ERR if the LST request is not correctly formulated*/
	//Close TCP connection with FS.

	/*
		if is integer (status)
			printf name \t\t\t size		like a table header?
			for i from 1 to status
				printf name size
		else if status = EOF
			printf No files are available
		else if status = INV
			printf Failed to validate TID near AS
		else if status = ERR
			printf LST request wrongly formulated.

		// Close TCP connection with FS


	*/
}


bool_t resp_retrieve() {
/*RRT status [Fsize data]
After receiving a message from the AS validating the transaction (CNF), and in
reply to a RTV request, the FS server transfers to the User application the
contents (data) of the selected file, as well as the file size Fsize in bytes. 
If the RTV request was successful the status is OK, 
the status is EOF if the file is not available, 
the status is NOK if there is no content available in the FS
for the user with ID UID, 
the status is INV in case of an AS validation error
of the provided TID, and 
the status is ERR if the RTV request is not correctly
formulated.
The name and path where the file is stored are displayed by the User application.
After receiving the reply message, the User application closes the TCP
connection with the FS.*/

/*
	if status = OK
		printf Retrieve request successeful
		printf filename, path
		printf fsize, data
	else if status = EOF
		printf File not available
	else if status = NOK
		printf here is no content available in the FS
for the user with UID %d, UID
	else if status = INV
		printf AS failed to validate the retrieve request 
		[maybe present TID?]
	else if status = ERR
		printf Retreive command wrongly formulated

	// Close down FS TCP connection.

*/
	return TRUE;
}


bool_t resp_upload(int fsSockfd, char *status) {
/* RUP status
After receiving a message from the AS (CNF) validating the transaction, the
answer to a UPL request consists in the FS server replying with the status of the
file transfer. 
If the UPL request was successful the status is OK, the status
is NOK if the UID does not exist, the status is DUP if the file already existed,
the status is FULL if 15 files were previously uploaded by this User, the
status is INV in case of an AS validation error of the provided TID, and the
status is ERR if the UPL request is not correctly formulated.
The upload success (or not) is displayed by the User application.
After receiving the reply message, the User application closes the TCP
connection with the FS.*/

	if (!strcmp(status, STATUS_OK))
		printf ("Upload successeful.");
	else if (!strcmp(status, STATUS_NOK))
		printf ("File dne");
	else if (!strcmp(status, STATUS_DUP))
		printf ("The file already existed.");
	else if (!strcmp(status, STATUS_FULL))
		printf("You have already uploaded 15 files");
	else if (!strcmp(status, STATUS_INV))
		printf("Failed authentication near AS");
	else if (!strcmp(status, SERVER_ERR))
		printf ("Request wrongly formulated.");
	
	// Close FS TCP connection
	tcpDestroySocket(fsSockfd);
	return TRUE;
}


bool_t resp_delete(int fsSockfd, char *status) {
/*RDL status
After receiving a message from the AS (CNF) validating the transaction, the
answer to a DEL request consists in the FS server replying with the status of the
file deletion. 
If the DEL request was successful the status is OK, 
the status is EOF if the file is not available, 
the status is NOK if the UID does not exist,
the status is INV in case of an AS validation error of the provided TID, and
the status is ERR if the DEL request is not correctly formulated.
The delete success (or not) is displayed by the User application.
After receiving the reply message, the User application closes the TCP
connection with the FS.*/

/* This function and the remove one are one and the same. 
Does it make sense to join them? */

	if (!strcmp(status, STATUS_OK))
		printf("Successefully deleted le file.");
	else if (!strcmp(status, STATUS_NOK))
		printf("UID dne");
	else if (!strcmp(status, STATUS_INV))
		printf("AS failed to validate le TID");
	else if (!strcmp(status, SERVER_ERR))
		WARN("Delete request wrongly formulated");

	// Close FS TCP connection
	tcpDestroySocket(fsSockfd);
	return TRUE;
}


bool_t resp_remove(int fsSockfd, char *status) {
/*After receiving a message from the AS (CNF) validating the transaction and
confirming the user deletion in the AS, the FS removes all the user’s files and
directories. 
It then replies with the status of the operation: 
the status is OK if the REM request was successful, 
the status is NOK if the UID does not exist,
the status is INV in case of an AS validation error of the provided TID, and
the status is ERR if the REM request is not correctly formulated.
The remove success (or not) is displayed by the User application.
After receiving the reply message, the User application closes the TCP
connection with the FS.*/

	if (!strcmp(status, STATUS_OK))
		printf("Remotion successeful! You're free!! :D");
	else if (!strcmp(status, STATUS_NOK))
		printf("UID dne");
	else if (!strcmp(status, STATUS_INV))
		printf("The Authentication Server failed to validate the TID");
	else if (!strcmp(status, SERVER_ERR))
		WARN("Remove request wrongly formulated...");
	
	// Close FS connection
	tcpDestroySocket(fsSockfd);
	return TRUE;
}