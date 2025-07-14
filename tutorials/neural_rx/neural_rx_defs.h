#ifndef __NR_NEURAL_RX_DEFS_H__
#define __NR_NEURAL_RX_DEFS_H__

#include <stdint.h>

typedef int32_t(neural_rx_initfunc_t)(void);
typedef int32_t(neural_rx_shutdownfunc_t)(void);

typedef int(neural_rx_extract_rbs_t)( c16_t* const rxdataF,
                                 c16_t* const chF,
                                 c16_t *rxFext,
                                 c16_t *chFext,
                                 int rxoffset,
                                 int choffset,
                                 int aarx,
                                 int is_dmrs_symbol,
                                 nfapi_nr_pusch_pdu_t *pusch_pdu,
                                 NR_DL_FRAME_PARMS *frame_parms);

#endif  // __NR_NEURAL_RX_DEFS_H__

