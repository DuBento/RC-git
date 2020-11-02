#include "as_aux.h"

bool_t req_registerUser(char* args) {
    
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