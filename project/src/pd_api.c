#include "pd_api.h"

void registerUser(int fd, const char *uid, const char *pass, 
        connectionInfo_t connectionInfo, userInfo_t *userInfo) {
        char cmdAS[BUFFERSIZE];
        int size, retSize;
        
        if (userInfo->sessionFlag) {
                warning("Session already on.\nTo register another user please unregister first.");
                return;
        }

        // Update User info
        strcpy(userInfo->uid, uid);
        strcpy(userInfo->pass, pass);
        userInfo->sessionFlag = FALSE;
        
        // REG UID pass PDIP PDport
        size = sprintf(cmdAS, "%s %s %s %s %s%c", REG_REQ, userInfo->uid, userInfo->pass, connectionInfo.pdip,
                        connectionInfo.pdport, ENDMSG);
        
        retSize = udpSendMessage(fd, cmdAS, size);
        
        if (size != retSize) {
                warning("Failed to send registration message.");
                return;
        }
        // Only update msg sent status if message successfuly sent
        setDirty();
}


void unregisterUser(int fd, char* msgBuf, userInfo_t *userInfo) {
	/* verify what comes after - nothing
	 * send to AS: UNR UID pass
	 * receive RUN status
	 */
        int size, retSize;

        // If no user info confirmed
        if (!userInfo->sessionFlag) {
                warning("No session currently log in.");
                return;
        }
        
        // Sending Unregister message "UNR UID pass"
        size = sprintf(msgBuf, "%s %s %s%c", UNREG_REQ, userInfo->uid, userInfo->pass, ENDMSG);
        
        retSize = udpSendMessage(fd, msgBuf, size);
        
        if (size != retSize) {
                warning("Failed to send unregister message.");
                return;
        }
        // Only update msg sent status if message successfuly sent
        setDirty();
}


/* ========== SERVER HANDLING ============= */

void registerUser_Response(char *status, userInfo_t *userInfo) {
        if(!strcmp(status, STATUS_OK)){
                puts("Registration successful.");
                userInfo->sessionFlag = TRUE;
        }
        else if(!strcmp(status, STATUS_NOK))
                puts("Registration unsuccessful.");
}

void sendValidCode_Response(int sockfd, char *msgBuf, const char *status) {
        int size, retSize;
        size = sprintf(msgBuf, "%s %s %c", VALIDCODE_RESP, STATUS_OK, ENDMSG);
        retSize = udpSendMessage(sockfd, msgBuf, size);
        
        if (size != retSize) warning("Failed to send confirmation reply.");
}

void validationCode_Response(int sockfd, char *msgBuf, char *reponse, userInfo_t *userInfo) {
        // Parse the rest of the response received from the server
        char uid[BUFFERSIZE], vc[BUFFERSIZE], fname[BUFFERSIZE];
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

void unregisterUser_Response(char *status, userInfo_t *userInfo) {
        if(!strcmp(status, STATUS_OK)){
                puts("Unregistration was successful.");
                userInfo->sessionFlag = FALSE;
        }
        else if(!strcmp(status, STATUS_NOK))
                puts("Unregistration was unsuccessful.");
}