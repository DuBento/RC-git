#include "common.h"

void fatal(const char *message) {
    fprintf(stderr, "\033[1;31m[FATAL]: \33[0m%s\n", message);
    exit(EXIT_FAILURE);
}
