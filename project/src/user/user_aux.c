#include "user_aux.h"


// logins a user on the authentication system.
bool_t req_login(int fd, userInfo_t *userInfo, const char *uid, const char *pass) {
        /* User application sends to the AS the user’s ID UID and a password */
        if (userInfo->connected) {
                _WARN("A session is already on! Operation ignored.\n\t - Current uid: %s\n\t"
                "Unregister first if you wish to use another user.\n", userInfo->uid);
                return FALSE;
        }

        char msgBuffer[BUFFER_SIZE * 2];
        int msgSize = sprintf(msgBuffer, "%s %s %s\n", REQ_LOG, uid, pass);
        int sizeSent = tcpSendMessage(fd, msgBuffer, msgSize);
        if (msgSize != sizeSent) {
                WARN("A problem may have occured while sending the registration request!");
                return FALSE;
        }

        // Adjust size.
        userInfo->uid = (char*)(malloc((strlen(uid) + 1) * sizeof(char)));
        userInfo->pass = (char*)(malloc((strlen(pass) + 1) * sizeof(char)));
        strcpy(userInfo->uid, uid);
        strcpy(userInfo->pass, pass);
        return TRUE
}


bool_t req_request(int fd, const userInfo_t *userInfo, const char *fop, const char *fname) {
        /* User sends a message to 
        the AS requesting a transaction ID code (TID). This request message includes
the UID and the type of file operation desired (Fop), either list (L), retrieve (R),
upload (U), delete (D) or remove (X), and if appropriate (when Fop is R, U or D)
also sends the Fname. */

/* Following the req command, the User application sends a request to the AS to
inform of the user’s desire to perform the operation Fop (either L, R, U, D or X)
on the FS server. If the operation is retrieve (R), upload (U) or delete (D) also
the file name Fname is sent. This message initiates the user’s second factor
authentication procedure. A random natural number of 4 digits is added as a
request identifier RID. Upon receipt of this message, the AS will send the VLC
message to the PD.*/

        int rid, mssgSize;
        rid = randomNumber();
        
        char mssgBuffer[2 * BUFFER_SIZE];
        
        // Fops R, U, D require Fname also to be sent.
        if (!strcmp(fop, FOP_R)|| !strcmp(fop, FOP_U)|| !strcmp(fop, FOP_D) ) {
                // what if R is written without fname?
                mssgSize = sprintf(mssgBuffer, "%s %s %d %s %s\n", REQ_REQ, userInfo->uid, rid, fop, fname);
        } else {
                mssgSize = sprintf(mssgBuffer, "%s %s %d %s\n", REQ_REQ, userInfo->uid, rid, fop);
        }
                
        // Send message = REQ UID RID Fop [Fname] to AS requesting TID.
        int sizeSent = tcpSendMessage(fd, mssgBuffer, mssgSize);

        if (mssgSize != sizeSent) {
                WARN("A problem may have occured while sending the request request!");
                return FALSE;
        }
        return TRUE;
}


bool_t req_val() {
        /*after checking the VC on the PD the user issues this command,
sending a message to the AS with the VC. In reply the AS should confirm (or not)
the success of the two-factor authentication, which should be displayed. The AS
also sends the transaction ID TID.*/
        return TRUE;
}


bool_t req_list() {
        /*following this command the User application establishes a TCP
session with the FS server, asking for the list of files this user has previously
uploaded to the server. The message includes the UID, the TID and the type of
file operation desired (Fop). The reply should be displayed as a numbered list of
filenames and the respective sizes.*/
        return TRUE;
}


bool_t req_retrieve() {
        /* User application establishes a TCP session with the FS server, to retrieve the
selected file filename. The message includes the UID, the TID, the Fop and
Fname. The confirmation of successful transmission (or not) should be
displayed */
        return TRUE;
}


bool_t req_upload() {
        /*  User
application establishes a TCP session with the FS server, to upload the file
filename. The message includes the UID, the TID, the Fop, Fname and the
file size. The confirmation of successful transmission (or not) should be
displayed.*/
        return TRUE;
}


bool_t req_delete() {
        /*following this command the User
application establishes a TCP session with the FS server, to delete the file
filename. The message includes the UID, the TID, the Fop and Fname.
The confirmation of successful deletion (or not) should be displayed.*/
        return TRUE;
}


bool_t req_remove() {
        /*this command is used to request the FS to remove all files and
directories of this User, as well as to request the FS to instruct the AS to delete 
22/10/2020
this user’s login information. The result of the command should be displayed to
the user. The User application then closes all TCP connections and terminates*/
        return TRUE;
}


bool_t req_exit() {
        /*he User application terminates after closing any open TCP connections*/
        return TRUE;
}


bool_t resp_request() {
        return TRUE;
}