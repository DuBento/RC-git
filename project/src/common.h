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

/* A function to show an error and exit */
/* #define FATAL(MSG, ...) {                                       \
    fprintf(stderr, "\033[1;31m[FATAL]: \33[0m" MSG "\n", __VA_ARGS__);     \
    exit(EXIT_FAILURE); }
 */


#endif /* COMMON_H */