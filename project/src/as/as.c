#include "../common.h"
#include "../udp.h"
#include "../tcp.h"
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

typedef struct connectionInfo_t {

        char asport[PORT_SIZE + 1];             /* port of the autentication server */

} connectionInfo_t;

/* ========== GLOBAL ============= */
int udpServerfd, tcpServerfd;
char msgBuffer[2*BUFFER_SIZE];	// prevent overflows, giving space to concatenate msgs
char verbosity = FALSE;

#define CHECK_VERBOSITY { return verbosity }

void parseArgs(int argc, char *argv[], connectionInfo_t *info) {
        /* check the number of arguments */        
	if (argc < 1 || argc > 4)
		_FATAL("Failed to parse arguments.\nUsage: %s [-p ASport] [-v]\n", argv[0]);
	
        // else
        for (int i = 1; i < argc; i++){
                if (!strcmp(ARG_ASPORT, argv[i]) && isPortValid((const char*) argv[i+1])
                        && i+1 < argc)
                        strncpy(info->asport, argv[++i], PORT_SIZE);
                else if (!strcmp(ARG_VERBOS, argv[i]))
                        /* activate verbose mode - flag */
                        verbosity = TRUE;
                else 
                        FATAL("Invalid PORT format. \nPlease only use unrestricted ports.");
        }

        /* logs the server information (on debug mod only) */
        _LOG("Runtime settings:\nASport\t: %s\nVerbosity\t: %d", 
                info->asport, verbosity);
}

/* Handle UDP Responses (Incoming Messages) */
void handleUDP(int fd, char *msgBuf) {
	int n;
	char respHead[BUFFER_SIZE]; char respEnd[BUFFER_SIZE];
	
        n = udpReceiveMessage(fd, msgBuf, BUFFER_SIZE);
        setClean();

        sscanf(msgBuf, "%s %s", respHead, respEnd);

        // Registration Request
        if (!strcmp(respHead, REQ_REG))
                ;// TODO registerUser(respEnd);
        // Unregistration Request
        else if (!strcmp(respHead, REQ_UNR))
                ;// TODO unregisterUser(respEnd);                
        // Validation Code received "VLC"
        else if (!strcmp(respHead, RESP_VLC))
                ;// TODO validationCode_Response();
        else
                WARN("Server Error.");
}

void waitMainEvent(int tcpServerFD, int udpFD, char *msgBuf) {
	fd_set fds, ready_fds;
        struct timeval tv, tmp_tv;
        int selectRet, fds_size;
        int nTry = 0;

	/* SELECT */
	FD_ZERO(&fds);
        FD_SET(tcpServerFD, &fds);
        FD_SET(udpFD, &fds);
        fds_size = (tcpServerFD > udpFD) ? tcpServerFD+1 : udpFD+1;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	putStr(STR_INPUT, TRUE);		// string before the user input
        
	while (TRUE) {
		// because select is destructive
		ready_fds = fds;
                tmp_tv = tv;

		selectRet = select(fds_size, &ready_fds, NULL, NULL, &tmp_tv);

		if (selectRet == -1)
			FATAL("Failed System Call Select");
		if (FD_ISSET(udpFD, &ready_fds)){
			// handle PD interaction
                        handleUDP(udpFD, msgBuf);
                }
		if (FD_ISSET(tcpServerFD, &ready_fds)){
			// handle User new connection
                }
		if (selectRet == 0 && isDirty()) {// timeout expired
			// act as previous message didn't reach the target
                        // try to resend NTRIES_NORESP times
			if (nTry < NREQUEST_TRIES)
                                ;//handle no reponse to prev msg
                        else{
                                nTry = 0;
                                WARN("No response received from sent message.\nCommunication error.");
                        }
                }       
                
	}
}



void exitAS() {
        udpDestroySocket(udpServerfd);
        tcpDestroySocket(tcpServerfd);
        exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
        /* AS makes available two server applications? Does it mean 2 process? */
        /* Default AS port. */        
        connectionInfo_t connectionInfo = {"58053\0"};
        parseArgs(argc, argv, &connectionInfo);

        // mount UDP server socket
        udpServerfd = udpCreateServer(NULL, connectionInfo.asport);
        // mount TCP server socket
        tcpServerfd = tcpCreateServer(NULL, connectionInfo.asport, SOMAXCONN);

        waitMainEvent(tcpServerfd, udpServerfd, msgBuffer);

        return 0; // Never used
}