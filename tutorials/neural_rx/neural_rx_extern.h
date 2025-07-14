#ifndef _NR_NEURAL_RX_EXTERN_H__
#define _NR_NEURAL_RX_EXTERN_H__


#include "neural_rx_defs.h"

typedef struct neural_rx_interface_s {
    neural_rx_initfunc_t        *init;
    neural_rx_initfunc_t        *init_thread;
    neural_rx_shutdownfunc_t    *shutdown;
    neural_rx_extract_rbs_t *extract_rbs;
}neural_rx_interface_t;

// global access point for the plugin interface
extern neural_rx_interface_t neural_rx_interface;

int load_neural_rx_lib( char *version, neural_rx_interface_t * );
int free_neural_rx_lib( neural_rx_interface_t *neural_rx_interface );

neural_rx_initfunc_t        neural_rx_init;
neural_rx_shutdownfunc_t    neural_rx_shutdown;
neural_rx_extract_rbs_t neural_rx_extract_rbs;

#endif // _NR_NEURAL_RX_EXTERN_H__
