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
 */

#include "wiced.h"
#include "http.h"
#include "parse.h"
#include <stdio.h>
#include <ctype.h>
#include "wiced_result.h"
#include "wiced_utilities.h"
#include "http_stream.h"
#include "parse_http.h"
#include "dns.h"
#include "wiced_tls.h"
#include "simplejson.h"
#include "uuid.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define TCP_SERVER_LISTEN_PORT              (50007)

#define PARSE_SERVER    "api.parse.com"
#define HTTPS_PORT      ( 443 )
#define PUSH_SERVER     "push.parse.com"
#define PUSH_PORT       ( 8253 )
#define PUSH_TIMEOUT_MS ( 10000 )

#define PARSE_SUCCESS    0
#define PARSE_ERROR      1

#define MAX_CLIENTS      5

#define RECEIVE_BUFFER_SIZE     1024

static const char parse_pem_certificate[] =
        "-----BEGIN CERTIFICATE-----\n"\
        "MIIEsTCCA5mgAwIBAgIQBOHnpNxc8vNtwCtCuF0VnzANBgkqhkiG9w0BAQsFADBs\n"\
        "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"\
        "d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n"\
        "ZSBFViBSb290IENBMB4XDTEzMTAyMjEyMDAwMFoXDTI4MTAyMjEyMDAwMFowcDEL\n"\
        "MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n"\
        "LmRpZ2ljZXJ0LmNvbTEvMC0GA1UEAxMmRGlnaUNlcnQgU0hBMiBIaWdoIEFzc3Vy\n"\
        "YW5jZSBTZXJ2ZXIgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC2\n"\
        "4C/CJAbIbQRf1+8KZAayfSImZRauQkCbztyfn3YHPsMwVYcZuU+UDlqUH1VWtMIC\n"\
        "Kq/QmO4LQNfE0DtyyBSe75CxEamu0si4QzrZCwvV1ZX1QK/IHe1NnF9Xt4ZQaJn1\n"\
        "itrSxwUfqJfJ3KSxgoQtxq2lnMcZgqaFD15EWCo3j/018QsIJzJa9buLnqS9UdAn\n"\
        "4t07QjOjBSjEuyjMmqwrIw14xnvmXnG3Sj4I+4G3FhahnSMSTeXXkgisdaScus0X\n"\
        "sh5ENWV/UyU50RwKmmMbGZJ0aAo3wsJSSMs5WqK24V3B3aAguCGikyZvFEohQcft\n"\
        "bZvySC/zA/WiaJJTL17jAgMBAAGjggFJMIIBRTASBgNVHRMBAf8ECDAGAQH/AgEA\n"\
        "MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIw\n"\
        "NAYIKwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2Vy\n"\
        "dC5jb20wSwYDVR0fBEQwQjBAoD6gPIY6aHR0cDovL2NybDQuZGlnaWNlcnQuY29t\n"\
        "L0RpZ2lDZXJ0SGlnaEFzc3VyYW5jZUVWUm9vdENBLmNybDA9BgNVHSAENjA0MDIG\n"\
        "BFUdIAAwKjAoBggrBgEFBQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQ\n"\
        "UzAdBgNVHQ4EFgQUUWj/kK8CB3U8zNllZGKiErhZcjswHwYDVR0jBBgwFoAUsT7D\n"\
        "aQP4v0cB1JgmGggC72NkK8MwDQYJKoZIhvcNAQELBQADggEBABiKlYkD5m3fXPwd\n"\
        "aOpKj4PWUS+Na0QWnqxj9dJubISZi6qBcYRb7TROsLd5kinMLYBq8I4g4Xmk/gNH\n"\
        "E+r1hspZcX30BJZr01lYPf7TMSVcGDiEo+afgv2MW5gxTs14nhr9hctJqvIni5ly\n"\
        "/D6q1UEL2tU2ob8cbkdJf17ZSHwD2f2LSaCYJkJA69aSEaRkCldUxPUd1gJea6zu\n"\
        "xICaEnL6VpPX/78whQYwvwt/Tv9XBZ0k7YXDK/umdaisLRbvfXknsuvCnQsH6qqF\n"\
        "0wGjIChBWUMo0oHjqvbsezt3tkBigAVBRQHvFwY+3sAzm2fTYS5yh+Rp/BIAV0Ae\n"\
        "cPUeybQ=\n"\
        "-----END CERTIFICATE-----\n";

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

