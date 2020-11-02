#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>



/* Generic constants. */
typedef char bool_t;
#define TRUE		1
#define FALSE		0
#define DIR_NAME	"USERS"
#define FILE_SUFIX 	"UID"

#define TEJO_IP "193.136.138.142\0"
#define TEJO_FS_PORT	"59000\0"
#define TEJO_AS_PORT	"58011\0"

#define	DEFAULT_PD_PORT	"57053\0"
#define DEFAULT_FS_PORT	"59053\0"
#define	DEFAULT_AS_PORT	"58053\0"

/* General implementation constants  */
#define UID_SIZE	5

/* Execution arguments */
#define	ARG_PDPORT	"-d"	// the execution argument to specify the PDport
#define	ARG_ASIP	"-n"	// the execution argument to specify the ASIP
#define	ARG_ASPORT	"-p"	// the execution argument to specify the ASport
#define	ARG_FSIP	"-m"	// the execution argument to specify the FSIP
#define	ARG_FSPORT	"-q"	// the execution argument to specify the FSport
#define ARG_VERBOS	"-v"	// the execution argument to specify the server verbose mode

#define ARG_IP          1
#define ARG_PORT 	2
#define ARG_STR_IP	"IP address"
#define ARG_STR_PORT	"Port number"
#define ARG_USAGE_IP	"xxx.xxx.xxx.xxx"
#define ARG_USAGE_PORT	"xxxxx"

/* Connection constants. */
#define IP_SIZE		15	// the maximum size of the IPv4 address
#define PORT_SIZE	5	// the maximum size of the port
#define IP_BLOCK_MIN	0	// the minimum value of each block of the IPv4
#define IP_BLOCK_MAX	255	// the minimum value of each block of the IPv4
#define PORT_MIN	1000	// the minumum value of the port allowed
#define PORT_MAX	65535	// the maximum value of the port allowed

/* Protocol constants. */
#define CMD_SIZE	4	// the size of command
#define UID_SIZE	5	// the size of the UID
#define PASS_SIZE	8	// the size of the password
#define BUFFER_SIZE	128	// the maximum size of the buffer [ALOCATE DYNAMIC]

#define BUFFER_SIZE	128	// the maximum number of characters of a message/input line
#define TIMEOUT		10	// the number of seconds before timeout
#define NREQUEST_TRIES	3	// the number of resent messages before quiting
#define	STR_INPUT	"> "	// the string before the user input
#define	STR_RESPONSE	"# "	// the string before the server output
#define STR_CLEAN	"\b\b"	// the string to clean the input and response strings
#define CHAR_SEP_MSG	' '	// the sepatation character on a protocol message
#define CHAR_END_MSG	'\n'	// the last character of a protocol message

/* Protocol commands */
#define SERVER_ERR	"ERR"

#define REQ_REG		"REG"
#define REQ_UNR		"UNR"
#define REQ_VLC		"VLC"

#define REQ_LOG		"LOG"
#define REQ_REQ		"REQ"
#define REQ_RRQ		"RRQ"
#define	REQ_AUT		"AUT"

#define RESP_REG	"RRG"
#define RESP_UNR	"RUN"
#define RESP_VLC	"RVC"

#define RESP_LOG	"RLO"
#define RESP_REQ	"RRQ"
#define RESP_AUT	"RAU"


/* Protocol status */
#define STATUS_OK	"OK"
#define STATUS_NOK	"NOK"

#define STATUS_EPD	"EPD"
#define	STATUS_ELOG "ELOG"
#define	STATUS_EUSER "EUSER"
#define	STATUS_EFOP	"EFOP"

#define STATUS_DUP	"DUP"
#define	STATUS_FULL	"FULL"
#define STATUS_INV	"INV"

/* file operations */
#define FOP_L		'L'
#define FOP_R		'R'
#define FOP_U		'U'
#define FOP_D		'D'
#define FOP_X		'X'



/* Macro for logging debug messages. */
#ifdef DEBUG
	#define LOG(MSG)	printf("\033[1;36m[LOG]: \33[0m" MSG "\n")
	#define _LOG(MSG, ...)	printf("\033[1;36m[LOG]: \33[0m" MSG "\n", __VA_ARGS__)
#else
	#define LOG(MSG)
	#define _LOG(MSG, ...)
#endif


/* Macro for logging warnings fatal errors. */
#define WARN(MSG)		{ fprintf(stderr, "\033[1;33m[WARNING]: \33[0m" MSG "\n");}
#define _WARN(MSG, ...)		{ fprintf(stderr, "\033[1;33m[WARNING]: \33[0m" MSG "\n", __VA_ARGS__); }

