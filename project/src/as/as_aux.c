#include "as_aux.h"

bool_t req_registerPD(int fd, char* buf, char* path) {
        // parse buf
        char uid[BUFFER_SIZE], pass[BUFFER_SIZE], pdip[BUFFER_SIZE], pdport[BUFFER_SIZE];
        // dir and file manipulation
        char user_path[BUFFER_SIZE+PATH_MAX];
        char dirname[FILE_SIZE+BUFFER_SIZE];
        char reg_file[2*FILE_SIZE+BUFFER_SIZE], pass_file[2*FILE_SIZE+BUFFER_SIZE];
        DIR *dir;
        // response
        char answer[BUFFER_SIZE];
        int msgLen;
        
        // parse buf
        sscanf(buf+strlen(REQ_REG) , "%s %s %s %s\n", uid, pass, pdip, pdport);
        _LOG("Arguments received from Personal Device:\nuid: %s\npass: %s\npdip: %s\npdport: %s\n",
                                uid, pass, pdip, pdport);
        
        // Checks
        if (!isUIDValid(uid) || !isPassValid(pass)) {
                if (!isIPValid(pdip) || !isPortValid(pdport)) {
                        _WARN("Invalid arguments received from Personal Device:\nuid: %s\npass: %s\npdip: %s\npdport: %s\nUnable to send error.",
                                uid, pass, pdip, pdport);
                        return FALSE;
                }
                _WARN("Invalid arguments received from Personal Device:\nuid: %s\n pass: %s\nSending error...", 
                        uid, pass);
                msgLen = sprintf(answer, "%s %s", RESP_REG, STATUS_NOK);
                udpSendMessage_specify(fd, answer, msgLen, pdip, pdport);
                return FALSE;
        }

        // create dir if does not exist and open dir
        sprintf(dirname, "%s%s", USERDIR_PREFIX, uid); 
        dir = initDirectoryFromExe(path, dirname, user_path);
        printf("old path:%s\n", user_path);
        // check if connection not already established (reg file)
        sprintf(reg_file, "%s%s", dirname, REGFILE_SUFIX);
        if (inDir(dir, reg_file)) { 
                _WARN("User: %s tried to register twice. Sending server error...", uid);
                msgLen = sprintf(answer, "%s %s", RESP_REG, STATUS_NOK);
                udpSendMessage_specify(fd, answer, msgLen, pdip, pdport);
                return FALSE;
        }

        // check if password file doesnt exists, then create
        sprintf(pass_file, "%s%s", dirname, PASSFILE_SUFIX);
        if (!inDir(dir, pass_file))
                storePassPD(user_path, pass_file, pass); // create file
        registerPD(user_path, reg_file, pdip, pdport);

        // reply to PD
        msgLen = sprintf(answer, "%s %s", RESP_REG, STATUS_OK);
        udpSendMessage_specify(fd, answer, msgLen, pdip, pdport);
}



void registerPD(char* relative_path, char* filename, char* pdip, char* pdport) {
        char file_path[BUFFER_SIZE+PATH_MAX];
        char data[BUFFER_SIZE];
        int size;
        fprintf(stderr, "registering");

        size = sprintf(data, "%s\n%s", pdip, pdport);
        sprintf(file_path, "%s/%s", relative_path, filename);
        createFile(file_path, data, size);    // doesnt include nullbyte
}

void storePassPD(char* relative_path, char* filename, char* pass) {
        char file_path[BUFFER_SIZE+PATH_MAX];
        puts("storing pass...");
        sprintf(file_path, "%s/%s", relative_path, filename);
        printf("File Path:%s", file_path);
        createFile(file_path, pass, strlen(pass));    // doesnt include nullbyte
}






// send server error
// bool_t req_serverError(int fd) {
//         msgSize = sprintf(msgBuffer, "%s\n", SERVER_ERR);
//         int sizeSent = udpSendMessage(fd, msgBuffer, msgSize);
//         if (msgSize != sizeSent) {
//                 WARN("A problem may have occured while sending the registration request!");
//                 return FALSE;
//         }
//         return TRUE;
// }