static wiced_result_t client_connected_callback   ( wiced_tcp_socket_t* socket, void* arg );
static int            init_parse_socket           ( parse_client_t* client, wiced_tcp_socket_t* socket );
static wiced_result_t client_disconnected_callback( wiced_tcp_socket_t* socket, void* arg );
static wiced_result_t connect_to_push_socket      ( parse_client_t* client );
static wiced_result_t receive_data                ( wiced_tcp_socket_t* socket, char* data, uint16_t data_size, uint16_t timeout );
static wiced_result_t write_data                  ( wiced_tcp_socket_t* socket, const char* data, uint16_t data_size );
static wiced_result_t send_keep_alive             ( parse_client_t* client );
static void           parseSendRequestInternal    ( parse_client_t* client, const char* httpVerb, const char* httpPath, const char* httpRequestBody, parse_request_callback_t callback, int addInstallationHeader );
static int            sendRequest                 ( parse_client_t* parseClient, const char* host, const char* httpVerb, const char* httpRequestBody, int addInstallationHeader );
static short          socketSslConnectAndSend     ( parse_client_t* client, const char* host, unsigned short port );
static int            buildRequestHeaders         ( parse_client_t* parseClient, const char* host, const char* httpVerb, const char* httpRequestBody, int addInstallationHeader );
static void           createNewInstallationId     ( parse_client_t* parseClient );
static void           getInstallation             ( parse_client_t* client );
static void           getInstallationByIdCallback ( parse_client_t* client, int error, int httpStatus, const char* httpResponseBody );
static void           createInstallation          ( parse_client_t* client );
static void           createInstallationCallback  ( parse_client_t* client, int error, int httpStatus, const char* httpResponseBody );
static void           getInstallationCallback     ( parse_client_t* client, int error, int httpStatus, const char* httpResponseBody );

/******************************************************
 *               Variable Definitions
 ******************************************************/

//parse_push_callback_t s_push_callback = NULL;
static wiced_interface_t net_iface = WICED_STA_INTERFACE;

static wiced_tcp_socket_t tcp_server_socket;

static char received_keepalive_buffer[1024];
static char push_notification_buffer [2048];
static char json_data_buffer         [1024];
static char received_data_buffer     [2048];
static char sending_data_buffer      [1024];

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t parse_init( parse_client_t* client, const char* application_id, const char* client_key, const char* installation_id )
{
    wiced_result_t result;

    memset( client, 0, sizeof( *client ) );

    strncpy( client->app_id,     application_id, sizeof( client->app_id ) );
    strncpy( client->client_key, client_key,     sizeof( client->client_key ) );

    result = wiced_tls_init_root_ca_certificates( parse_pem_certificate );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    if ( strlen( installation_id) == 0 )
    {
        /* Generate Installation ID */
        createInstallation( client );
    }
    else
    {
        /* Set Installation ID */
        parse_set_installation_id( client, installation_id );
    }

    return WICED_SUCCESS;
}

/* Start and stop the push service. This opens or tears down the socket to push.parse.com (http://push.parse.com/) (http://push.parse.com/)
   Only one socket is opened, no matter how many times this is called.
   Alternative approach - we start/stop when callback is set cleared
 */
int parse_start_push_service( parse_client_t* client )
{
    if ( client->installation_id[ 0 ] == 0 )
    {
        return PARSE_ERROR;
    }

    WPRINT_APP_INFO( ("parseStartPushService.\r\n") );
    if ( connect_to_push_socket( client ) != WICED_SUCCESS )
    {
        return PARSE_ERROR;
    }

    client->push_socket_stop = 0;

    return PARSE_SUCCESS;
}

void parse_stop_push_service( parse_client_t* client )
{
    if ( client->installation_id[ 0 ] == 0 )
    {
        return;
    }

    client->push_socket_stop = 1;
    wiced_tcp_disconnect( &client->tcp_socket );
}

void parse_set_push_callback( parse_client_t* client, parse_push_callback_t push_callback )
{
    client->push_callback = push_callback;
}

