/*
SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
SPDX-License-Identifier: Apache-2.0
*/
#include "openair1/PHY/TOOLS/tools_defs.h"
#include "openair1/PHY/sse_intrin.h"
#include "nr_neural_rx_capture.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "PHY/defs_gNB.h"
#include "PHY/phy_extern.h"
#include "nr_transport_proto.h"
#include "PHY/impl_defs_top.h"
#include "PHY/NR_TRANSPORT/nr_sch_dmrs.h"
#include "PHY/NR_REFSIG/dmrs_nr.h"
#include "PHY/NR_REFSIG/ptrs_nr.h"
#include "PHY/NR_ESTIMATION/nr_ul_estimation.h"
#include "PHY/defs_nr_common.h"
#include "common/utils/nr/nr_common.h"
#include <openair1/PHY/TOOLS/phy_scope_interface.h>
#include "PHY/sse_intrin.h"
#include <../../nfapi/open-nFAPI/nfapi/public_inc/nfapi_nr_interface_scf.h>

#ifdef __aarch64__
#define USE_128BIT
#endif


static char *filename_in = "channel_est_in.txt";
static FILE *f_in;
static pthread_mutex_t capture_lock = PTHREAD_MUTEX_INITIALIZER;

// import default implementations
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


/* configure which clock source to use, will affect resolution */
#define TIMESTAMP_CLOCK_SOURCE CLOCK_MONOTONIC

/* time processing functions */
void fprint_time( FILE* file, struct timespec *ts )
{
  fprintf( file, "%jd.%09ld\n", ts->tv_sec, ts->tv_nsec );
  return;
}

// START marker-capture-input-format
/*
 * Input file format (f_in):
 * time source resolution (sec.nanosec) - only first line
 * timestamp (sec.nanosec)
 * modulation scheme (string: QPSK, QAM16)
 * nb_re (int32)
 * real imag (for QPSK: int16 <space> int16)
 * real imag ch_mag.r ch_mag.i (for QAM16: int16 <space> int16 <space> int16 <space> int16)̦̦̦̦̦̦
 * ... (done nb_re times)
 */
// END marker-capture-input-format

// START marker-capture-output-format
/*
 * Output file format (f_out):
 * time source resolution (sec.nanosec) - only first line
 * timestamp (sec.nanosec)
 * modulation scheme (string: QPSK, QAM16)
 * nb_re (int32)
 * real imag llr.r llr.i (int16 <space> int16 <space> int16 <space> int16 )
 * --->number of elements in each row depend on the modulation scheme: QPSK: 2 ; QAM16: 4
 * ... (done nb_re times)
 */
// END marker-capture-output-format


void capture_neural_rx_data(c16_t* const rxdataF,
                                 c16_t* const chF, struct timespec *ts)
{
    pthread_mutex_lock( &capture_lock );

    c16_t *rxF = (c16_t *)rxdataF;

    fprint_time( f_in, ts );
    fprintf( f_in, "RX_DATA\n" );
    for(int i = 0; i < 12; i++ )
      fprintf( f_in, "%hd %hd\n", rxF[i].r, rxF[i].i );
    fflush( f_in );

    pthread_mutex_unlock( &capture_lock );
}

// Plugin Init / Shutdown

int32_t neural_rx_init( void )
{
    // initialize capture mutex
    pthread_mutex_init( &capture_lock, NULL );

    // open capture files
    f_in = fopen( filename_in, "w" );
    AssertFatal( f_in != NULL, "Cannot open file %s for writing\n", filename_in );

   // f_out = fopen( filename_out, "w");
   // AssertFatal( f_out != NULL, "Cannot open file %s for writing\n", filename_out );

    // print clock resolution
    struct timespec ts;
    clock_getres( TIMESTAMP_CLOCK_SOURCE, &ts );
    fprint_time( f_in, &ts );
   // fprint_time( f_out, &ts );

    return 0;
}

int32_t neural_rx_shutdown( void )
{
    fclose( f_in );
    //fclose( f_out );

    pthread_mutex_destroy( &capture_lock );

    return 0;
}

