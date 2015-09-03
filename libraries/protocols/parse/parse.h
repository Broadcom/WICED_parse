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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/*! \def APPLICATION_ID_MAX_LEN
 *  \brief The length of application id
 */
#define APPLICATION_ID_MAX_LEN      ( 40 )

/*! \def CLIENT_KEY_MAX_LEN
 *  \brief The length of client key
 */
#define CLIENT_KEY_MAX_LEN          ( 40 )

/*! \def OBJECT_ID_MAX_LEN
 *  \brief The length of object id
 */
#define OBJECT_ID_MAX_LEN           ( 16 )

/*! \def INSTALLATION_ID_MAX_LEN
 *  \brief The length of installation id
 */
#define INSTALLATION_ID_MAX_LEN     ( 36 )

/*! \def SESSION_TOKEN_MAX_LEN
 *  \brief The length of session token
 */
#define SESSION_TOKEN_MAX_LEN       ( 40 )

#define RESPONSE_SIZE               ( 2048 )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct _parse_client_t parse_client_t;

/*! \typedef parse_request_callback_t
 *  \brief Callback for API requests.
 *
 *  Called when an API request is finished.
 *
 *  \param[in]  client           The Parse client that made the API request.
 *  \param[in]  error            OS-specific error code.
 *                               If the API request was successful, this will be 0.
 *                               If any error occured during the request, the rest of the
 *                               parameters are meaningless.
 *  \param[in]  httpStatus       HTTP status of the API request.
 *  \param[in]  httpResponseBody The response body for the request. If the request
 *                               is successful, this is a JSON document describing the
 *                               result. If the request is unsuccessful, this contains
 *                               error details.
 *
 *  The SDK retains ownership of the response body. If you need to retain any data from it
 *  for use outside of the scope of the callback, make a copy.
 */
typedef void (*parse_request_callback_t)( parse_client_t* client, int error, int httpStatus, const char* httpResponseBody );


/*! \typedef parse_push_callback_t
 *  \brief Callback for push notifications and errors from the push service.
 *
 *  \param[in]  client           The Parse client associated with the callback.
 *  \param[in]  error            OS-specific error code.
 *                               If the push service received the notification data
 *                               normally, this will be 0.
 *                               If any error occurred during receiving
 *                               the notification data, the rest of the parameters are
 *                               meaningless.
 *  \param[in]  data             The data for the incoming push notification. If the notification
 *                               is received successfully, this is a JSON document describing the
 *                               payload for the notification.
 *
 *  The SDK retains ownership of the notification data buffer. If you need to retain any data from it
 *  for use outside of the scope of the callback, make a copy.
 */
typedef void (*parse_push_callback_t)( parse_client_t* client, int error, const char* data );

/******************************************************
 *                    Structures
 ******************************************************/

struct _parse_client_t
{
    char                   app_id                [ APPLICATION_ID_MAX_LEN    + 1];
    char                   client_key            [ CLIENT_KEY_MAX_LEN        + 1];
    char                   session_token         [ SESSION_TOKEN_MAX_LEN     + 1];
    char                   installation_id       [ INSTALLATION_ID_MAX_LEN   + 1];
    char                   installation_id_string[ INSTALLATION_ID_MAX_LEN*2 + 1];
    char                   installationObjectId  [ OBJECT_ID_MAX_LEN         + 1];
    parse_push_callback_t  push_callback;
    wiced_tcp_socket_t     tcp_socket;
    volatile int           push_socket_connected;
    volatile int           push_socket_stop;
    char                   parse_buffer[ RESPONSE_SIZE ];
#ifdef USE_STREAM
    wiced_tcp_stream_t     tcp_stream;
#endif
};

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

