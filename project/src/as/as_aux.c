#include "as_aux.h"

extern int verbosity;

/* ====== */
/* UDP    */
/* ====== */


bool_t req_registerPD(UDPConnection_t *udpConnec, UDPConnection_t *receiver, char* buf, char* path) {
        // parse buf
        char uid[BUFFER_SIZE], pass[BUFFER_SIZE], pdip[BUFFER_SIZE], pdport[BUFFER_SIZE];
        // dir and file manipulation
        char *stored_pass;
        char user_path[BUFFER_SIZE+PATH_MAX];
        char dirname[FILE_SIZE+BUFFER_SIZE];
        char reg_file[2*FILE_SIZE+BUFFER_SIZE], pass_file[2*FILE_SIZE+BUFFER_SIZE];
        DIR *dir;
        // response
        char answer[BUFFER_SIZE];
        int msgLen;
        
        // parse buf
        sscanf(buf, "%s %s %s %s", uid, pass, pdip, pdport);
        // Checks
        if (!isUIDValid(uid) || !isPassValid(pass) || !isIPValid(pdip) || !isPortValid(pdport)) {
                _WARN("Invalid arguments received from Personal Device:\nuid: %s\npass: %s\npdip: %s\npdport: %s\nSending error...",
                                uid, pass, pdip, pdport);
                req_serverErrorUDP(udpConnec, receiver, buf);
                return FALSE;
        }

        // create dir if does not exist and open dir
        sprintf(dirname, "%s%s", USERDIR_PREFIX, uid); 
        dir = initDir(path, dirname, user_path);
        // check if connection not already established (reg file)
        sprintf(reg_file, "%s%s", dirname, REGFILE_SUFIX);
        if (inDir(dir, reg_file)) { 
                _WARN("PD: %s tried to register twice. Sending not ok status...", uid);
                msgLen = sprintf(answer, "%s %s%c", RESP_REG, STATUS_NOK, CHAR_END_MSG);
                udpSendMessage_specifyConn(udpConnec, receiver, answer, msgLen);
                return FALSE;
        }

        // check if password file doesnt exists, then create
        sprintf(pass_file, "%s%s", dirname, PASSFILE_SUFIX);

        if (retreiveFile(path, dirname, pass_file, &stored_pass) > 0){          // file available from previous registration
                if (strcmp(stored_pass, pass) != 0){        // check given password dont match
                        _WARN("Regestring: Passwords don't match:\nuid: %s\npass: %s\tstored pass:%s\nSending error...",
                                uid, pass, stored_pass);
                        msgLen = sprintf(answer, "%s %s%c", RESP_REG, STATUS_NOK, CHAR_END_MSG);
                        udpSendMessage_specifyConn(udpConnec, receiver, answer, msgLen);
                        return FALSE;
                }
                free(stored_pass);
        }       
        else {  // no file to be read, first time connection
                _storePassPD(path, dirname, pass_file, pass); // create file
        }
        
        _VERBOSE("Regestring Personal Device:\nuid: %s\npass: %s\npdip: %s\npdport: %s\nFrom IP:%s\tPORT:%d",
                                uid, pass, pdip, pdport, udpConnIp(receiver), udpConnPort(receiver));
        
        _registerPD(path, dirname, reg_file, pdip, pdport);

        // reply to PD
        msgLen = sprintf(answer, "%s %s%c", RESP_REG, STATUS_OK, CHAR_END_MSG);
        udpSendMessage_specifyConn(udpConnec, receiver, answer, msgLen);
        return TRUE;
}



void _registerPD(char* relative_path, char* dirname, char* filename, char* pdip, char* pdport) {
        char data[BUFFER_SIZE];
        int size;

        size = sprintf(data, "%s\n%s", pdip, pdport);
        storeFile(relative_path, dirname, filename, data, size);
}

void _storePassPD(char* relative_path, char* dirname, char* filename, char* pass) {
        storeFile(relative_path, dirname, filename, pass, strlen(pass));
}



bool_t req_unregisterPD(UDPConnection_t *udpConnec, UDPConnection_t *receiver, char* buf, char* path) {
        // parse buf
        char uid[BUFFER_SIZE], pass[BUFFER_SIZE];
        // dir and file manipulation
        char user_path[BUFFER_SIZE+PATH_MAX];
        char dirname[FILE_SIZE+BUFFER_SIZE];
        char reg_path[2*FILE_SIZE+BUFFER_SIZE];
        DIR *dir;
        // response
        char answer[BUFFER_SIZE];
        int msgLen;
        
        // parse buf
        sscanf(buf, "%s %s", uid, pass);
        
        _VERBOSE("Unregestring Personal Device:\nuid: %s\npass: %s\nFrom IP:%s\tPORT:%d",
                uid, pass, udpConnIp(receiver), udpConnPort(receiver));
        
        // Checks
        if (!isUIDValid(uid) || !isPassValid(pass)) {
                _WARN("Invalid arguments received from Personal Device:\nuid: %s\n pass: %s\nSending error...", 
                        uid, pass);
                req_serverErrorUDP(udpConnec, receiver, buf);
                return FALSE;
        }

        sprintf(dirname, "%s%s", USERDIR_PREFIX, uid); 
        sprintf(reg_path, "%s%s/%s%s", path, dirname, dirname, REGFILE_SUFIX);

        // tries to delete password file
        if (remove(reg_path) == -1)
                if(errno == ENOENT){
                        _WARN("PD: %s was not registered. Sending server error...", uid);
                        msgLen = sprintf(answer, "%s %s%c", RESP_UNR, STATUS_NOK, CHAR_END_MSG);
                        udpSendMessage_specifyConn(udpConnec, receiver, answer, msgLen);
                        return FALSE;
                }


        // reply to PD
        msgLen = sprintf(answer, "%s %s%c", RESP_UNR, STATUS_OK, CHAR_END_MSG);
        udpSendMessage_specifyConn(udpConnec, receiver, answer, msgLen);
        return TRUE;
}




