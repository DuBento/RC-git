#ifndef PD_API_H
#define PD_API_H

#include "common.h"
#include "udp.h"

#define VALIDCODE "VC"

/* the information to allow communication with the autentication server */
typedef struct connectionInfo_t {

        char pdip[IP_SIZE + 1];                 /* ip address of the program */
        char pdport[PORT_SIZE + 1];             /* port of the program */        
        char asip[IP_SIZE + 1];                 /* ip address of the autentication server */
        char asport[PORT_SIZE + 1];             /* port of the autentication server */

} connectionInfo_t;


/* the user's information */
typedef struct user_info_t {

        char uid[BUFFERSIZE];
        char pass[BUFFERSIZE];
        char sessionFlag;

} userInfo_t;


void registerUser(int fd, const char *uid, const char *pass, 
        connectionInfo_t connectionInfo, userInfo_t *userInfo);
void unregisterUser(int fd, char* msgBuf, userInfo_t *userInfo);
void registerUser_Response(char *status, userInfo_t *userInfo);
void sendValidCode_Response(int sockfd, char *msgBuf, const char *status);
void validationCode_Response(int sockfd, char *msgBuf, char *reponse, userInfo_t *userInfo);
void unregisterUser_Response(char *status, userInfo_t *userInfo);

#endif /* PD_API_H */
