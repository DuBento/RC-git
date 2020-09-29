#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"


#define IP_SIZE 15          /* maximum size of the ip address (in chars) */
#define PORT_SIZE 5         /* maximum size of the port (in chars) */
#define UID_SIZE 5          /* size of the UID */
#define PASS_SIZE 8         /* size of the password */

#define PORTARG "-d"        /* console argument to specify PDport */
#define ASIPARG "-n"        /* console argument to specify ASIP */
#define ASPORTARG "-p"      /* console argument to specify ASport */



typedef struct server_info_t {

    char pdip[IP_SIZE + 1];         /* ip address of the program */
    char pdport[PORT_SIZE + 1];     /* port of the program */
    
    char asip[IP_SIZE + 1];         /* ip address of the autentication server */
    char asport[PORT_SIZE + 1];     /* port of the autentication server */

} serverInfo_t;


typedef struct user_info_t {

    char uid[UID_SIZE + 1];
    char pass[PASS_SIZE + 1];

} userInfo_t;



/* A function to parse console input */
void parse(int nArgs, char *argV[], serverInfo_t *serverInfo) {
    int i;
    if (nArgs < 2 && nArgs > 8){
        printf("Usage: %s PDIP [-d PDport] [-n ASIP] [-p ASport]", argV[0]);
        fatal("Failed to parse arguments");
    }

    /* Set personal device ip addr*/
    strcpy(serverInfo->pdip, argV[1]);
    
    /* Override defaults */
    for (i = 2; i < nArgs; i = i+2){
        if ( !strcmp(PORTARG, argV[i]) )
            strcpy(serverInfo->pdport, argV[i+1]);
        else if ( !strcmp(ASIPARG, argV[i]) )
            strcpy(serverInfo->asip, argV[i+1]);            
        else if ( !strcmp(ASPORTARG, argV[i]) )
            strcpy(serverInfo->asport, argV[i+1]);            
    }
}



/** \brief Registers a user.
 * 
 * 	Waits for user input so that they can register themselves.
 * 
 * 	\param 	userInfo
 *          a pointer to store the user's info.
 *  \return nothing.
 */
void regCmd(const char *buffer, userInfo_t *userInfo) {
    char errCheck = '\0';
    sscanf(buffer, "%5s %s %c", userInfo->uid, userInfo->pass, &errCheck);
    if (errCheck != '\0')
        fatal("Invalid arguments on the reg command!");
}


void unregister() {
    /* sends UNR UID pass to AS
        receives RUN status*/
}


int main(int argc, char *argv[]) {
    serverInfo_t serverInfo = {"", "57053\0", "localhost\0", "58053\0"};
    userInfo_t userInfo = {0};
    char token[BUFSIZ];
    parse(argc, argv, &serverInfo);

    /* SELECT */

    if (fgets(buffer, BUFSIZ, stdin) == NULL)
        fatal("Failed to read user input");

    if ( !strncmp(token, REGCMD, REGCMDLEN) )
        regCmd(buffer + REGCMDLEN, &userInfo);
    else if ( !strcmp(token, EXITCMD, EXITCMDLEN) )
        exitCmd();
    else
        
       /* udp send to(ASfd, "ERR");*/


    /* first command should be reg (what if it's not?) DISPLAY ERR */
    
    /*  establish UDP client connection with AS server
        sends register message REG to AS
        receives RRG from AS
        while command is not exit
            waits for VLCs from AS and displays them
            sends RVC to AS server
        unregister user: send UNR to AS
        receives RUN response from AS
        shut down UDP client connection with AS server
    */  
    /* 
    
    what if exit comes before reg?
    */

    return 0;
}