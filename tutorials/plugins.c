/*
SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
SPDX-License-Identifier: Apache-2.0
*/
#include "plugins.h"
#include <stddef.h>

// START marker-plugins
// global entry points for the tutorial plugins
// any global structures defined here.

demapper_interface_t demapper_interface = {0};
neural_rx_interface_t neural_rx_interface = {0};

//__attribute__((constructor)) void force_init_plugins() {
//    printf("[PLUGIN] Auto-calling init_plugins()\n");
//    init_plugins();
//}

void init_plugins()
{
    // insert your plugin init herie.
    // printf("[PLUGIN-INIT] Loading neural_rx plugin (version = _capture)...\n");
    load_demapper_lib( NULL, &demapper_interface);
    load_neural_rx_lib( NULL, &neural_rx_interface);
}

void free_plugins()
{
    // insert your plugin release/free here.

    free_demapper_lib(&demapper_interface);
    free_neural_rx_lib(&neural_rx_interface);
}

void worker_thread_plugin_init()
{
    // insert your plugin per thread initialization here.

    if (demapper_interface.init_thread)
        demapper_interface.init_thread();

     if (neural_rx_interface.init_thread)
        neural_rx_interface.init_thread();
}
// END marker-plugins
