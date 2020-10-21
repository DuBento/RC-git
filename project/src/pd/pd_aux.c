#include "protocol.h"

// registers a user on the authentication system.
bool_t req_registerUser(int fd, const connectionInfo_t *connectionInfo, userInfo_t *userInfo) {
        char buffer[BUFSIZ];

        // check if the user is connected
        if (userInfo->connected) {
                _WARN("A session is already on! Operation ignored.\n\t - Current uid: %s\n\t"
                "Unregister first if you wish to use another user.\n", userInfo->uid);
                return FALSE;
        }

        // send the request
        int formatSize = sprintf(buffer, "%s %s %s %s %s%c", REQ_REG, userInfo->uid, userInfo->pass, 
        connectionInfo->pdip, connectionInfo->pdport, '\n');        
        int sentSize = udpSendMessage(fd, buffer, formatSize);        
        if (formatSize != sentSize) {
                WARN("A problem occured while sending the unregistration request!");
                return FALSE;
        }
        
        return TRUE;
}


// unregisters a user from the authentication system
bool_t req_unregisterUser(int fd, userInfo_t *userInfo) {
        char buffer[BUFSIZ];

        // check if the user is connected
        if (!userInfo->connected) {
                WARN("No session is currently opened! Operation ignored.");
                return FALSE;
        }
        
        // send the request
        int sizeFormat = sprintf(buffer, "%s %s %s%c", REQ_UNR, userInfo->uid, userInfo->pass, '\n');        
        int sizeSent = udpSendMessage(fd, buffer, sizeFormat);        
        if (sizeFormat != sizeSent) {
                WARN("A problem occured while sending the unregistration request!");
                return FALSE;
        }

        return TRUE;
}


// checks the response from the server to the register request
bool_t resp_registerUser(char *status, userInfo_t *userInfo) {
        if (!strcmp(status, STATUS_OK)) {
                printf("Registration successful.\n");
                userInfo->connected = TRUE;
                return TRUE;
        }

        free(userInfo->uid);
	free(userInfo->pass);
	printf("Registration unsuccessful.\n");
        if (!strcmp(status, STATUS_NOK))
                return FALSE;

        WARN("Invalid status on the server response! Operation ignored.");
        return FALSE;
}




#define VALIDCODE "VC"
void sendValidCode_Response(int sockfd, char *msgBuf, const char *status) {
        int size, retSize;
        size = sprintf(msgBuf, "%s %s %c", RESP_VAL, STATUS_OK, '\n');
        retSize = udpSendMessage(sockfd, msgBuf, size);
        
        if (size != retSize) warning("Failed to send confirmation reply.");
}


// processes the server's requests to display the 2FA validation code
bool_t req_valCode(int sockfd, char *buffer, userInfo_t *userInfo) {
        // Parse the rest of the response received from the server
        char uid[BUFFER_SIZE], vc[BUFFER_SIZE], fname[BUFFER_SIZE];
        char fop;
        *fname = '\0';
        sscanf(reponse, "%s %s %c %s", uid, vc, &fop, fname);
        if(!strcmp(uid, userInfo->uid)){
                // Operations over files (with filename)
                if((fop == FOP_R || fop == FOP_U || fop == FOP_D)
                        && *fname != '\0')
                        printf("%s=%s, %s: %s", VALIDCODE, vc, getFileOp(fop), fname);
                else if(fop == FOP_L || fop == FOP_X)
                        printf("%s=%s, %s", VALIDCODE, vc, getFileOp(fop));
                
                // Respond to server OK
                sendValidCode_Response(sockfd, msgBuf, STATUS_OK);
        } else 
                // Respond to server NOT OK
                sendValidCode_Response(sockfd, msgBuf, STATUS_NOK);

}


// checks the response from the server to the unregister request
bool_t resp_unregisterUser(char *status, userInfo_t *userInfo) {
        if (!strcmp(status, STATUS_OK)) {                
                printf("Unregistration successful.\n");
		raise(SIGTERM);
        }

        printf("Unregistration unsuccessful.\n");
        if (!strcmp(status, STATUS_NOK))
                return FALSE;

        WARN("Invalid status on the server response! Operation ignored.");
        return FALSE;
}