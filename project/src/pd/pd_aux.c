#include "pd_aux.h"

// the buffer containing the last message sent to the server and its size
static char msgBuffer[BUFFER_SIZE * 2];
static int msgSize;

// send server error
bool_t req_serverError(UDPConnection_t *udpConnec) {
        msgSize = sprintf(msgBuffer, "%s%c", SERVER_ERR, CHAR_END_MSG);
        int sizeSent = udpSendMessage(udpConnec, msgBuffer, msgSize);
        if (msgSize != sizeSent) {
                WARN("A problem may have occured while sending the registration request!");
                return FALSE;
        }
        return TRUE;
}


// registers a user on the authentication system.
bool_t req_registerUser(UDPConnection_t *udpConnec, const connectionInfo_t *connectionInfo, const char *uid, 
const char *pass, userInfo_t *userInfo) {
        // check if the user is connected
        if (userInfo->connected) {
                _WARN("A session is already on! Operation ignored.\n\t - Current uid: %s\n\t"
                "Unregister first if you wish to use another user.\n", userInfo->uid);
                return FALSE;
        }

        msgSize = sprintf(msgBuffer, "%s %s %s %s %s%c", REQ_REG, uid, pass, 
        connectionInfo->pdip, connectionInfo->pdport, CHAR_END_MSG);        
        int sizeSent = udpSendMessage(udpConnec, msgBuffer, msgSize);        
        if (msgSize != sizeSent) {
                WARN("A problem may have occured while sending the registration request!");
                return FALSE;
        }

        userInfo->uid = (char*)(malloc((strlen(uid) + 1) * sizeof(char)));
        userInfo->pass = (char*)(malloc((strlen(pass) + 1) * sizeof(char)));
        strcpy(userInfo->uid, uid);
        strcpy(userInfo->pass, pass);        
        return TRUE;
}


// unregisters a user from the authentication system
bool_t req_unregisterUser(UDPConnection_t *udpConnec, userInfo_t *userInfo) {
        // check if the user is connected
        if (!userInfo->connected) {
                WARN("No session is currently opened! Operation ignored.");
                return FALSE;
        }

        msgSize = sprintf(msgBuffer, "%s %s %s%c", REQ_UNR, userInfo->uid, userInfo->pass, CHAR_END_MSG);        
        int sizeSent = udpSendMessage(udpConnec, msgBuffer, msgSize);        
        if (msgSize != sizeSent) {
                WARN("A problem may have occured while sending the unregistration request!");
                return FALSE;
        }

        userInfo->connected = FALSE;
        return TRUE;
}


/*! \brief Replies to the server's validation code request.
 *
 *  Uses the specified status to reply to the server.
 * 
 * \param  fd		        the authentication system's file descriptor.
 * \param  uid                  the uid of the user.
 * \param  status               the status of the request.
 * \return TRUE if the status is STATUS_OK, FALSE otherwise.
 */
bool_t _resp_valCode(UDPConnection_t *udpConnec, UDPConnection_t *res, const char *uid, const char *status) {
        msgSize = sprintf(msgBuffer, "%s %s %s%c", RESP_VLC, uid, status, CHAR_END_MSG);
        int sizeSent = udpSendMessage_specifyConn(udpConnec, res, msgBuffer, msgSize);
        
        if (msgSize != sizeSent) {
                WARN("A problem may have occured while sending the validation code response!");
                return FALSE;
        }
        
        return (!strcmp(status, STATUS_OK) ? TRUE: FALSE);
}


// processes the server's requests to display the 2FA validation code
bool_t resp_valCode(UDPConnection_t *udpConnec, UDPConnection_t *res, char *buf, userInfo_t *userInfo) {
        char uid[BUFFER_SIZE] = { 0 }, vc[BUFFER_SIZE] = { 0 }, 
        fname[BUFFER_SIZE] = { 0 }, fop;
        sscanf(buf+strlen(REQ_VLC), "%s %s %c %s", uid, vc, &fop, fname);

        if (strcmp(uid, userInfo->uid))
                return _resp_valCode(udpConnec, res, userInfo->uid, STATUS_NOK);

        if ((fop == FOP_R || fop == FOP_U || fop == FOP_D) && fname[0] != '\0') {
                printf("VC=%s, %s: %s%c", vc, getFileOp(fop), fname, CHAR_END_MSG);
                return _resp_valCode(udpConnec, res, userInfo->uid, STATUS_OK);
        }

        if ((fop == FOP_L || fop == FOP_X) && fname[0] == '\0') {
                printf("VC=%s, %s%c", vc, getFileOp(fop), CHAR_END_MSG);
                return _resp_valCode(udpConnec, res, userInfo->uid, STATUS_OK);
        }

        return  _resp_valCode(udpConnec, res, userInfo->uid, STATUS_NOK);
}


// checks the response from the server to the register request
bool_t resp_registerUser(char *status, userInfo_t *userInfo) {
        if (!strcmp(status, STATUS_OK)) {
                printf("Registration successful.\n");
                userInfo->connected = TRUE;
                return TRUE;
        }

        // Free memory in case something didn't go so well...
        free(userInfo->uid);
	free(userInfo->pass);
        if (!strcmp(status, STATUS_NOK)) {
                printf("Registration unsuccessful.\n");
                return FALSE;
        }
                
        WARN("Invalid status on the server response! Operation ignored.");
        return FALSE;
}


// checks the response from the server to the unregister request
bool_t resp_unregisterUser(char *status, userInfo_t *userInfo) {
        if (!strcmp(status, STATUS_OK)) {
                printf("Unregistration successful.\n");
                raise(SIGTERM); // unconditional stop execution
        }               

        if (!strcmp(status, STATUS_NOK)) {
                printf("Unregistration unsuccessful.\n");
                raise(SIGTERM); // unconditional stop execution
        }

        WARN("Invalid status on the server response! Operation ignored.");
        return FALSE;
}


// resends the last message sent
bool_t req_resendLastMessage(UDPConnection_t *connection) {   
        int sizeSent = udpSendMessage(connection, msgBuffer, msgSize);        
        if (msgSize != sizeSent) {
                WARN("A problem may have occured while resending the previous message!");
                return FALSE;
        }
        return TRUE;
}