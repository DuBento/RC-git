#ifndef COMMON_H
#define COMMON_H


/* generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>


/* generic constants/macros */
#define IP_SIZE     	15          /* maximum size of the ip address */
#define PORT_SIZE   	5         /* maximum size of the port */
#define UID_SIZE    	5          /* size of the UID */
#define PASS_SIZE   	8         /* size of the password */
#define CMD_SIZE	    4          /* size of command */

#define BUFFERSIZE	    64	
#define TRUE		    1
#define FALSE		    0
#define IP_DELIM	    "."
#define ENDMSG		    '\n'
#define	INPUTCHAR	    '>'
#define	RESPONSECHAR    '#'
#define	DELCHAR		    '\b'
#define TIMER_SEC       10          /* seconds for SELECT timer */ 
#define NTRIES_NORESP   3           /* number of resent messages before quiting */

#define	PDPORTARG	"-d"          /* console argument to specify PDport */
#define	ASIPARG		"-n"          /* console argument to specify ASIP */
#define	ASPORTARG	"-p"          /* console argument to specify ASport */
#define	FSIPARG		"-m"
#define	FSPORTARG	"-q"
#define VERBOSE		"-v"

/* macros for the string validation functions */
#define STR_DIGIT           isdigit     // digit matcher
#define STR_ALPHA           isalpha     // alphabetic matcher
#define STR_ALPHALOWER      islower     // lower case alphabetic
#define STR_ALPHAUPPER      isupper     // upper case alphabetic
#define STR_ALPHANUM        isalnum     // alpha numeric matcher
#define STR_NLEN            0           // all lengths considered


/* Protocol commands */
//status
#define STATUS_OK	"OK"
#define STATUS_NOK	"NOK"
//commands
#define REG_REQ		"REG"
#define REG_RESP	"RRG"
#define UNREG_REQ	"UNR"
#define UNREG_RESP	"RUN"
#define VALIDCODE_REQ	"VLC"
#define VALIDCODE_RESP	"RVC"
//file operations
#define FOP_L		'L'
#define FOP_R		'R'
#define FOP_U		'U'
#define FOP_D		'D'
#define FOP_X		'X'

const char *getFileOp(const char op);

/* macro for logging debug messages */
#ifdef DEBUG
    #define LOG(MSG)        printf("\033[1;36m[LOG]: \33[0m" MSG "\n")
    #define _LOG(MSG, ...)  printf("\033[1;36m[LOG]: \33[0m" MSG "\n", __VA_ARGS__)
#else
    #define LOG(MSG)
    #define _LOG(MSG, ...)
#endif


/* Macro for logging fatal errors */
#define FATAL(MSG)          { fprintf(stderr, "\033[1;31m[FATAL]: \33[0m" MSG "\n");               exit(EXIT_FAILURE); }
#define _FATAL(MSG, ...)    { fprintf(stderr, "\033[1;31m[FATAL]: \33[0m" MSG "\n", __VA_ARGS__);  exit(EXIT_FAILURE); }






/* Functions prototypes */

// Functions over previous message sent 
void setDirty();
void setClean();
char isDirty();

/*! \brief Reads the user input.
 *
 *  Reads the user input while checking for errors.
 *
 * \param  buffer a buffer where the message will be stored.
 * \return the pointer to the specified buffer.
 */
void fatal(const char *message);
void warning(const char *message);

char* getUserInput(char *buffer);
void display(const char c);
/*! \brief Initialize Signals
 *
 *  Change default behaviour of SIGINT and SIGTERM
 *
 * \param  handler a pointer to the handling fucntion
 * \return NULL.
 */
void initSignal(void *handler);

/*! \brief Checks if the string is valid using to the match() function and size.
 *
 *  Compares each character of the string with the specified match() functiion,
 *  while also verifying if the string length. 
 *
 * \param  buffer   the buffer containing the string.
 * \param  matcher  the function to check the characters.
 * \param  forceLen the required size of the string.
 * \return the string's length if the string is valid, FALSE otherwise.
 */
int isStringValid(const char* buffer, int (*matcher)(int), int forceLen);


int checkAlfaNum(const char *str, int forceLen);
int checkOnlyChar(const char *str, int forceLen);
int checkOnlyNum(const char *str, int forceLen);

int checkValidIp(const char *ip_str);
int checkValidPORT(const char *str);


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */


#endif /* COMMON_H */
