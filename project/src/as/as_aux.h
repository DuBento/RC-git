#ifndef AS_AUX_H
#define AS_AUX_H

#include "../common.h"
#include "../udp.h"

typedef struct as_node_udp {
        struct as_node_udp *next;
        char uid[UID_SIZE+1];
        bool_t msg_sent;
        char msg[BUFFER_SIZE];
        udpNode_t udp_node;
} asNodeUDP_t;

bool_t req_registerUser(char* args);

#endif /* AS_AUX_H */