/* Main event loop */
void parse_run_push_loop( parse_client_t* client )
{
    while ( 1 )
    {
        wiced_result_t result;

        memset( push_notification_buffer, 0, sizeof( push_notification_buffer ) );

        if ( client->push_socket_connected )
        {
            result = receive_data( &client->tcp_socket, push_notification_buffer, sizeof( push_notification_buffer ), PUSH_TIMEOUT_MS );
            if ( result == WICED_SUCCESS && client->push_callback != NULL )
            {
                client->push_callback( client, 0, push_notification_buffer );
            }
            else if ( result == WICED_TIMEOUT )
            {
                send_keep_alive( client );
            }
            else
            {
                client->push_socket_connected = 0;
                break;
            }
        }
    }

    WPRINT_APP_INFO( ("Parse socket disconnected\n") );
}

void parse_set_installation_id( parse_client_t* client, const char* installationId )
{
    if ( ( installationId != NULL ) && ( strlen( installationId ) > 0 ) )
    {
        if ( strncasecmp( client->installation_id, installationId, INSTALLATION_ID_MAX_LEN ) == 0 )
        {
            return;
        }

        strncpy( client->installation_id, installationId, INSTALLATION_ID_MAX_LEN );

        int i = 0;
        for ( i = 0; i < sizeof( client->installation_id ); i++ )
        {
            client->installation_id[ i ] = (char) tolower( (int) ( client->installation_id[ i ] ) );
        }

        getInstallation( client );
    }
    else
    {
        if ( strlen( client->installation_id ) == 0 )
        {
            return;
        }

        memset( client->installation_id, 0, sizeof( client->installation_id ) );
    }

    //saveClientState(parseClient);
}

const char* parse_get_installation_id( parse_client_t* client )
{
    if ( strlen( client->installation_id ) == 0 )
    {
        return NULL;
    }

    return client->installation_id;
}

static wiced_result_t client_connected_callback( wiced_tcp_socket_t* socket, void* arg )
{
    wiced_result_t      result;
    wiced_ip_address_t  ipaddr;
    uint16_t            port;

    UNUSED_PARAMETER( arg );

    /* Accept connection request */
    result = wiced_tcp_accept( &tcp_server_socket );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    /* Extract IP address and the Port of the connected client */
    wiced_tcp_server_peer( socket, &ipaddr, &port );

    WPRINT_LIB_INFO( ("Accepted connection from : ") );
    WPRINT_LIB_INFO ( ("IP %u.%u.%u.%u : %d\r\n", (unsigned char) ( ( GET_IPV4_ADDRESS(ipaddr) >> 24 ) & 0xff ),
                                                  (unsigned char) ( ( GET_IPV4_ADDRESS(ipaddr) >> 16 ) & 0xff ),
                                                  (unsigned char) ( ( GET_IPV4_ADDRESS(ipaddr) >>  8 ) & 0xff ),
                                                  (unsigned char) ( ( GET_IPV4_ADDRESS(ipaddr) >>  0 ) & 0xff ),
                                                  port ) );

    return result;
}

static int init_parse_socket( parse_client_t* client, wiced_tcp_socket_t* socket )
{
    int printed;
    int result;

    if ( &( client->tcp_socket ) != socket )
    {
        return WICED_SUCCESS;
    }

    memset( json_data_buffer, 0, sizeof(json_data_buffer) );
    memset( received_data_buffer, 0, sizeof(received_data_buffer) );

#ifdef USE_STREAM
    if (wiced_tcp_stream_init(&s_tcp_stream, &(client->tcp_socket)) != WICED_SUCCESS)
    {
        WPRINT_LIB_INFO(("Failed to initialize stream!\r\n"));
        return WICED_SUCCESS;
    }
#endif

    client->push_socket_connected = 1;

    printed = snprintf( json_data_buffer, sizeof(json_data_buffer),
                "{\"installation_id\":\"%s\", \"oauth_key\":\"%s\", \"v\": \"a1.4.1\", \"last\": null, \"ack_keep_alive\":true}\n",
                client->installation_id, client->app_id );

    result = write_data( &client->tcp_socket, json_data_buffer, (uint16_t) printed );

    result = receive_data( &client->tcp_socket, received_data_buffer, sizeof( received_data_buffer ), 10000 );

    return result;
}

