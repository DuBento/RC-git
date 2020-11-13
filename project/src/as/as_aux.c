#include "as_aux.h"

extern int verbosity;
extern List_t pdList;
extern List_t userList;
extern char mainDir_path[PATH_MAX];

// remove msgs from waitingReply Queue for specified uid
void _cleanQueueFromUID(char *uid) {
        ListIterator_t iter = listIteratorCreate(pdList);
        while (!listIteratorEmpty(&iter)){
                ListNode_t node = (ListNode_t) iter;
                pdNode_t *nodeData = listIteratorNext(&iter);
                if (!strcmp(nodeData->uid, uid))        // remove all with matching uid
                        listRemove(pdList, node, free);
        }
}

void _addMsgToQueue(char* uid, char* msg) {
        pdNode_t *newNode = (pdNode_t*) malloc(sizeof(pdNode_t));
        newNode->nAttempts = 0;
        strcpy(newNode->msg, msg);
        strcpy(newNode->uid, uid);
        listInsert(pdList, newNode);
}

bool_t inUserList(char* uid) {
        ListIterator_t iter = listIteratorCreate(userList);
        while (!listIteratorEmpty(&iter)){
                ListNode_t node = (ListNode_t) iter;
                userNode_t *nodeData = listIteratorNext(&iter);
                if (!strcmp(nodeData->uid, uid))
                        return TRUE;
        }
        return FALSE;
}

userNode_t* _getUserNode(char* uid) {
        ListIterator_t iter = listIteratorCreate(userList);
        while (!listIteratorEmpty(&iter)){
                ListNode_t node = (ListNode_t) iter;
                userNode_t *nodeData = listIteratorNext(&iter);
                if (!strcmp(nodeData->uid, uid))
                        return nodeData;
        }
        return NULL;
}

// deletes everything 
void _removeUID(userNode_t* node) {
        char dirname[FILE_SIZE];
        sprintf(dirname, "%s%s", DIR_NAME, node->uid);
        _cleanQueueFromUID(node->uid);  // clears queue
        deleteDirectory(mainDir_path, dirname);  // clear all file logs
        USER_CLEAR(node);       // unlog user

}

void _removeQueueOP(char* uid, const char* checkOP) {
        char op[BUFFER_SIZE];
        
        ListIterator_t iter = listIteratorCreate(pdList);
        while (!listIteratorEmpty(&iter)){
                ListNode_t node = (ListNode_t) iter;
                pdNode_t *nodeData = listIteratorNext(&iter);
                if (!strcmp(nodeData->uid, uid)){       // matching uid
                        sscanf(nodeData->msg, "%s", op);
                        if (!strcmp(op, checkOP)){
                                listRemove(pdList, node, free);
                                break;  // remove the first
                        }
                }
        }
}

void _cleanLogFile(char* dir_name, const char* fileType) {
	char file[2*FILE_SIZE+PATH_MAX];
        sprintf(file, "%s%s", dir_name, fileType);
        if (deleteFile(mainDir_path, dir_name, file))
                _VERBOSE("Deleted registration file. %s", file);
}

void cleanLogs(DIR* dir){
	struct dirent *ent;
    	while ((ent = readdir(dir)) != NULL) {
		_cleanLogFile(ent->d_name, REGFILE_SUFIX);
                // sprintf(file, "%s%s", ent->d_name, LOGINFILE_SUFIX);
		// if (deleteFile(path, ent->d_name, file))
		// 	_VERBOSE("Deleted login file. %s", file);	
	}
}

/* ====== */
/* UDP    */
/* ====== */
void resendMessagePD(UDPConnection_t *udpConn, pdNode_t *node) {
        UDPConnection_t udpRecv;
        bool_t val = _getUDPConnPD(node->uid, &udpRecv);

        if(val) {
                udpSendMessage_specifyConn(udpConn, &udpRecv, node->msg, strlen(node->msg));
                node->nAttempts++;
        } else
                _WARN("Implementation issue. Trying to resend message to a Personal Device which is not registred.\
                        \nUID: %s.", node->uid);
}

