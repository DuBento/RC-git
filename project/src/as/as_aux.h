#ifndef AS_AUX_H
#define AS_AUX_H

#include "../common.h"
#include "../udp.h"

#define DIR_NAME	"USERS"
#define USERDIR_PREFIX 	"UID"
#define REGFILE_SUFIX   "_reg.txt"
#define PASSFILE_SUFIX  "_pass.txt"
#define LOGINFILE_SUFIX "_login.txt"
#define TIDFILE_SUFIX   "_tid.txt"
#define FILE_SIZE       32

typedef struct as_node_udp {
        struct as_node_udp *next;
        char uid[UID_SIZE+1];
        bool_t msg_sent;
        char msg[BUFFER_SIZE];
        udpNode_t udp_node;
} asNodeUDP_t;

bool_t req_registerPD(int fd, char* args, char* path);
void registerPD(char* relative_path, char* filename, char* pdip, char* pdport);
void storePassPD(char* relative_path, char* filename, char* pass);

#endif /* AS_AUX_H */