#define FATAL(MSG)		{ fprintf(stderr, "\033[1;31m[FATAL]: \33[0m" MSG "\n");               raise(SIGABRT); }
#define _FATAL(MSG, ...)	{ fprintf(stderr, "\033[1;31m[FATAL]: \33[0m" MSG "\n", __VA_ARGS__);  raise(SIGABRT); }



/*! \brief Initializes the program termination signals.
 *
 *  Change the actions of the SIGINT and SIGTERM signals to match the specified
 *  handler function.
 *
 * \param  handlerSucc	the pointer to success termination handeling function.
 * \param  handlerUn	the pointer to unsuccess termination handeling function.
 */
void initSignal(void *handlerSucc, void *handlerUn);



/* Macros for the string validation functions. */
#define STR_DIGIT	isdigit		// digit matcher
#define STR_ALPHA	isalpha		// alphabetic matcher
#define STR_ALPHALOWER	islower		// lower case alphabetic
#define STR_ALPHAUPPER	isupper		// upper case alphabetic
#define STR_ALPHANUM	isalnum		// alpha numeric matcher
#define STR_ALLLEN	0 		// all lengths considered

/*! \brief Checks if the string is valid using to the match() function and size.
 *
 *  Compares each character of the string with the specified match() functiion,
 *  while also verifying if the string length. 
 *
 * \param  buffer	the buffer containing the string.
 * \param  matcher	the function to check the characters.
 * \param  forceLen	the required size of the string.
 * \return the string's length if the string is valid, 0 otherwise.
 */
size_t isStringValid(const char* buffer, int (*matcher)(int), int forceLen);



/*! \brief Checks if the IP address' format is valid (in dot notaion).
 *
 *  Checks if the IP address is made of four blocks separated by '.' and
 *  each of them is a number between IP_BLOCK_MIN and IP_BLOCK_MAX.
 *
 * \param  buffer	the buffer containing the ip address.
 * \return TRUE if the IP address' format is valid, FALSE otherwise.
 */
bool_t isIPValid(const char *buffer);


/*! \brief Checks if the port number's format is valid.
 *
 *  Checks if the port is a number between PORT_MIN and PORT_MAX.
 * 
 * \param  buffer	the buffer containing the port number.
 * \return TRUE if the IP address' format is valid, FALSE otherwise.
 */
bool_t isPortValid(const char *buffer);


/*! \brief Checks if the UID is valid.
 *
 *  Checks if the UID is a number with 5 digits.
 * \param  buffer	the buffer containing the UID.
 * \return TRUE if the UID is valid, FALSE otherwise.
 */
bool_t isUIDValid(const char *buffer);


/*! \brief Checks if the password is valid.
 *
 *  Checks if the password is a alphanumerical string with 8 symbols.
 * \param  buffer	the buffer containing the password.
 * \return TRUE if the password is valid, FALSE otherwise.
 */
bool_t isPassValid(const char *buffer);



/*! \brief Reads an input line from the user.
 *
 *  Stores the input line read from the user on a temporary buffer and then
 *  copies it to a dynamic allocated string returning a pointer to it.
 * 
 * \param   buffer	the buffer where the input will be stored.
 * \param   size	the size of the specified buffer.
 * \return  the pointer to the buffer if the input didn't exceed the buffer's 
 *          size, NULL otherwise.
 */
char* getUserInput(char *buffer, size_t size);



/*! \brief Outputs a string character by character.
 *
 *  Writes the specified string on to the output stream, character by character,
 *  using the putchar() function.
 * 
 * \param  buffer	the string to be displayed.
 * \param  flush	TRUE to flush the output stream, FALSE, otherwise.
 */
void putStr(const char *buffer, bool_t flush);



/*! \brief Gets the string correspondent to the file operation.
 *
 *  Writes the specified string on to the output stream, character by character,
 *  using the putchar() function.
 * 
 * \param  op		file operation
 * \return the string correspondent to the file operation.
 */ 
const char *getFileOp(const char op);



/*! \brief Generates a random number
 *
 *  Generates a random number between the min and max and resets the random seed.
 * 
 * \param  min
 * \param  max
 * \return a random number between min and max.
 */ 
int randomNumber(int min, int max);


/*! \brief Initialize a directory near the executable
 *
 *  Opens a directory. Also creates if it does not exist.
 * 
 * \param  sufix
 * \param  dirname
 * \return an open directory named `dirname`.
 */ 
DIR* initDirectory(char* sufix, const char* dirname);


#endif 	/* COMMON_H */
