#include "common.h"
#include "udp.h"
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>


/* the information to allow communication with the autentication server */
typedef struct connectionInfo_t {

        char pdip[IP_SIZE + 1];                 /* ip address of the program */
        char pdport[PORT_SIZE + 1];             /* port of the program */        
        char asip[IP_SIZE + 1];                 /* ip address of the autentication server */
        char asport[PORT_SIZE + 1];             /* port of the autentication server */

} connectionInfo_t;


void displayConnectionInfo(connectionInfo_t *info) {
	printf("serverInfo settings:\nPDIP\t: %s\nPDport\t: %s\nASIP\t: %s\nASport\t: %s\n", 
                info->pdip, info->pdport, info->asip, info->asport);
}



/* the user's information */
typedef struct user_info_t {

        char uid[BUFFERSIZE];
        char pass[BUFFERSIZE];
        char sessionFlag;

} userInfo_t;

/* ======== CONSTANTS ========= */
#define REGCMD  "reg"
#define EXITCMD "exit"
#define VALIDCODE "VC"


/* ======== GLOBAL VARS ========= */
userInfo_t userInfo;
connectionInfo_t connectionInfo = {"", "57053\0", "193.136.138.142\0", "58011\0"};
char msgSentFlag = FALSE; //trace back response from server

void setDirty() { msgSentFlag = TRUE; }
void setClean() { msgSentFlag = FALSE; }
char isDirty() { return msgSentFlag; }


/*! \brief Parses the execution arguments.
 *
 *  Validates the execution arguments and sets the connection settings.
 *
 * \param argc The number of execution arguments.
 * \param argv An array with the execution arguments.
 * \param info The instance that stores the connection settings.
 * \return NULL.
 */
void parseArgs(int argc, char *argv[], connectionInfo_t *info) {
	// check the number of arguments
	if (argc < 2 || argc > 8 || argc % 2 != 0)
		_FATAL("Invalid number of arguments!\nUsage: %s PDIP [-d PDport] [-n ASIP] [-p ASport]", argv[0]);

	// override default connection settings
	if (!checkValidIp((const char*) argv[1])) 
		fatal("Invalid IP address!\nUsage: xxx.xxx.xxx.xxx");

	strncpy(info->pdip, argv[1], IP_SIZE);

	for (int i = 2; i < argc; i++){
		if (!strcmp(PDPORTARG, argv[i]) && checkValidPORT((const char *)argv[i+1])) 
			strncpy(info->pdport, argv[++i], PORT_SIZE);
		else if (!strcmp(ASIPARG, argv[i]) && checkValidIp((const char*) argv[i+1]))
			strncpy(info->asip, argv[++i], IP_SIZE);
		else if (!strcmp(ASPORTARG, argv[i]) && checkValidPORT((const char *)argv[i+1]))
			strncpy(info->asport, argv[++i], PORT_SIZE); 
		else 
			fatal("Invalid IP address formart.\nPlease use dot notation.\nOr invalid PORT format. \nPlease only use unrestricted ports.");
	}

	/* logs the server information (on debug mod only) */
	_LOG("serverInfo settings:\nPDIP\t: %s\nPDport\t: %s\nASIP\t: %s\nASport\t: %s\n", 
			info->pdip, info->pdport, info->asip, info->asport);
}


 /*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  userInfo a pointer to store the user's info.
 * \return NULL.
 */
