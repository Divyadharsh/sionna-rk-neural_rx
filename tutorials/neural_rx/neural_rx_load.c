/*
SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
SPDX-License-Identifier: Apache-2.0
*/
#include "common/config/config_userapi.h"
#include "common/utils/LOG/log.h"
#include "common/utils/load_module_shlib.h"
#include "neural_rx_extern.h"

// TODO: Q: can this be inside the loader function?
/* demapper_arg is used to initialize the config module so that the loader works as expected */
static char *neural_rx_arg[64]={"neural_rx_test",NULL};


static int32_t neural_rx_no_thread_init() {
    return 0;
}

int load_neural_rx_lib( char *version, neural_rx_interface_t *interface )
{   printf("[DBG] Enter load_neural_rx_capture_lib\n");
    char *ptr = (char*)config_get_if();
    char libname[64] = "neural_rx";

    if (ptr == NULL) {  // config module possibly not loaded
        uniqCfg = load_configmodule( 1, neural_rx_arg, CONFIG_ENABLECMDLINEONLY );
        logInit();
    }

    // function description array for the shlib loader
    loader_shlibfunc_t shlib_fdesc[] = { {.fname = "neural_rx_init" },
                                         {.fname = "neural_rx_init_thread", .fptr = &neural_rx_no_thread_init },
                                         {.fname = "neural_rx_shutdown" },
                                         {.fname = "neural_rx_extract_rbs" }};

    int ret;
    ret = load_module_version_shlib( libname, version, shlib_fdesc, sizeofArray(shlib_fdesc), NULL );
    AssertFatal((ret >= 0), "Error loading neural_rx library");

    // assign loaded functions to the interface
    interface->init = (neural_rx_initfunc_t *)shlib_fdesc[0].fptr;
    interface->init_thread = (neural_rx_initfunc_t *)shlib_fdesc[1].fptr;
    interface->shutdown = (neural_rx_shutdownfunc_t *)shlib_fdesc[2].fptr;
    interface->extract_rbs = (neural_rx_extract_rbs_t *)shlib_fdesc[3].fptr;

    AssertFatal( interface->init() == 0, "Error starting neural_rx library %s %s\n", libname, version );

    return 0;
}

int free_neural_rx_lib( neural_rx_interface_t *neural_rx_interface )
{
    return neural_rx_interface->shutdown();
}