/*! \fn wiced_result_t parse_init( parse_client_t* client, const char* application_id, const char* client_key, const char* installation_id )
 *  \brief Initialize the Parse client and user session
 *
 *  This method only initializes the Parse client, used for subsequent API requests. It does
 *  not start the push service, and does not create or load the installation object for the client.
 *
 *  \param[in]  client            A pointer to a parse_client_t object          (required)
 *  \param[in]  application_id    The application id for the Parse application. (required)
 *  \param[in]  client_key        The client API key for the Parse application. (required)
 *  \param[in]  installation_id   The installation ID to be used by the client  (optional)
 *
 *  \result                       wiced_result_t
 *
 *  The caller retains ownership of the applicationId and clientKey buffers, and is responsible for
 *  freeing them and reclaiming the memory after this call.
 *  The SDK will make copies of the buffers.
 */
wiced_result_t parse_init( parse_client_t* client, const char* application_id, const char* client_key, const char* installation_id );

/*! \fn void parse_set_installation_id( parse_client_t* client, const char* installationId )
 *  \brief Set the installation object id for this client.
 *
 *  Set the installation object id for this client. If this is not called before making an
 *  API request or starting push service, the SDK will create a new installation object for
 *  the client.
 *
 *  \param[in]  client           The Parse client for which the installation id will be set.
 *  \param[in]  installationId   The installation id to be used by the client.
 *
 *  The caller retains ownership of the installationId buffer, and is responsible for
 *  freeing it and reclaiming the memory after this call.
 *  The SDK will make copies of the buffer.
 */
void parse_set_installation_id( parse_client_t* client, const char* installationId );

/*! \fn const char* parse_get_installation_id( parse_client_t* client )
 *  \brief Return the client installation id
 *
 *  Return the installation id for the client. If called before the installation id for the
 *  client was set, this method will NOT create a new installation object.
 *  The typical usage scenario is:
 *
 *  1.   Call parseInitialize().
 *  2.   Call parse_get_installation_id().
 *  3.a. If the result is not NULL, the SDK has a previous installation id,
 *       there's nothing else to do.
 *       Otherwise:
 *  3.b.1. If the device has already an installation id, call parse_set_installation_id().
 *  3.b.2. If the device doesn't have an installation id, the SDK will generate one
 *         the first time it needs it.
 *
 *  \param[in]  client           The Parse client for which the installation id should be returned.
 *
 *  \result                      The installation id.
 *
 *  The SDK retains ownership of the result buffer, and still owns it after
 *  this call. Do not free it.
 */
const char* parse_get_installation_id( parse_client_t* client );

/*! \fn void parse_set_session_token( parse_client_t* client, const char* sessionToken )
 *  \brief Set the session token for the Parse client
 *
 *  \param[in]  client           The Parse client for which the session token is set.
 *  \param[in]  sessionToken     The new session token. All subsequent API calls will
 *                               use this token for authentication and authorization.
 *                               If this is NULL, the client will be unauthenticated.
 *
 *  The caller retains ownership of the sessionToken buffer, and is responsible for
 *  freeing it and reclaiming the memory after this call.
 *  The SDK will make copies of the buffer.
 */
void parse_set_session_token( parse_client_t* client, const char* sessionToken );

/*! \fn void parseClearSessionToken(ParseClient client)
 *  \brief Clear the session token
 *
 *  Same as parse_set_session_token(client, NULL);
 */
void parseClearSessionToken( parse_client_t* client );

/*! \fn const char* parse_get_session_token( parse_client_t* client )
 *  \brief Return the client session token.
 *
 *  Return the session token for the client, or NULL if there isn't one.
 *
 *  \param[in]  client           The Parse client for which the session token should be returned.
 *
 *  \result                      The session token.
 *
 *  The SDK retains ownership of the result buffer, and still owns it after
 *  this call. Do not free it.
 */
const char* parse_get_session_token( parse_client_t* client );

/*! \fn void parse_set_push_callback( parse_client_t* client, parse_push_callback_t callback )
 *  \brief Set the callback for push notifications and errors
 *
 *  This method sets or clears the callback for push notifications and errors from
 *  the push service, associated with this client.
 *
 *  This method DOES NOT start or stop the push service.
 *
 *  \param[in]  client           The Parse client for which the callback is set.
 *  \param[in]  callback         The new callback method. All subsequent push notifications
 *                               or errors will result  in a call to this callback method.
 *                               If this parameter is NULL, the callback will be removed.
 *
 *  Any push notifications received while there is no callback associated with the client
 *  will be skipped, and the application will not received them.
 */