void registerUser(int fd, const char *uid, const char *pass) {
        char cmdAS[BUFFERSIZE];
        int size, retSize;
        
        if (userInfo.sessionFlag) {
                warning("Session already on.\nTo register another user please unregister first.");
                return;
        }

        // Update User info
        strcpy(userInfo.uid, uid);
        strcpy(userInfo.pass, pass);
        userInfo.sessionFlag = FALSE;
        
        // REG UID pass PDIP PDport
        size = sprintf(cmdAS, "%s %s %s %s %s%c", REG_REQ, userInfo.uid, userInfo.pass, connectionInfo.pdip,\
                        connectionInfo.pdport, ENDMSG);
        
        retSize = udpSendMessage(fd, cmdAS, size);
        
        if (size != retSize) {
                warning("Failed to send registration message.");
                return;
        }
        // Only update msg sent status if message successfuly sent
        setDirty();
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
void unregisterUser(int fd, char* msgBuf) {
	/* verify what comes after - nothing
	 * send to AS: UNR UID pass
	 * receive RUN status
	 */
        int size, retSize;

        // If no user info confirmed
        if (!userInfo.sessionFlag) {
                warning("No session currently log in.");
                return;
        }
        
        // Sending Unregister message "UNR UID pass"
        size = sprintf(msgBuf, "%s %s %s%c", UNREG_REQ, userInfo.uid, userInfo.pass, ENDMSG);
        
        retSize = udpSendMessage(fd, msgBuf, size);
        
        if (size != retSize) {
                warning("Failed to send unregister message.");
                return;
        }
        // Only update msg sent status if message successfuly sent
        setDirty();
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
int handleUser(int sockfd, char* buf) {
	int n, size;
	char command[BUFFERSIZE], uid[BUFFERSIZE], pass[BUFFERSIZE];

	/* Read user input - Check for error */
        // fgets(buf, BUFFERSIZE, stdin);		/* fgets returns NULL on error or EOF? */
        getUserInput(buf);

        sscanf(buf, "%s %s %s", command, uid, pass);

        /* logs the server information (on debug mod only) */
	_LOG("handleUser input:\nBUFFER\t: %s\nCMD\t: %s\nUID\t: %s\nPASS\t: %s\n", 
			buf, command, uid, pass);

	/* Check if command is valid: reg, exit */
        if (!strcmp(command, REGCMD))
                registerUser(sockfd, uid, pass);
        else if (!strcmp(command, EXITCMD))
                unregisterUser(sockfd, buf);
        else
                warning("Not a valid command.");   
}

/* ========== SERVER HANDLING ============= */

void registerUser_Response(char *status) {
        if(!strcmp(status, STATUS_OK)){
                puts("Registration successful.");
                userInfo.sessionFlag = TRUE;
        }
        else if(!strcmp(status, STATUS_NOK))
                puts("Registration unsuccessful.");
}

void sendValidCode_response(int sockfd, char *msgBuf, const char *status) {
        int size, retSize;
        size = sprintf(msgBuf, "%s %s %c", VALIDCODE_RESP, STATUS_OK, ENDMSG);
        retSize = udpSendMessage(sockfd, msgBuf, size);
        
        if (size != retSize) warning("Failed to send confirmation reply.");
}

void validationCode_response(int sockfd, char *msgBuf, char *reponse) {
        // Parse the rest of the response received from the server
        char uid[BUFFERSIZE], vc[BUFFERSIZE], fname[BUFFERSIZE];
        char fop;
        *fname = '\0';
        sscanf(reponse, "%s %s %c %s", uid, vc, &fop, fname);
        if(!strcmp(uid, userInfo.uid)){
                // Operations over files (with filename)
                if((fop == FOP_R || fop == FOP_U || fop == FOP_D)
                        && *fname != '\0')
                        printf("%s=%s, %s: %s", VALIDCODE, vc, getFileOp(fop), fname);
                else if(fop == FOP_L || fop == FOP_X)
                        printf("%s=%s, %s", VALIDCODE, vc, getFileOp(fop));
                
                // Respond to server OK
                sendValidCode_response(sockfd, msgBuf, STATUS_OK);
        } else 
                // Respond to server NOT OK
                sendValidCode_response(sockfd, msgBuf, STATUS_NOK);

}

void unregisterUser_Response(char *status) {
        if(!strcmp(status, STATUS_OK)){
                puts("Unregistration was successful.");
                userInfo.sessionFlag = FALSE;
        }
        else if(!strcmp(status, STATUS_NOK))
                puts("Unregistration was unsuccessful.");
}
/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
void handleServer(int sockfd, char* msgBuf){
	/* PD will be server to receive VC codes from client AS */
	int n;
	char respHead[BUFFERSIZE]; char respEnd[BUFFERSIZE];
	
        n = udpReceiveMessage(sockfd, msgBuf, BUFFERSIZE);
        setClean();

        sscanf(msgBuf, "%s %s", respHead, respEnd);

        // Registration Response "RRG"
        if (!strcmp(respHead, REG_RESP))
                registerUser_Response(respEnd);
        // Validation Code received "VLC"
        else if (!strcmp(respHead, VALIDCODE_REQ))
                validationCode_response(sockfd, msgBuf, respEnd);
        else if (!strcmp(respHead, UNREG_RESP))
                unregisterUser_Response(respEnd);                
        else
                warning("Server Error.");

        // fwrite(buf, n, sizeof(*buf), stdout);
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
int handleNoResponse(int sockfd, char* msgBuf) {
	return udpSendMessage(sockfd, (const char*) msgBuf, BUFFERSIZE);
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \return Return parameter description
 */
void waitEvent(int fd) {
	fd_set fds, ready_fds;
        struct timeval tv, tmp_tv;
        int selectRet, fds_size, retVal;
	char buffer[2*BUFFERSIZE];	// prevent overflows, giving space to concatenate msgs

	/* SELECT */
	FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(fd, &fds);
        fds_size = fd+1;
	tv.tv_sec = 15;
	tv.tv_usec = 0;

        display(INPUTCHAR);
        
	while (TRUE) {
		// because select is destructive
		ready_fds = fds;
                tmp_tv = tv;

		selectRet = select(fds_size, &ready_fds, NULL, NULL, &tmp_tv);

		if (selectRet == -1)
			fatal("Failed System Call Select");
		if (FD_ISSET(fd, &ready_fds)){	// give prority to server responses
			// handle fd interaction
                        putchar(DELCHAR);          // clear PREVIOUS INPUTCHAR
                        display(RESPONSECHAR);
			handleServer(fd, buffer);
                        display(INPUTCHAR);
                }
		if (FD_ISSET(STDIN_FILENO, &ready_fds)){
			// handle stdin
			retVal = handleUser(fd, buffer);
                        display(INPUTCHAR);
                }
		if (selectRet == 0 && isDirty()) // timeout expired
			// act as previous message didn't reach the target
			retVal = handleNoResponse(fd, buffer);
                
	}
}


int main(int argc, char *argv[]) {
	//connectionInfo_t connectionInfo = {"", "57053\0", "127.0.0.1\0", "58053\0"};

	int asSockfd;
	parseArgs(argc, argv, &connectionInfo);
        userInfo.sessionFlag = FALSE;

	/* Socket to contact with AS. */
	asSockfd = udpCreateClient(connectionInfo.asip, connectionInfo.asport);

	waitEvent(asSockfd);

	udpShutdownSocket(asSockfd);
        
	
        /* first command should be reg (what if it's not?) DISPLAY ERR */
        
        /*  establish UDP client connection with AS server
                sends register message REG to AS
                receives RRG from AS
                while command is not exit
                        waits for VLCs from AS and displays them
                        sends RVC to AS server
                unregister user: send UNR to AS
                receives RUN response from AS
                shut down UDP client connection with AS server
        */  
        /* 
        
        what if exit comes before reg?
        */
	

        return 0;
}