static wiced_result_t client_disconnected_callback( wiced_tcp_socket_t* socket, void* arg )
{
    UNUSED_PARAMETER( arg );

    /* Start listening on the socket again */
    if ( wiced_tcp_listen( socket, TCP_SERVER_LISTEN_PORT ) != WICED_SUCCESS )
    {
        wiced_tcp_delete_socket( socket );
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

static wiced_result_t connect_to_push_socket( parse_client_t* client )
{
    wiced_ip_address_t ip_address;

    if ( client->installation_id[ 0 ] == 0 )
    {
        return WICED_ERROR;
    }

    if ( wiced_hostname_lookup( PUSH_SERVER, &ip_address, 30000 ) != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ("Cannot do DNS lookup for PARSE!\n") );
        return WICED_ERROR;
    }

    if ( wiced_tcp_create_socket( &( client->tcp_socket ), net_iface ) != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ("TCP socket creation failed\n") );
        return WICED_ERROR;
    }

    WPRINT_LIB_INFO( ("Created a push socket\n" ) );

    if ( wiced_tcp_register_callbacks( &( client->tcp_socket ), client_connected_callback, NULL, client_disconnected_callback, NULL ) != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ("wiced_tcp_register_callbacksfailed\n") );
        return WICED_ERROR;
    }

    if ( wiced_tcp_connect( &( client->tcp_socket ), &ip_address, PUSH_PORT, PUSH_TIMEOUT_MS ) != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ("TCP socket connection failed\n") );
        return WICED_ERROR;
    }
    else
    {
        WPRINT_LIB_INFO( ("TCP socket connected to %s:%u (%lu.%lu.%lu.%lu:%u)\n", PUSH_SERVER, PUSH_PORT, (ip_address.ip.v4 >> 24), (ip_address.ip.v4 >> 16) & 0xFF, (ip_address.ip.v4 >> 8) & 0xFF, ip_address.ip.v4 & 0xFF, PUSH_PORT) );
    }

    return init_parse_socket( client, &( client->tcp_socket ) );
}

static wiced_result_t receive_data( wiced_tcp_socket_t* socket, char* data, uint16_t data_size, uint16_t timeout )
{
    wiced_packet_t* data_packet = NULL;
    uint16_t        data_available;
    uint16_t        received_count;
    uint8_t*        received_data_buffer;
    wiced_result_t  result;

    result = wiced_tcp_receive( socket, &data_packet, timeout );
    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    wiced_packet_get_data( data_packet, 0, &received_data_buffer, &received_count, &data_available );
    memcpy( data, received_data_buffer, MIN( data_size, received_count ) );
    data[ MIN( data_size - 1, received_count ) ] = 0;
    result = wiced_packet_delete( data_packet );

    return result;
}

static wiced_result_t write_data(wiced_tcp_socket_t* socket, const char* data, uint16_t data_size)
{
    wiced_result_t result;

#ifdef USE_STREAM
    if (wiced_tcp_stream_write(&s_tcp_stream, data, data_size) != WICED_SUCCESS)
    {
        rc = WICED_ERROR;
        WPRINT_LIB_INFO(("Failed to send data to socket!\n"));
    }
    if (wiced_tcp_stream_flush(&s_tcp_stream) != WICED_SUCCESS)
    {
        rc = WICED_ERROR;
        WPRINT_LIB_INFO(("Failed to send flush data to socket!\n"));
    }
#else
    result = wiced_tcp_send_buffer(socket, data, data_size);
#endif

    if ( data_size > 10 )
    {
        WPRINT_LIB_INFO( ("Send to socket \"%s\"\n", data) );
    }

    return result;
}

static wiced_result_t send_keep_alive( parse_client_t* client )
{
    static const char data_keepalive[ ] = "{}\n";
    wiced_result_t    result;

    memset( received_keepalive_buffer, 0, sizeof( received_keepalive_buffer ) );

    result = write_data( &( client->tcp_socket ), data_keepalive, sizeof( data_keepalive ) - 1 );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ("Cannot write keep-alive data:%i\n", result) );
        return result;
    }

    result = receive_data( &( client->tcp_socket ), received_keepalive_buffer, sizeof( received_keepalive_buffer ), PUSH_TIMEOUT_MS );
    if ( result != WICED_SUCCESS )
    {
        WPRINT_LIB_INFO( ("Cannot read keep-alive back:%i\n", result) );
        return result;
    }

    if ( strncmp( data_keepalive, received_keepalive_buffer, 2 ) )
    {
        WPRINT_LIB_INFO( ("Unexpected response %s\n", data_keepalive) );
        if ( client->push_callback )
        {
            client->push_callback( client, 0, data_keepalive );
        }
    }
    return WICED_SUCCESS;
}

