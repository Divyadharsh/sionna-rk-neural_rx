/*
SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
SPDX-License-Identifier: Apache-2.0
*/

#ifndef __NR_NEURAL_RX_CAPTURE_H__
#define __NR_NEURAL_RX_CAPTURE_H__

#include <stdint.h>

// Include the NFAPI SCF interface header that defines nfapi_nr_pusch_pdu_t
#include "nfapi_nr_interface_scf.h"

// Include the gNB PHY definitions header for NR_DL_FRAME_PARMS
#include "defs_gNB.h"

// The function definitions must match the names in the plugin load and the function signatures in openair1/PHY/NR_TRANSPORT/nr_demapper_defs.h

// Plugin API functions

int32_t neural_rx_init( void );

int32_t neural_rx_shutdown( void );

static void nr_ulsch_extract_rbs(c16_t* const rxdataF,
                                 c16_t* const chF,
                                 c16_t *rxFext,
                                 c16_t *chFext,
                                 int rxoffset,
                                 int choffset,
                                 int aarx,
                                 int is_dmrs_symbol,
                                 nfapi_nr_pusch_pdu_t *pusch_pdu,
                                 NR_DL_FRAME_PARMS *frame_parms);


// Internal functions
void capture_neural_rx_data(c16_t* const rxdataF,
                                 c16_t* const chF, 
				 struct timespec *ts);


#endif // __NR_DEMAPPER_ORIG_H__