int neural_rx_extract_rbs(c16_t* const rxdataF,
                                 c16_t* const chF,
                                 c16_t *rxFext,
                                 c16_t *chFext,
                                 int rxoffset,
                                 int choffset,
                                 int aarx,
                                 int is_dmrs_symbol,
                                 nfapi_nr_pusch_pdu_t *pusch_pdu,
                                 NR_DL_FRAME_PARMS *frame_parms)
{
  struct timespec ts;
  clock_gettime( TIMESTAMP_CLOCK_SOURCE, &ts );
   uint8_t delta = 0;
  int start_re = (frame_parms->first_carrier_offset + (pusch_pdu->rb_start + pusch_pdu->bwp_start) * NR_NB_SC_PER_RB)%frame_parms->ofdm_symbol_size;
  int nb_re_pusch = NR_NB_SC_PER_RB * pusch_pdu->rb_size;
  c16_t *rxF = &rxdataF[rxoffset];
  c16_t *rxF_ext = &rxFext[0];
  c16_t *ul_ch0 = &chF[choffset];
  c16_t *ul_ch0_ext = &chFext[0];

  if (is_dmrs_symbol == 0) {
    if (start_re + nb_re_pusch <= frame_parms->ofdm_symbol_size)
      memcpy(rxF_ext, &rxF[start_re], nb_re_pusch * sizeof(c16_t));
    else {
      int neg_length = frame_parms->ofdm_symbol_size - start_re;
      int pos_length = nb_re_pusch - neg_length;
      memcpy(rxF_ext, &rxF[start_re], neg_length * sizeof(c16_t));
      memcpy(&rxF_ext[neg_length], rxF, pos_length * sizeof(c16_t));
    }
    memcpy(ul_ch0_ext, ul_ch0, nb_re_pusch * sizeof(c16_t));
    capture_neural_rx_data(rxdataF, chF, &ts);
  }
  else if (pusch_pdu->dmrs_config_type == pusch_dmrs_type1) { // 6 REs / PRB
    AssertFatal(delta == 0 || delta == 1, "Illegal delta %d\n",delta);
    c16_t *rxF32 = &rxF[start_re];
    if (start_re + nb_re_pusch < frame_parms->ofdm_symbol_size) {
      for (int idx = 1 - delta; idx < nb_re_pusch; idx += 2) {
        *rxF_ext++ = rxF32[idx];
        *ul_ch0_ext++ = ul_ch0[idx];
      }
    }
    else { // handle the two pieces around DC
      int neg_length = frame_parms->ofdm_symbol_size - start_re;
      int pos_length = nb_re_pusch - neg_length;
      int idx, idx2;
      for (idx = 1 - delta; idx < neg_length; idx += 2) {
        *rxF_ext++ = rxF32[idx];
        *ul_ch0_ext++= ul_ch0[idx];
      }
      rxF32 = rxF;
      idx2 = idx;
      for (idx = 1 - delta; idx < pos_length; idx += 2, idx2 += 2) {
        *rxF_ext++ = rxF32[idx];
        *ul_ch0_ext++ = ul_ch0[idx2];
      }
    }
  }
  else if (pusch_pdu->dmrs_config_type == pusch_dmrs_type2) { // 8 REs / PRB
    AssertFatal(delta==0||delta==2||delta==4,"Illegal delta %d\n",delta);
    if (start_re + nb_re_pusch < frame_parms->ofdm_symbol_size) {
      for (int idx = 0; idx < nb_re_pusch; idx ++) {
        if (idx % 6 == 2 * delta || idx % 6 == 2 * delta + 1)
          continue;
        *rxF_ext++ = rxF[idx];
        *ul_ch0_ext++ = ul_ch0[idx];
      }
    }
    else {
      int neg_length = frame_parms->ofdm_symbol_size - start_re;
      int pos_length = nb_re_pusch - neg_length;
      c16_t *rxF64 = &rxF[start_re];
      int idx, idx2;
      for (idx = 0; idx < neg_length; idx ++) {
        if (idx % 6 == 2 * delta || idx % 6 == 2 * delta + 1)
          continue;
        *rxF_ext++ = rxF64[idx];
        *ul_ch0_ext++ = ul_ch0[idx];
      }
      rxF64 = rxF;
      idx2 = idx;
      for (idx = 0; idx < pos_length; idx++, idx2++) {
        if (idx % 6 == 2 * delta || idx % 6 == 2 * delta + 1)
          continue;
        *rxF_ext++ = rxF64[idx];
        *ul_ch0_ext++ = ul_ch0[idx2];
      }
    }
  }
  
  return 1;
}

