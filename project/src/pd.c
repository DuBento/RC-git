#include "common.h"
#include "udp.h"
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>


/* the information to allow communication with the autentication server */
typedef struct connectionInfo_t {

        char pdip[IP_SIZE + 1];                 /* ip address of the program */
        char pdport[PORT_SIZE + 1];         /* port of the program */        
        char asip[IP_SIZE + 1];                 /* ip address of the autentication server */
        char asport[PORT_SIZE + 1];         /* port of the autentication server */

} connectionInfo_t;


void displayConnectionInfo(connectionInfo_t *info) {
	printf("serverInfo settings:\nPDIP\t: %s\nPDport\t: %s\nASIP\t: %s\nASport\t: %s\n", 
                info->pdip, info->pdport, info->asip, info->asport);
}



/* the user's information */
typedef struct user_info_t {

        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

} userInfo_t;

/* GLOBAL VARS */


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
        /* check the number of arguments */        
	if (argc < 2 || argc > 8 || argc % 2 != 0){
		printf("Usage: %s PDIP [-d PDport] [-n ASIP] [-p ASport]\n", argv[0]);
		fatal("Failed to parse arguments");
	}

        /* override default connection settings */
	if (!checkValidIp((const char*) argv[1])) 
                fatal("Invalid IP address formart.\nPlease use dot notation.");
        // else
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
void regCmd(const char *buffer, userInfo_t *userInfo) {
        char errCheck = '\0';
        sscanf(buffer, "%5s %s %c", userInfo->uid, userInfo->pass, &errCheck);
        if (errCheck != '\0')
                fatal("Invalid arguments on the reg command!");
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
void registerUser(){
	/* verify correct id and pass
	 * send to AS: REG UID pass PDIP PDport
	 * receive response RRG status from AS
	 * wait to establish AS client connection socket here?
	 */
}


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */
void unregisterUser() {
	/* verify what comes after - nothing
	 * send to AS: UNR UID pass
	 * receive RUN status
	 */
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

        //sscanf(buf, "%4s %5s %8s", command, uid, pass);
        
        /*if (strcmp(command, REGCMD) == 0) {
                regCmd(uid, pass);
        } else if (!strcmp(command, EXITCMD)) {
                unregisterUser(userInfo);       // userInfo dinamically stored
        } else {

        }*/
	/* Check if command is valid: reg, exit */
	/* 
         * if command reg strcmp(token, REGCMD)  
	 * 	registerUser()
	 * else if command exit
	 * 	exit()
	 * else
	 * 	not allowed, you dumbass
	 */

	_LOG("Message to be sent: %s", buf);
        size = strlen(buf);

	n = udpSendMessage(sockfd, (const char*) buf, size);
	*flag = TRUE;
	return n;
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
        fprintf(stdout, "server: %s", buf);
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

	while (TRUE) {
		// because select is destructive
		ready_fds = fds;
                tmp_tv = tv;

		selectRet = select(fds_size, &ready_fds, NULL, NULL, &tmp_tv);

		if (selectRet == -1)
			fatal("Failed System Call Select");
		if (FD_ISSET(fd, &ready_fds))	// give prority to server responses
			// handle fd interaction
			retVal = handleServer(fd, buffer, &msgSent);
		if (FD_ISSET(STDIN_FILENO, &ready_fds))
			// handle stdin
			retVal = handleUser(fd, buffer, &msgSent);
		if (selectRet == 0 && msgSent == TRUE) // timeout expired
			// act as previous message didn't reach the target
			retVal = handleNoResponse(fd, buffer);
	}
}


int main(int argc, char *argv[]) {
        // connectionInfo_t connectionInfo = {"", "57053\0", "127.0.0.1\0", "58053\0"};
        connectionInfo_t connectionInfo = {"", "57053\0", "193.136.138.142\0", "58011\0"};
//        userInfo_t userInfo = {0};
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
