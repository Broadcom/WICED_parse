/*
 * Copyright (c) 2015 Broadcom
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of Broadcom nor the names of other contributors to this 
 * software may be used to endorse or promote products derived from this software 
 * without specific prior written permission.
 *
 * 4. This software may not be used as a standalone product, and may only be used as 
 * incorporated in your product or device that incorporates Broadcom wireless connectivity 
 * products and solely for the purpose of enabling the functionalities of such Broadcom products.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY WARRANTIES OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT, ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @file
 *
 *PARSE Application
 *
 *
 */

#include "wiced.h"
#include "parse.h"
#include "parse_dct.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/

static void push_received( parse_client_t* client, int error, const char* data );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static parse_client_t client;

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start(void)
{
    parse_dct_t* parse_dct;

    /* Initialise the WICED device */
    wiced_init( );

    /* Bring up the network interface and connect to the Wi-Fi network */
    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

    /* Get keys from DCT */
    wiced_dct_read_lock( (void**) &parse_dct, WICED_FALSE, DCT_APP_SECTION, 0, sizeof(parse_dct_t) );

    /* Initialize Parse */
    parse_init( &client, parse_dct->application_id, parse_dct->client_key, parse_dct->installation_id );

    wiced_dct_read_unlock( parse_dct, WICED_FALSE );

    parse_set_push_callback( &client, push_received );

    WPRINT_APP_INFO( ("Starting push service...\n") );
    parse_start_push_service( &client );

    WPRINT_APP_INFO( ("Starting run loop...\n") );
    parse_run_push_loop( &client );

    wiced_deinit();
}

static void push_received( parse_client_t* client, int error, const char* data )
{
#ifdef PLATFORM_HAS_LEDS
    static int led_state = 0;

    led_state = 1 - led_state;
    if (led_state)
    {
        wiced_gpio_output_high(WICED_LED1);
    }
    else
    {
        wiced_gpio_output_low(WICED_LED1);
    }
#endif
    if ( error == 0 && data != NULL )
    {
        WPRINT_APP_INFO( ("Received push message:\"%s\"\n", data) );
    }
}