static void parseSendRequestInternal( parse_client_t* client, const char* httpVerb, const char* httpPath, const char* httpRequestBody, parse_request_callback_t callback, int addInstallationHeader )
{
    short status = sendRequest( client, httpPath, httpVerb, httpRequestBody, addInstallationHeader );

    if ( callback != NULL )
    {
        if ( status >= 0 )
        {
            callback( client, 0, getHttpResponseStatus( received_data_buffer ), getHttpResponseBody( received_data_buffer ) );
        }
        else
        {
            callback( client, status, -1, NULL );
        }
    }
}

static int sendRequest( parse_client_t* parseClient, const char* host, const char* httpVerb, const char* httpRequestBody, int addInstallationHeader )
{
    int status = 0;
    buildRequestHeaders( parseClient, host, httpVerb, httpRequestBody, addInstallationHeader );

    status = socketSslConnectAndSend( parseClient, PARSE_SERVER, HTTPS_PORT );

    return status;
}

static short socketSslConnectAndSend( parse_client_t* client, const char* host, unsigned short port )
{
    wiced_tcp_socket_t         socket;
    wiced_packet_t*            reply_packet;
    wiced_tls_simple_context_t context;
    wiced_ip_address_t         parse_ip_address;
    wiced_result_t             rx_result;
    wiced_result_t             result;
    uint8_t                    dns_retries = 0;

    wiced_tls_init_simple_context( &context, NULL );

    wiced_tcp_create_socket( &socket, WICED_STA_INTERFACE );
    wiced_tcp_enable_tls( &socket, &context );

    do
    {
        result = dns_client_hostname_lookup( PARSE_SERVER, &parse_ip_address, 5000 );
        ++dns_retries;
    } while ( result != WICED_SUCCESS && dns_retries < 4 );

    if ( dns_retries >= 4 )
    {
        WPRINT_LIB_INFO( ("Failed to do DNS lookup\n" ) );
        return result;
    }

    result = wiced_tcp_connect( &socket, &parse_ip_address, HTTPS_PORT, 20000 );
    if ( result != WICED_SUCCESS )
    {
        wiced_tcp_delete_socket( &socket );
        return result;
    }

    wiced_tcp_send_buffer( &socket, client->parse_buffer, (uint16_t) strlen( client->parse_buffer ) );

    WPRINT_LIB_INFO( ("waiting for HTTP reply\n") );

    do
    {
        rx_result = wiced_tcp_receive( &socket, &reply_packet, 5000 );

        if ( rx_result == WICED_SUCCESS )
        {
            http_status_code_t responseCode;

            if ( http_process_response( reply_packet, &responseCode ) == WICED_SUCCESS )
            {
                uint8_t* response;
                uint16_t response_length;
                uint16_t available_data_length;
                wiced_packet_get_data( reply_packet, 0, &response, &response_length, &available_data_length );
                memcpy( received_data_buffer, response, response_length );
            }
            wiced_packet_delete( reply_packet );
        }
    } while ( rx_result == WICED_SUCCESS );

    wiced_tcp_disconnect( &socket );
    wiced_tcp_delete_socket( &socket );

    return result;
}

