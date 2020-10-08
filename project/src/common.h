#ifndef COMMON_H
#define COMMON_H


/* generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/* generic constants/macros */
#define IP_SIZE     15          /* maximum size of the ip address */
#define PORT_SIZE   5         /* maximum size of the port */
#define UID_SIZE    5          /* size of the UID */
#define PASS_SIZE   8         /* size of the password */
#define CMD_SIZE    4          /* size of command

#define TRUE        1
#define FALSE       0
#define IP_DELIM    "."

#define	PDPORTARG	"-d"          /* console argument to specify PDport */
#define	ASIPARG		"-n"          /* console argument to specify ASIP */
#define	ASPORTARG	"-p"          /* console argument to specify ASport */
#define	FSIPARG		"-m"
#define	FSPORTARG	"-q"
#define VERBOSE		"-v"

/* macros for the string validation functions */
#define DIGIT           isdigit     // digit matcher
#define ALPHA           isaplha     // alphabetic matcher
#define ALPHALOWER      islower     // lower case alphabetic
#define ALPHAUPPER      isupper     // upper case alphabetic
#define ALPHANUM        isalnum     // alpha numeric matcher
#define NLEN     0                  // all lengths considered




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



/* Proto */

/*! \brief Reads the user input.
 *
 *  Reads the user input while checking for errors.
 *
 * \param  buffer a buffer where the message will be stored.
 * \return the pointer to the specified buffer.
 */
char* getUserInput(char *buffer);


int checkAlfaNum(const char *str, int forceLen);
int checkOnlyChar(const char *str, int forceLen);
int checkOnlyNum(const char *str, int forceLen);
int checkValidIp(const char *ip_str);
int checkValidPORT(const char *str);
/* A function to show an error and exit */
/* #define FATAL(MSG, ...) {                                       \
    fprintf(stderr, "\033[1;31m[FATAL]: \33[0m" MSG "\n", __VA_ARGS__);     \
    exit(EXIT_FAILURE); }
 */


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \return Return parameter description
 */


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param  Parameter description
 * \param  Parameter description
 * \return Return parameter description
 */


/*! \brief Brief function description here
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \param Parameter Parameter description
 * \return Return parameter description
 */

void fatal(const char *message);


#endif /* COMMON_H */
