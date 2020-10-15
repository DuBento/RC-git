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

        char uid[BUFSIZ];
        char pass[BUFSIZ];

} userInfo_t;

/* ======== CONSTANTS ========= */
#define REGCMD  "reg"
#define EXITCMD "exit"


/* ======== GLOBAL VARS ========= */
userInfo_t userInfo;
connectionInfo_t connectionInfo = {"", "57053\0", "193.136.138.142\0", "58011\0"};



void displayShell() {
        printf(">");                      //waiting for command visual indicator
        fflush(stdout);
}


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
		FATAL("Invalid IP address!\nUsage: xxx.xxx.xxx.xxx");

	strncpy(info->pdip, argv[1], IP_SIZE);
	printf("%s\n", info->pdip);
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
        char cmdAS[BUFSIZ];
        int size, retSize;
        
        // Update User info
        strcpy(userInfo.uid, uid);
        strcpy(userInfo.pass, pass);
        
        // REG UID pass PDIP PDport
        size = sprintf(cmdAS, "REG %s %s %s %s%c", userInfo.uid, userInfo.pass, connectionInfo.pdip,\
                        connectionInfo.pdport, ENDMSG);
        
        retSize = udpSendMessage(fd, cmdAS, size);
        
        if (size != retSize) fatal("Failed to send registration message.");
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
void unregisterUser(int fd) {
	/* verify what comes after - nothing
	 * send to AS: UNR UID pass
	 * receive RUN status
	 */
        char cmdAS[BUFSIZ];
        int size, retSize;
        
        // UNR UID pass
        size = sprintf(cmdAS, "UNR %s %s%c", userInfo.uid, userInfo.pass, ENDMSG);
        
        retSize = udpSendMessage(fd, cmdAS, size);
        
        if (size != retSize) fatal("Failed to send registration message.");
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
int handleUser(int sockfd, char* buf, short *flag) {
	
	int n, size;
	char command[CMD_SIZE], uid[UID_SIZE], pass[PASS_SIZE];

	/* Read user input - Check for error */
        // fgets(buf, BUFSIZ, stdin);		/* fgets returns NULL on error or EOF? */
        getUserInput(buf);

        sscanf(buf, "%s %s %s", command, uid, pass);
        
	/* Check if command is valid: reg, exit */
        if (!strcmp(command, REGCMD)) {
                registerUser(sockfd, uid, pass);
        } else if (!strcmp(command, EXITCMD)) {
                unregisterUser(sockfd);       // userInfo dinamically stored
        } else {
                puts("Not a valid command.");
        }
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
int handleServer(int sockfd, char* buf, short *flag){
	/* PD will be server to receive VC codes from client AS */
	int n;

	n = udpReceiveMessage(sockfd, buf, BUFSIZ);
	*flag = FALSE;
        printf("\nserver: %s", buf);
	return n;
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */
int handleNoResponse(int sockfd, char* buf) {
	return udpSendMessage(sockfd, (const char*) buf, BUFSIZ);
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
	short msgSent=0;	//trace back response from server
	char buffer[BUFSIZ];

	/* SELECT */
	FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(fd, &fds);
        fds_size = fd+1;
	tv.tv_sec = 15;
	tv.tv_usec = 0;

        displayShell();
        
	while (TRUE) {
		// because select is destructive
		ready_fds = fds;
                tmp_tv = tv;

		selectRet = select(fds_size, &ready_fds, NULL, NULL, &tmp_tv);

		if (selectRet == -1)
			fatal("Failed System Call Select");
		if (FD_ISSET(fd, &ready_fds)){	// give prority to server responses
			// handle fd interaction
			retVal = handleServer(fd, buffer, &msgSent);
                        displayShell();
                }
		if (FD_ISSET(STDIN_FILENO, &ready_fds)){
			// handle stdin
			retVal = handleUser(fd, buffer, &msgSent);
                        displayShell();
                }
		if (selectRet == 0 && msgSent == TRUE) // timeout expired
			// act as previous message didn't reach the target
			retVal = handleNoResponse(fd, buffer);
	}
}


int main(int argc, char *argv[]) {
	//connectionInfo_t connectionInfo = {"", "57053\0", "127.0.0.1\0", "58053\0"};

	int asSockfd;
	parseArgs(argc, argv, &connectionInfo);
	displayConnectionInfo(&connectionInfo);


	/* Socket to contact with AS. */
	asSockfd = udpCreateClient(connectionInfo.asip, connectionInfo.asport);

	waitEvent(asSockfd);

	udpShutdownSocket(asSockfd);
        
	

        /*if (fgets(buffer, BUFSIZ, stdin) == NULL)
                fatal("Failed to read user input");

        if ( !strncmp(token, REGCMD, REGCMDLEN) )
                regCmd(buffer + REGCMDLEN, &userInfo);
        else if ( !strcmp(token, EXITCMD, EXITCMDLEN) )
                exitCmd();
        else
                */
           /* udp send to(ASfd, "ERR");*/


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