// send server error UDP
bool_t req_serverErrorUDP(UDPConnection_t *udpConnec, UDPConnection_t *recvConnoc, char *msgBuffer) {
        int msgSize = sprintf(msgBuffer, "%s%c", SERVER_ERR, CHAR_END_MSG);
        int sizeSent = udpSendMessage_specifyConn(udpConnec, recvConnoc, msgBuffer, msgSize);
        if (msgSize != sizeSent) {
                WARN("A problem may have occured while sending the registration request!");
                return FALSE;
        }
        return TRUE;
}



/* ====== */
/* TCP    */
/* ====== */

bool_t req_loginUser(TCPConnection_t *tcpConnect, char* buf, char* path) {
        // parse buf
        char uid[BUFFER_SIZE], pass[BUFFER_SIZE];
        // dir and file manipulation
        char *stored_pass;
        char user_path[BUFFER_SIZE+PATH_MAX];
        char dirname[FILE_SIZE+BUFFER_SIZE];
        char login_file[2*FILE_SIZE+BUFFER_SIZE], pass_file[2*FILE_SIZE+BUFFER_SIZE];
        DIR *dir;
        // response
        char answer[BUFFER_SIZE];
        int msgLen;
        
        // parse buf
        sscanf(buf, "%s %s", uid, pass);
        // Checks
        if (!isUIDValid(uid) || !isPassValid(pass)) {
                _WARN("Invalid arguments received from User at login:\nuid: %s\npass: %s",
                                uid, pass);
                req_serverErrorTCP(tcpConnect, buf);
                return FALSE;
        }

        // create dir if does not exist and open dir
        sprintf(dirname, "%s%s", USERDIR_PREFIX, uid); 
        dir = initDir(path, dirname, user_path);
        // check if connection not already established (login file)
        sprintf(login_file, "%s%s", dirname, LOGINFILE_SUFIX);
        if (inDir(dir, login_file)) { 
                _WARN("User: %s tried to login twice. Sending not ok status...", uid);
                msgLen = sprintf(answer, "%s %s%c", RESP_LOG, STATUS_NOK, CHAR_END_MSG);
                tcpSendMessage(tcpConnect, answer, msgLen);
                return FALSE;
        }

        // check if password file doesnt exists, then create
        sprintf(pass_file, "%s%s", dirname, PASSFILE_SUFIX);

        if (retreiveFile(path, dirname, pass_file, &stored_pass) > 0){          // file available from previous registration
                if (strcmp(stored_pass, pass) != 0){        // check given password dont match
                        _WARN("Login: Passwords don't match:\nuid: %s\npass: %s\tstored pass:%s\nSending error...",
                                uid, pass, stored_pass);
                        msgLen = sprintf(answer, "%s %s%c", RESP_LOG, STATUS_NOK, CHAR_END_MSG);
                        tcpSendMessage(tcpConnect, answer, msgLen);
                        return FALSE;
                }
                free(stored_pass);
        }       
        else {  // no file to be read, first time connection
                _storePassPD(path, dirname, pass_file, pass); // create file
        }
        
        _VERBOSE("Loging User:\nuid: %s\npass: %s\nFrom IP:%s\tPORT:%d",
                                uid, pass, tcpConnIp(tcpConnect), tcpConnPort(tcpConnect));
        
        _loginUser(path, dirname, login_file, tcpConnIp(tcpConnect), tcpConnPort(tcpConnect));

        // reply to PD
        msgLen = sprintf(answer, "%s %s%c", RESP_LOG, STATUS_OK, CHAR_END_MSG);
        tcpSendMessage(tcpConnect, answer, msgLen);
        return TRUE;
}


void _loginUser(char* relative_path, char* dirname, char* filename, char* ip, int port) {
        char data[BUFFER_SIZE];
        int size;

        size = sprintf(data, "%s\n%d", ip, port);
        storeFile(relative_path, dirname, filename, data, size);
}





// send server error UDP
bool_t req_serverErrorTCP(TCPConnection_t *tcpConnect, char *msgBuffer) {
        int msgSize = sprintf(msgBuffer, "%s%c", SERVER_ERR, CHAR_END_MSG);
        tcpSendMessage(tcpConnect, msgBuffer, msgSize);
        return TRUE;
}