void parse_set_push_callback( parse_client_t* client, parse_push_callback_t callback );

/*! \fn int parse_start_push_service( parse_client_t* client )
 *  \brief Start the push notifications service.
 *
 *  This method will start the push notification service and will listen for incoming
 *  push notifications. If the push service is already started, this will do nothing.
 *  To actually process incoming push notifications, it is still necessary to repeatedly
 *  call parse_process_next_push_notification() or call parseRunPushLoop().
 *
 *  \param[in]  client           The Parse client for which the service should be started.
 *
 *  \result                      OS-specific error if the push can't be started or 0 if
 *                               started successfully.
 *
 *  If a push callback has been set for the client, any errors during starting the service will
 *  passed to it as well.
 *
 */
int parse_start_push_service( parse_client_t* client );

/*! \fn void parse_stop_push_service( parse_client_t* client )
 *  \brief Stop the push notifications service.
 *
 *  This method will stop the push notification service. If the push service is not started,
 *  this will do nothing.
 *
 *  \param[in]  client           The Parse client for which the service should be stopped.
 */
void parse_stop_push_service( parse_client_t* client );

/*! \fn int parse_process_next_push_notification( parse_client_t* client )
 *  \brief Process next pending push notification.
 *
 *  Push notifications are processed one at a time. This method will process the next push
 *  notification and will call the client callback, if one is set.
 *
 *  \param[in]  client           The Parse client for which the next event should be processed.
 *
 *  \result                      0 if there are no more pending notification events or the push
 *                               service is not started.
 *                               Positive number if there are more pending notification events.
 *
 *  If the push notifications callback has been set for the client, it will also be called in
 *  the case of an error during the processing of the notification. If the callback is not set,
 *  the client will not get notified about any errors that during the processing of the notification.
 */
int parse_process_next_push_notification( parse_client_t* client );

/*! \fn void parseRunPushLoop(ParseClient client)
 *  \brief Process push notifications events in a loop.
 *
 *  This method will keep processing the push notification events for the client and will call
 *  the client callback for each one. If the push notification service for the client is not
 *  started or is stopped, this method will exit.
 *
 *  \param[in]  client           The Parse client for which the push events should be processed.
 */
void parse_run_push_loop( parse_client_t* client );

/*! \fn void parse_send_request( parse_client_t* client, const char* httpVerb, const char* httpPath, const char* httpRequestBody, parse_request_callback_t callback )
 *  \brief Send an API request.
 *
 *  This method makes an API request for the client. If called before installation id for the client
 *  is set, this will also create a new installation object.
 *
 *  The API requests supported are any valid requests that can be made through the REST API as well.
 *
 *  \param[in]  client           The Parse client for which the request is made.
 *  \param[in]  httpVerb         The type of request - POST, GET, PUT, DELETE
 *  \param[in]  httpPath         The path for the request, i.e. /1/classes/MyClass
 *  \param[in]  httpRequestBody  The JSON payload for the request
 *  \param[in]  callback         The callback to process the result  of the request.
 *
 *  The caller retains ownership of the httpVerb, httpPath, and requestBody buffers, and is responsible for
 *  freeing them and reclaiming the memory after this call.
 */
void parse_send_request( parse_client_t* client, const char* httpVerb, const char* httpPath, const char* httpRequestBody, parse_request_callback_t callback );

/*! \fn int parse_get_error_code( const char* httpResponseBody )
 *  \brief Extract Parse error code.
 *
 *  Helper function to extract Parse errors from the response body, when httpStatus of the response is 4xx or 5xx.
 *  TODO: Link to the public documentation of the Parse error codes
 *
 *  \param[in]  httpResponseBody The response body for the request.
 *
 *  \result                      Return the error code from the server.
 *
 * The caller retains ownership of the httpResponseBody buffer, and is responsible for
 * freeing it and reclaiming the memory after this call.
 */
int parse_get_error_code( const char* httpResponseBody );


#ifdef __cplusplus
} /* extern "C" */
#endif

