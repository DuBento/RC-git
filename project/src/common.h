#ifndef COMMON_H
#define COMMON_H

/* Generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Generic constants/macros */
#define IP_SIZE 15          /* maximum size of the ip address */
#define PORT_SIZE 5         /* maximum size of the port */
#define UID_SIZE 5          /* size of the UID */
#define PASS_SIZE 8         /* size of the password */

#define ERRORBUF_SIZE 100   /* size of buffer dedicated to errors*/
#define TRUE 1
#define FALSE 0
#define IP_DELIM "."



#define	PDPORTARG	"-d"                /* console argument to specify PDport */
#define	ASIPARG		"-n"                /* console argument to specify ASIP */
#define	ASPORTARG	"-p"          /* console argument to specify ASport */
#define	FSIPARG		"-m"
#define	FSPORTARG	"-q"
#define VERBOSE		"-v"



/* Macro for debug only */
#ifdef DEBUG
    #define LOG(MSG)        printf("\033[1;36m[LOG]: \33[0m" MSG "\n")
    #define _LOG(MSG, ...)  printf("\033[1;36m[LOG]: \33[0m" MSG "\n", __VA_ARGS__)
#else
    #define LOG(MSG)
    #define _LOG(MSG, ...)
#endif


/* Proto */
void fatal(const char *message);
int checkAlfaNum(char *str, int forceLen);
int checkOnlyChar(char *str, int forceLen);
int checkOnlyNum(char *str, int forceLen);
int isNumber(char c);
int isChar(char c);
int checkValidIp(const char *ip_str);
/* A function to show an error and exit */
/* #define FATAL(MSG, ...) {                                       \
    fprintf(stderr, "\033[1;31m[FATAL]: \33[0m" MSG "\n", __VA_ARGS__);     \
    exit(EXIT_FAILURE); }
 */


#endif /* COMMON_H */