bool_t req_registerPD(UDPConnection_t *udpConn, UDPConnection_t *receiver, char* buf) {
        // parse buf
        char uid[BUFFER_SIZE], pass[BUFFER_SIZE], pdip[BUFFER_SIZE], pdport[BUFFER_SIZE];
        INIT_BUF(uid); INIT_BUF(pass); INIT_BUF(pdip); INIT_BUF(pdport);
        // dir and file manipulation
        char *stored_pass;
        char dirname[FILE_SIZE+BUFFER_SIZE];
        char reg_file[2*FILE_SIZE+BUFFER_SIZE], pass_file[2*FILE_SIZE+BUFFER_SIZE];
        DIR *dir;
        // response
        char answer[BUFFER_SIZE];
        int msgLen;
        
        _VERBOSE("Registering Personal Device:\n\tFrom IP:%s\tPORT:%d", udpConnIp(receiver), udpConnPort(receiver));
        
        // parse buf
        sscanf(buf, "%s %s %s %s", uid, pass, pdip, pdport);
        // Checks
        if (!isUIDValid(uid) || !isPassValid(pass) || !isIPValid(pdip) || !isPortValid(pdport)) {
                _WARN("Invalid arguments received from Personal Device:\nuid: %s\npass: %s\npdip: %s\npdport: %s\nSending error...",
                                uid, pass, pdip, pdport);
                req_serverErrorUDP(udpConn, receiver, buf);
                return FALSE;
        }

        // create dir if does not exist and open dir
        sprintf(dirname, "%s%s", USERDIR_PREFIX, uid); 
        dir = initDir(mainDir_path, dirname, NULL);
        // check if connection not already established (reg file)
        sprintf(reg_file, "%s%s", dirname, REGFILE_SUFIX);
        if (inDir(dir, reg_file)) { 
                _WARN("PD: %s tried to register twice. Sending not ok status...", uid);
                msgLen = sprintf(answer, "%s %s%c", RESP_REG, STATUS_NOK, CHAR_END_MSG);
                udpSendMessage_specifyConn(udpConn, receiver, answer, msgLen);
                return FALSE;
        }
        
        closedir(dir);

        // check if password file doesnt exists, then create
        sprintf(pass_file, "%s%s", dirname, PASSFILE_SUFIX);

        if (retreiveFile(mainDir_path, dirname, pass_file, &stored_pass) > 0){          // file available from previous registration
                if (strcmp(stored_pass, pass) != 0){        // check given password dont match
                        _WARN("Regestring: Passwords don't match:\nuid: %s\npass: %s\tstored pass:%s\nSending error...",
                                uid, pass, stored_pass);
                        msgLen = sprintf(answer, "%s %s%c", RESP_REG, STATUS_NOK, CHAR_END_MSG);
                        udpSendMessage_specifyConn(udpConn, receiver, answer, msgLen);
                        free(stored_pass);
                        return FALSE;
                }
        }       
        else {  // no file to be read, first time connection
                _storePassPD(mainDir_path, dirname, pass_file, pass); // create file
        }

        free(stored_pass);      // if null op not perfomed
        
        _VERBOSE("\tuid: %s\n\tpass: %s\n\tPDip: %s\n\tPDport: %s", uid, pass, pdip, pdport);
        
        _registerPD(mainDir_path, dirname, reg_file, pdip, pdport);

        // reply to PD
        msgLen = sprintf(answer, "%s %s%c", RESP_REG, STATUS_OK, CHAR_END_MSG);
        udpSendMessage_specifyConn(udpConn, receiver, answer, msgLen);
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



bool_t req_unregisterPD(UDPConnection_t *udpConn, UDPConnection_t *receiver, char* buf) {
        // parse buf
        char uid[BUFFER_SIZE], pass[BUFFER_SIZE];
        // dir and file manipulation
        char dirname[FILE_SIZE+BUFFER_SIZE];
        char reg_file[2*FILE_SIZE+BUFFER_SIZE];
        DIR *dir;
        // response
        char answer[BUFFER_SIZE];
        int msgLen;
        
        // parse buf
        sscanf(buf, "%s %s", uid, pass);
        
        _VERBOSE("Unregistering Personal Device:\t\nFrom IP:%s\tPORT:%d", udpConnIp(receiver), udpConnPort(receiver));
        
        // Checks
        if (!isUIDValid(uid) || !isPassValid(pass)) {
                _WARN("Invalid arguments received from Personal Device:\nuid: %s\n pass: %s\nSending error...", 
                        uid, pass);
                req_serverErrorUDP(udpConn, receiver, buf);
                return FALSE;
        }

        sprintf(dirname, "%s%s", USERDIR_PREFIX, uid); 
        sprintf(reg_file, "%s%s", dirname, REGFILE_SUFIX);

        // tries to delete password file
        if (!deleteFile(mainDir_path, dirname, reg_file))
                if(errno == ENOENT){    // file doesnt exist
                        _WARN("PD: %s was not registered. Sending server error...", uid);
                        msgLen = sprintf(answer, "%s %s%c", RESP_UNR, STATUS_NOK, CHAR_END_MSG);
                        udpSendMessage_specifyConn(udpConn, receiver, answer, msgLen);
                        return FALSE;
                }

        // remove msgs from waitingReply Queue
        _cleanQueueFromUID(uid);
        
        _VERBOSE("\tuid: %s\n\tpass: %s", uid, pass);

        // reply to PD
        msgLen = sprintf(answer, "%s %s%c", RESP_UNR, STATUS_OK, CHAR_END_MSG);
        udpSendMessage_specifyConn(udpConn, receiver, answer, msgLen);
        return TRUE;
}

bool_t resp_validationCode(UDPConnection_t *udpConn, UDPConnection_t *receiver, char* buf) {
        // parse buf
        char uid[BUFFER_SIZE], status[BUFFER_SIZE];

        _VERBOSE("Received validation code response from Personal Device:\t\nFrom IP:%s\tPORT:%d", 
                udpConnIp(receiver), udpConnPort(receiver));


        sscanf(buf, "%s %s", uid, status);

        if (!isUIDValid(uid) || !inUserList(uid) || !isStatusValid(status)) {
                req_serverErrorUDP(udpConn, receiver, buf);
                return FALSE;
        }

        // remove from waitinh queue
        _removeQueueOP(uid, REQ_VLC);
        

        _VERBOSE("\tuid: %s\n\tstatus: %s", uid, status);
        resp_fileOP(uid, status);
        
}

bool_t req_authOP(UDPConnection_t *udpConn, UDPConnection_t *receiver, char* buf) {
        char uid[BUFFER_SIZE], tid[BUFFER_SIZE];
        int msgLen;

        _VERBOSE("Authorizing File Op:\n\tFS IP:%s\tPORT:%d", udpConnIp(receiver), udpConnPort(receiver));

        sscanf(buf, "%s %s", uid, tid);

        // checks
        if (!isUIDValid(uid) || !isTIDValid(tid)) {
                req_serverErrorUDP(udpConn, receiver, buf);
                return FALSE;
        }

        _VERBOSE("\tuid: %s\n\ttid: %s", uid, tid);

        // find user in log list
        userNode_t *node = _getUserNode(uid); 

        if(node == NULL) {      // user not loged
                req_serverErrorUDP(udpConn, receiver, buf);
                return FALSE;
        }

        TCPConnection_t *tcpConn = &node->tcpConn;

        if (!IS_TID_CLEAR(node) && node->tid == atoi(tid)){
                if (node->fop == FOP_X){
                        msgLen = sprintf(buf, "%s %s %d %c%c", RESP_VLD, node->uid, node->tid, node->fop, CHAR_END_MSG);
                        _removeUID(node);
                }
                else if (node->fname[0] != '\0')        // op with filename
                        msgLen = sprintf(buf, "%s %s %d %c %s%c", RESP_VLD, node->uid, node->tid, node->fop, node->fname, CHAR_END_MSG);
                else 
                        msgLen = sprintf(buf, "%s %s %d %c%c", RESP_VLD, node->uid, node->tid, node->fop, CHAR_END_MSG);
                VERBOSE("\t## Valid ##");
        }else
                msgLen = sprintf(buf, "%s %s %s %c%c", RESP_VLD, node->uid, tid, FOP_E, CHAR_END_MSG);

        udpSendMessage_specifyConn(udpConn, receiver, buf, msgLen);
        
        TID_CLEAR(node);  // one time only tid, set as clean
        
        return TRUE;
}



// send server error UDP
bool_t req_serverErrorUDP(UDPConnection_t *udpConn, UDPConnection_t *recvConnoc, char *msgBuffer) {
        int msgSize = sprintf(msgBuffer, "%s%c", SERVER_ERR, CHAR_END_MSG);
        int sizeSent = udpSendMessage_specifyConn(udpConn, recvConnoc, msgBuffer, msgSize);
        if (msgSize != sizeSent) {
                WARN("A problem may have occured while sending the registration request!");
                return FALSE;
        }

        _VERBOSE("Error Sent:\n\tIP:%s\tPORT:%d", udpConnIp(recvConnoc), udpConnPort(recvConnoc));
        return TRUE;
}



/* ====== */
/* TCP    */
/* ====== */

bool_t req_loginUser(userNode_t *nodeTCP, char* buf) {
        // parse buf
        char uid[BUFFER_SIZE], pass[BUFFER_SIZE]={0};
        INIT_BUF(uid); INIT_BUF(pass);
        // dir and file manipulation
        char *stored_pass = NULL;
        char dirname[FILE_SIZE+BUFFER_SIZE];
        char login_file[2*FILE_SIZE+BUFFER_SIZE], pass_file[2*FILE_SIZE+BUFFER_SIZE];
        pass_file[0] = '\0';
        // DIR *dir;
        // response
        TCPConnection_t *tcpConn = &nodeTCP->tcpConn;
        char answer[BUFFER_SIZE];
        int msgLen;
        
        _VERBOSE("Login User:\nFrom\tIP:%s\tPORT:%d", tcpConnIp(tcpConn), tcpConnPort(tcpConn));

        // parse buf
        sscanf(buf, "%s %s", uid, pass);
        // Checks
        if (!isUIDValid(uid) || !isPassValid(pass)) {
                _WARN("Invalid arguments received from User at login:\nuid: %s\npass: %s",
                                uid, pass);
                req_serverErrorTCP(tcpConn, buf);
                return FALSE;
        }

        _VERBOSE("\tuid: %s\n\tpass: %s", uid, pass);

        // create dir if does not exist and open dir
        sprintf(dirname, "%s%s", USERDIR_PREFIX, uid); 
       
        // check if password file doesnt exists, then create
        sprintf(pass_file, "%s%s", dirname, PASSFILE_SUFIX);


        if (retreiveFile(mainDir_path , dirname, pass_file, &stored_pass) > 0){          // file available from previous registration
                _LOG("PASS, %s. Stored %s.", pass, stored_pass);
                if (strcmp(stored_pass, pass) != 0){        // check given password dont match
                        _WARN("Login: Passwords don't match:\nuid: %s\npass: %s\tstored pass:%s\nSending error...",
                                uid, pass, stored_pass);
                        msgLen = sprintf(answer, "%s %s%c", RESP_LOG, STATUS_NOK, CHAR_END_MSG);
                        tcpSendMessage(tcpConn, answer, msgLen);
                        free(stored_pass);
                        return FALSE;
                }
                free(stored_pass);
        }       
        else {  // no file to be read, login in without PD first registered
                free(stored_pass);      // if null op not perfomed
                msgLen = sprintf(answer, "%s %s%c", RESP_LOG, STATUS_NOK, CHAR_END_MSG);
                tcpSendMessage(tcpConn, answer, msgLen);
                return FALSE;
        }
        
        // log uid in list node
        strcpy(nodeTCP->uid, uid);
        // _loginUser(path, dirname, login_file, tcpConnIp(tcpConn), tcpConnPort(tcpConn));

        // reply to User
        msgLen = sprintf(answer, "%s %s%c", RESP_LOG, STATUS_OK, CHAR_END_MSG);
        tcpSendMessage(tcpConn, answer, msgLen);
        
        VERBOSE("\t## Valid ##");
        
        return TRUE;
}


void _loginUser(char* relative_path, char* dirname, char* filename, char* ip, int port) {
        char data[BUFFER_SIZE];
        int size;

        size = sprintf(data, "%s\n%d", ip, port);
        storeFile(relative_path, dirname, filename, data, size);
}


bool_t unregisterUser(userNode_t* nodeTCP) {
        char login_file[2*FILE_SIZE+BUFFER_SIZE];
        char dirname[FILE_SIZE+BUFFER_SIZE];
        //response
        TCPConnection_t *tcpConn = &nodeTCP->tcpConn;
        char answer[BUFFER_SIZE];
        int msgLen;

        if (!USER_LOGEDIN(nodeTCP)){
                _VERBOSE("User app exit, no login made.\nIP: %s\t Port: %d", tcpConnIp(tcpConn), tcpConnPort(tcpConn));
                return FALSE;   // no login was made
        }

        sprintf(dirname, "%s%s", USERDIR_PREFIX, nodeTCP->uid); 


        // remove msgs from waitingReply Queue
        _cleanQueueFromUID(nodeTCP->uid);

        _VERBOSE("User app exited.\nFrom\tIP:%s\tPORT:%d\n\tUID: %s", tcpConnIp(tcpConn), tcpConnPort(tcpConn), nodeTCP->uid);
        USER_CLEAR(nodeTCP);         // also clears the uid

        return TRUE;
}


bool_t req_fileOP(userNode_t *node, UDPConnection_t *udpConn, char* buf) {
        // parse buf
        char uid[BUFFER_SIZE], rid[BUFFER_SIZE], fop=0, fname[BUFFER_SIZE];
        INIT_BUF(uid); INIT_BUF(fname);
        // response
        TCPConnection_t *tcpConn = &node->tcpConn;
        UDPConnection_t udpRecv;
        char answer[BUFFER_SIZE];
        int msgLen;

        answer[0]='\0';         // reducing code size

        _VERBOSE("User request file op:\nFrom\tIP:%s\tPORT:%d", tcpConnIp(tcpConn), tcpConnPort(tcpConn));

        // UID RID Fop [Fname]
        sscanf(buf, "%s %s %c %s", uid, rid, &fop, fname);

        // Main checks
        if (!isUIDValid(uid) || !isRIDValid(rid)) {
                req_serverErrorTCP(tcpConn, answer);
                return FALSE;
        }
        else if (!USER_LOGEDIN(node))         // user not loged in
                msgLen = sprintf(answer, "%s %s%c", RESP_REQ, STATUS_ELOG, CHAR_END_MSG);

        else if (!strcmp(getFileOp(fop), "\0"))       // no known file op
                msgLen = sprintf(answer, "%s %s%c", RESP_REQ, STATUS_EFOP, CHAR_END_MSG);
                
               
        else if (strcmp(node->uid, uid))    // uid dont match
                msgLen = sprintf(answer, "%s %s%c", RESP_REQ, STATUS_EUSER, CHAR_END_MSG);

        // if buffer has ben modified
        if (answer[0]!='\0') {
                tcpSendMessage(tcpConn, answer, msgLen);
                return FALSE;
        }

        _VERBOSE("\tuid: %s\n\trid: %s\n\tfop: %c\n\tfname: %s", uid, rid, fop, fname);

        // Try to send request to PD
        bool_t val = _getUDPConnPD(node->uid, &udpRecv);
        
        if (val == FALSE) {
                // unable to talk with PD
                msgLen = sprintf(answer, "%s %s%c", RESP_REQ, STATUS_EPD, CHAR_END_MSG);
                tcpSendMessage(tcpConn, answer, msgLen);
                return FALSE;
        }

        // TODO _verbosity

        int vc = randomNumber(RAND_NUM_MIN, RAND_NUM_MAX);
        // VLC UID VC Fop [Fname]
        if ((fop == FOP_R || fop == FOP_U || fop == FOP_D) && fname[0] != '\0' && isFileNameValid(fname)) {
                // reply
                msgLen = sprintf(answer, "%s %s %d %c %s%c", REQ_VLC, node->uid, vc, fop, fname, CHAR_END_MSG);
                udpSendMessage_specifyConn(udpConn, &udpRecv, answer, msgLen);
                // store info
                node->rid = atoi(rid); node->vc = vc; node->fop = fop;
                strcpy(node->fname, fname);
                // log msg for waiting queue
                _addMsgToQueue(node->uid, answer);
                VERBOSE("\t## Valid ##");
                return TRUE;
        }

        else if ((fop == FOP_L || fop == FOP_X) && fname[0] == '\0') {
                // reply
                msgLen = sprintf(answer, "%s %s %d %c%c", REQ_VLC, node->uid, vc, fop, CHAR_END_MSG);
                udpSendMessage_specifyConn(udpConn, &udpRecv, answer, msgLen);
                // store info
                node->rid = atoi(rid); node->vc = vc; node->fop = fop; FNAME_CLEAR(node);
                // log msg for waiting queue
                _addMsgToQueue(node->uid, answer);
                VERBOSE("\t## Valid ##");
                return TRUE;
        }

        // else no good fop and fname combination, so server error
        req_serverErrorTCP(tcpConn, answer);
        return FALSE;
}


bool_t resp_fileOP(char* uid, char* status) {
        int msgLen;
        char answer[BUFFER_SIZE];
        
        ListIterator_t iter = listIteratorCreate(userList);
        while (!listIteratorEmpty(&iter)){
                ListNode_t node = (ListNode_t) iter;
                userNode_t *nodeData = listIteratorNext(&iter);
                if (!strcmp(nodeData->uid, uid)){       // matching uid
                        msgLen = sprintf(answer, "%s %s%c", RESP_REQ, status, CHAR_END_MSG);
                        tcpSendMessage(&nodeData->tcpConn, answer, msgLen);
                        VERBOSE("\t## Valid ##");
                        return TRUE;     
                }
        }
        _WARN("User %s not found after reciving confirmation of valdation code from Personal Device.\nImplementation Error", uid);
        return FALSE;
}


bool_t req_auth(userNode_t *node, char* buf) {
        // parse buffer
        char uid[BUFFER_SIZE], rid[BUFFER_SIZE], vc[BUFFER_SIZE];
        // responsse
        TCPConnection_t *tcpConn = &node->tcpConn;
        char answer[BUFFER_SIZE];
        int msgLen;
        answer[0]='\0';         // reducing code size

        _VERBOSE("User request authorization:\nFrom\tIP:%s\tPORT:%d", tcpConnIp(tcpConn), tcpConnPort(tcpConn));

        sscanf(buf, "%s %s %s", uid, rid, vc);

        // checks
        if (!isUIDValid(uid) || !isRIDValid(rid) || !isVCValid(vc)) {
                req_serverErrorTCP(tcpConn, buf);
                return FALSE;
        }
        else if (!strcmp(node->uid, uid) && node->rid == atoi(rid) && node->vc == atoi(vc)) {
                node->tid = randomNumber(RAND_NUM_MIN, RAND_NUM_MAX);
                VC_CLEAR(node); RID_CLEAR(node);      // clear vc and rid, one time usage
                msgLen = sprintf(answer, "%s %d%c", RESP_AUT, node->tid, CHAR_END_MSG);
        } else 
                msgLen = sprintf(answer, "%s %s%c", RESP_AUT, AUTH_ERROR, CHAR_END_MSG);

        _VERBOSE("\tuid: %s\n\trid: %s\n\tvc: %c", uid, rid, vc);

        tcpSendMessage(tcpConn, answer, msgLen);

        VERBOSE("\t## Valid ##");
        return TRUE;
}



bool_t _getUDPConnPD(char *uid, UDPConnection_t *conn) {
        // check if password file doesnt exists, then create
        char dirname[FILE_SIZE+BUFFER_SIZE];
        char reg_file[2*FILE_SIZE+BUFFER_SIZE];
        char *stored_data;
        char ip[IP_SIZE+1];
        char port[PORT_SIZE+1];

        sprintf(dirname, "%s%s", USERDIR_PREFIX, uid); 
        sprintf(reg_file, "%s%s", dirname, REGFILE_SUFIX);

        if (retreiveFile(mainDir_path, dirname, reg_file, &stored_data) > 0){          
                // file available from previous registration
                sscanf(stored_data, "%s\n%s", ip, port);                
                free(stored_data);
                udpMakeSockaddr(conn, ip, port);
                return TRUE;
        }
        else {
                // no reg file available, PD not registred
                free(stored_data);      // if null op not perfomed
                conn = NULL;
                return  FALSE;
        }       
}



// send server error UDP
bool_t req_serverErrorTCP(TCPConnection_t *tcpConn, char *msgBuffer) {
        int msgSize = sprintf(msgBuffer, "%s%c", SERVER_ERR, CHAR_END_MSG);
        tcpSendMessage(tcpConn, msgBuffer, msgSize);
        _VERBOSE("Error Sent:\n\tIP:%s\tPORT:%d", tcpConnIp(tcpConn), tcpConnPort(tcpConn));
        return TRUE;
}