#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "common.h"
#include "udp.h"
#include "pd_api.h"

/* ======== CONSTANTS ========= */
#define REGCMD  "reg"
#define EXITCMD "exit"


/* ======== GLOBAL VARS ========= */
userInfo_t userInfo;
connectionInfo_t connectionInfo = {"", "57053\0", "193.136.138.142\0", "58011\0"};
// connectionInfo_t connectionInfo = {"", "57053\0", "127.0.0.1\0", "58053\0"};
int asSockfd;
char msgBuffer[2*BUFFERSIZE];	// prevent overflows, giving space to concatenate msgs


/* ======== METHODS ========= */

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
	if (argc < 2 || argc > 8 || argc % 2 != 0){
		printf("Usage: %s PDIP [-d PDport] [-n ASIP] [-p ASport]\n", argv[0]);
		fatal("Invalid number of arguments!");
        }

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
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
void handleUser(int sockfd, char* buf, connectionInfo_t connectionInfo, userInfo_t *userInfo) {
	int n, size;
	char command[BUFFERSIZE], uid[BUFFERSIZE], pass[BUFFERSIZE];
        uid[0] = '\0'; pass[0] = '\0';          // prevent _LOG from priting stack trash
	
        /* Read user input - Check for error */
        // fgets(buf, BUFFERSIZE, stdin);		/* fgets returns NULL on error or EOF? */
        getUserInput(buf);

        sscanf(buf, "%s %s %s", command, uid, pass);

        /* logs the server information (on debug mod only) */
	_LOG("handleUser input:\nBUFFER\t: %s\nCMD\t: %s\nUID\t: %s\nPASS\t: %s\n", 
			buf, command, uid, pass);

	/* Check if command is valid: reg, exit */
        if (!strcmp(command, REGCMD))
                registerUser(sockfd, uid, pass, connectionInfo, userInfo);
        else if (!strcmp(command, EXITCMD))
                unregisterUser(sockfd, buf, userInfo);
        else
                warning("Not a valid command.");   
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
void handleServer(int sockfd, char* msgBuf, userInfo_t *userInfo){
	/* PD will receive commands from server AS */
	int n;
	char respHead[BUFFERSIZE]; char respEnd[BUFFERSIZE];
	
        n = udpReceiveMessage(sockfd, msgBuf, BUFFERSIZE);
        setClean();

        sscanf(msgBuf, "%s %s", respHead, respEnd);

        // Registration Response "RRG"
        if (!strcmp(respHead, REG_RESP))
                registerUser_Response(respEnd, userInfo);
        // Validation Code received "VLC"
        else if (!strcmp(respHead, VALIDCODE_REQ))
                validationCode_Response(sockfd, msgBuf, respEnd, userInfo);
        else if (!strcmp(respHead, UNREG_RESP))
                unregisterUser_Response(respEnd, userInfo);                
        else
                warning("Server Error.");
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
void handleNoResponse(int sockfd, char* msgBuf) {
	udpSendMessage(sockfd, (const char*) msgBuf, BUFFERSIZE);
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \return Return parameter description
 */
void waitEvent(int fd, char *buffer, connectionInfo_t connectionInfo, userInfo_t *userInfo) {
	fd_set fds, ready_fds;
        struct timeval tv, tmp_tv;
        int selectRet, fds_size;
        int nTry = 0;

	/* SELECT */
	FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(fd, &fds);
        fds_size = fd+1;
	tv.tv_sec = TIMER_SEC;
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
			handleServer(fd, buffer, userInfo);
                        display(INPUTCHAR);
                }
		if (FD_ISSET(STDIN_FILENO, &ready_fds)){
			// handle stdin
			handleUser(fd, buffer, connectionInfo, userInfo);
                        display(INPUTCHAR);
                }
		if (selectRet == 0 && isDirty()) {// timeout expired
			// act as previous message didn't reach the target
                        // try to resend NTRIES_NORESP times
			if (nTry < NTRIES_NORESP)
                                handleNoResponse(fd, buffer);
                        else{
                                nTry = 0;
                                warning("No response received from sent message.\nCommunication error.");
                        }
                }       
	}
}

void exitPD() {
        // unregisterUser(asSockfd, msgBuffer);    // TODO confirm if we can do this (also im not confirming )
	udpDestroySocket(asSockfd);
        exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
        initSignal(&exitPD);
	parseArgs(argc, argv, &connectionInfo);
        userInfo.sessionFlag = FALSE;

	/* Socket to contact with AS. */
	asSockfd = udpCreateClient(connectionInfo.asip, connectionInfo.asport);

	waitEvent(asSockfd, msgBuffer, connectionInfo, &userInfo);

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
	

        return 0; // Never used
}
