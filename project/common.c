#include "common.h"


/* A function to show an error and exit */
void fatal(const char *message) {
   fprintf(stderr, "[!!] Fatal Error: %s\n", message);
   exit(EXIT_FAILURE);
}
