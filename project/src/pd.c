#include "common.h"
#include "udp.h"
#include <stdio.h>
#include <sys/select.h>


#define PORTARG "-d"                /* console argument to specify PDport */
#define ASIPARG "-n"                /* console argument to specify ASIP */
#define ASPORTARG "-p"          /* console argument to specify ASport */


/* the information to allow communication with the autentication server */
typedef struct connectionInfo_t {

        char pdip[IP_SIZE + 1];                 /* ip address of the program */
        char pdport[PORT_SIZE + 1];         /* port of the program */        
        char asip[IP_SIZE + 1];                 /* ip address of the autentication server */
        char asport[PORT_SIZE + 1];         /* port of the autentication server */

} connectionInfo_t;


/* the user's information */
typedef struct user_info_t {

        char uid[UID_SIZE + 1];
        char pass[PASS_SIZE + 1];

} userInfo_t;



/** \brief Parses the execution arguments
 * 
 * 	Validates the execution arguments and sets the connection settings
 * 
 * 	\param 	argc
 *                  the number of execution arguments
* 	\param 	argv
 *                  an array with the execution arguments
 *  \param 	info
 *                  the instance that stores the connection settings
 *  \return NULL
 */
void parseArgs(int argc, char *argv[], connectionInfo_t *info) {
        /* check the number of arguments */        
	if (argc < 2 || argc > 8 || argc % 2 != 0){
		printf("Usage: %s PDIP [-d PDport] [-n ASIP] [-p ASport]", argv[0]);
		fatal("Failed to parse arguments");
	}

        /* override default connection settings */
        strncpy(info->pdip, argv[1], IP_SIZE);                                                                                                                                                          /*[IF THE IP HAS MORE THAN 15 CHARS IGNORE OR ERROR???]*/
        for (int i = 2; i < argc; i++){
                if (!strcmp(PORTARG, argv[i])) 
                        strncpy(info->pdport, argv[++i], PORT_SIZE);                                                                                                                                /*[IF THE IP HAS MORE THAN 6 CHARS IGNORE OR ERROR???]*/
                else if (!strcmp(ASIPARG, argv[i]) )
                        strncpy(info->asip, argv[++i], IP_SIZE);                                                                                                                                        /*[IF THE IP HAS MORE THAN 15 CHARS IGNORE OR ERROR???]*/
                else if (!strcmp(ASPORTARG, argv[i]) )
                        strncpy(info->asport, argv[++i], PORT_SIZE);                                                                                                                                /*[IF THE IP HAS MORE THAN 6 CHARS IGNORE OR ERROR???]*/
        }

        /* logs the server information (on debug mod only) */
        _LOG("serverInfo settings:\nPDIP\t: %s\nPDport\t: %s\nASIP\t: %s\nASport\t: %s\n", 
                info->pdip, info->pdport, info->asip, info->asport);
}



/** \brief Registers a user.
 * 
 * 	Waits for user input so that they can register themselves.
 * 
 * 	\param 	userInfo
 *                  a pointer to store the user's info.
 *  \return NULL
 */
void regCmd(const char *buffer, userInfo_t *userInfo) {
        char errCheck = '\0';
        sscanf(buffer, "%5s %s %c", userInfo->uid, userInfo->pass, &errCheck);
        if (errCheck != '\0')
                fatal("Invalid arguments on the reg command!");
}


void unregister() {
        /* sends UNR UID pass to AS
                receives RUN status*/
}



int main(int argc, char *argv[]) {
        connectionInfo_t connectionInfo = {"", "57053\0", "localhost\0", "58053\0"};
        // userInfo_t userInfo = {0};
        fd_set rfds;
        struct timeval tv;
        int selectRet;
        int sockfd;

        parseArgs(argc, argv, &connectionInfo);
	sockfd = udpCreateSocket("127.0.0.1", "5000");

        /* SELECT */
	FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(sockfd, &rfds);
        tv.tv_sec = 15;
	tv.tv_usec = 0;
	selectRet = select(2, &rfds, NULL, NULL, &tv);

	if (selectRet == -1)
		fatal("Failed System Call Select");

	printf("%d", selectRet);

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