static int buildRequestHeaders( parse_client_t* parseClient, const char* host, const char* httpVerb, const char* httpRequestBody, int addInstallationHeader )
{
    int status          = 0;
    int currentPosition = 0;
    int currentSize     = sizeof(sending_data_buffer) - currentPosition - 1;
    int isGetRequest    = strncasecmp(httpVerb, "GET", 3) == 0;
    int hasBody         = ( httpRequestBody != NULL ) && ( strlen( httpRequestBody ) > 0 );

    if ( isGetRequest != WICED_FALSE )
    {
        hasBody = WICED_FALSE;
    }

    memset( sending_data_buffer, 0, sizeof( sending_data_buffer ) );

    if ( isGetRequest != WICED_FALSE )
    {
        status = beginHttpGetRequest( sending_data_buffer + currentPosition, currentSize, host, httpVerb, httpRequestBody );
    }
    else
    {
        status = beginHttpRequest( sending_data_buffer + currentPosition, currentSize, host, httpVerb );
    }

    if ( status >= 0 )
    {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader( sending_data_buffer + currentPosition, currentSize, "Host", PARSE_SERVER );
    }

    if ( status >= 0 )
    {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader( sending_data_buffer + currentPosition, currentSize, "User-Agent", "WICEDSDK" );
    }

    if ( status >= 0 )
    {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader( sending_data_buffer + currentPosition, currentSize, "X-Parse-OS-Version", "ThreadX" );
    }

    if ( status >= 0 )
    {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader( sending_data_buffer + currentPosition, currentSize, "X-Parse-Client-Version", "331" );
    }

    if ( status >= 0 )
    {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader( sending_data_buffer + currentPosition, currentSize, "X-Parse-Application-Id", parseClient->app_id );
    }

    if ( status >= 0 )
    {
        currentPosition += status;
        currentSize -= status;
        status = addHttpRequestHeader( sending_data_buffer + currentPosition, currentSize, "X-Parse-Client-Key", parseClient->client_key );
    }

    if ( addInstallationHeader )
    {
        if ( status >= 0 )
        {
            currentPosition += status;
            currentSize -= status;
            status = addHttpRequestHeader( sending_data_buffer + currentPosition, currentSize, "X-Parse-Installation-Id", parseClient->installation_id );
        }
    }

    if ( hasBody )
    {
        if ( status >= 0 )
        {
            currentPosition += status;
            currentSize -= status;
            status = addHttpRequestHeader( sending_data_buffer + currentPosition, currentSize, "Content-Type", "application/json; charset=utf-8" );
        }

        if ( status >= 0 )
        {
            currentPosition += status;
            currentSize -= status;
            status = addHttpRequestHeaderInt( sending_data_buffer + currentPosition, currentSize, "Content-Length", strlen( httpRequestBody ) );
        }

        if ( status >= 0 )
        {
            currentPosition += status;
            currentSize -= status;
            status = snprintf( sending_data_buffer + currentPosition, currentSize, "\r\n" );
        }

        if ( status >= 0 )
        {
            currentPosition += status;
            currentSize -= status;
            status = snprintf( sending_data_buffer + currentPosition, currentSize, "%s", httpRequestBody );
        }
    }
    else
    {
        if ( status >= 0 )
        {
            currentPosition += status;
            currentSize -= status;
            status = snprintf( sending_data_buffer + currentPosition, currentSize, "\r\n" );
        }
    }

    if ( status >= 0 )
    {
        currentPosition += status;
        currentSize -= status;
        sending_data_buffer[ currentPosition ] = 0;
    }
    snprintf( parseClient->parse_buffer, RESPONSE_SIZE, "%s", sending_data_buffer );

    return ( status < 0 ) ? status : currentPosition;
}

static void createNewInstallationId( parse_client_t* parseClient )
{
    uuid_create( (uuid_t*)parseClient->installation_id );
}


static void getInstallation( parse_client_t* client )
{
    char content[ 150 ];

    // If we have both installation id and installation object id, don't do anything
    if ( ( strlen( client->installationObjectId ) > 0 ) && ( strlen( client->installation_id ) > 0 ) )
    {
        return;
    }

    // We have only installation object id or installation id
    if ( strlen( client->installationObjectId ) > 0 )
    {
        // This is just in case, we should never get in this branch in normal scenarios
        // as the device app will always give us installation id, and never installation object id
        snprintf( content, sizeof( content ) - 1, "/1/installations/%s", client->installationObjectId );

        parseSendRequestInternal( (parse_client_t*) client, "GET", content, NULL, getInstallationCallback, WICED_FALSE );
    }
    else if ( strlen( client->installation_id ) > 0 )
    {
        snprintf( content, sizeof( content ) - 1, "where=%%7b%%22installationId%%22%%3a+%%22%s%%22%%7d", client->installation_id );

        parseSendRequestInternal( (parse_client_t*) client, "GET", "/1/installations", content, getInstallationByIdCallback, WICED_FALSE );
    }

    // Go through create new installation, to catch the case we still don't have
    // an installation object id. This will be noop if we managed to get the installation
    // already
    createInstallation( client );
}

static void getInstallationByIdCallback( parse_client_t* client, int error, int httpStatus, const char* httpResponseBody )
{
    if ( ( error == 0 ) && ( httpStatus >= 200 && httpStatus < 300 ) )
    {
        char queryResults[ 400 ];
        memset( queryResults, 0, sizeof( queryResults ) );
        short status = simpleJsonProcessor( httpResponseBody, "results", queryResults, sizeof( queryResults ) );
        if ( ( status != 0 ) && ( queryResults[ 1 ] == '{' ) )
        {
            simpleJsonProcessor( queryResults + 1, "objectId", client->installationObjectId, sizeof( client->installationObjectId ) );
            simpleJsonProcessor( queryResults + 1, "installationId", client->installation_id, sizeof( client->installation_id ) );
            WPRINT_LIB_INFO( ("[Parse] Installation object id: %s.\r\n", client->installationObjectId) );
            WPRINT_LIB_INFO( ("[Parse] Installation id: %s.\r\n", client->installation_id) );
        }
    }
    else
    {
        WPRINT_LIB_INFO( ("[Parse] Failed to get installation. Error: %d, HTTP status: %d\r\n", error, httpStatus) );
        memset( client->installationObjectId, 0, sizeof( client->installationObjectId ) );
    }
}

static void createInstallation( parse_client_t* client )
{
    // If we have an installation object id, we don't need to create it again
    if ( strlen( client->installationObjectId ) == 0 )
    {
        WPRINT_LIB_INFO( ("[Parse] Creating new installation.\r\n") );

        // Create new id if necessary
        if ( strlen( client->installation_id ) == 0 )
        {
            createNewInstallationId( client );
        }

        // Send installation create request and get the object id from the response.
        // If the response is a failure, set the instalaltionObjectId back to empty
        char content[ 120 ];
        snprintf( content, sizeof( content ) - 1, "{\"installationId\": \"%s\", \"deviceType\": \"embedded\", \"parseVersion\": \"1.0.0\"}", client->installation_id );

        parseSendRequestInternal( (parse_client_t*) client, "POST", "/1/installations", content, createInstallationCallback, WICED_FALSE );
    }
}

static void createInstallationCallback( parse_client_t* client, int error, int httpStatus, const char* httpResponseBody )
{
    if ( ( error == 0 ) && ( httpStatus >= 200 && httpStatus < 300 ) )
    {
        simpleJsonProcessor( httpResponseBody, "objectId", client->installationObjectId, sizeof( client->installationObjectId ) );
        WPRINT_LIB_INFO( ("[Parse] Installation object id: %s.\r\n", client->installationObjectId) );
    }
    else
    {
        WPRINT_LIB_INFO( ("[Parse] Failed to create installation. Error: %d, HTTP status: %d\r\n", error, httpStatus) );
        memset( client->installationObjectId, 0, sizeof( client->installationObjectId ) );
    }
}

static void getInstallationCallback( parse_client_t* client, int error, int httpStatus, const char* httpResponseBody )
{
    if ( ( error == 0 ) && ( httpStatus >= 200 && httpStatus < 300 ) )
    {
        simpleJsonProcessor( httpResponseBody, "objectId", client->installationObjectId, sizeof( client->installationObjectId ) );
        simpleJsonProcessor( httpResponseBody, "installationId", client->installation_id, sizeof( client->installation_id ) );
        WPRINT_LIB_INFO( ("[Parse] Installation object id: %s.\r\n", client->installationObjectId) );
        WPRINT_LIB_INFO( ("[Parse] Installation id: %s.\r\n", client->installation_id) );
    }
    else
    {
        WPRINT_LIB_INFO( ("[Parse] Failed to get installation. Error: %d, HTTP status: %d\r\n", error, httpStatus) );
        memset( client->installationObjectId, 0, sizeof( client->installationObjectId ) );
    }
}


