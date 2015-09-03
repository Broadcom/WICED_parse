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

/**
 * Simple implementation of HTTP response status and body parsing
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "http.c"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

// We always request HTTP 1.1, so Parse should always response with 'HTTP/1.1 '
// Sadly, this can be broken in the rare case of an intermediate proxy which
// doesn't understand HTTP 1.1.
#define HTTP_HEADERS_BEGIN "HTTP/1.1 "
static short magicOffset = 9;

// As per HTTP RFC, HTTP headers finish with two CR LF in a row
// Order matters, and is dfefined in the RFC
#define HTTP_HEADERS_END "\r\n\r\n"
static short magicOffset2 = 4;

// HTTP Status codes are always three digits
static short magicOffset3 = 3;

/******************************************************
 *               Function Definitions
 ******************************************************/

int addHttpRequestLine( char* httpRequest, unsigned int httpRequestSize, const char* format, ... )
{
    if ( httpRequest == NULL )
    {
        return -1;
    }

    va_list list;
    va_start( list, format );
    int status = vsnprintf( httpRequest, httpRequestSize, (char*) format, list );
    va_end( list );

    if ( status >= httpRequestSize )
    {
        // Not enough space in the buffer :-(
        status = -1;
    }

    return status;
}

int beginHttpRequest( char* httpRequest, unsigned int httpRequestSize, const char* host, const char* httpVerb )
{
    return addHttpRequestLine( httpRequest, httpRequestSize, "%s %s HTTP/1.1\r\n", httpVerb, host );
}

int beginHttpGetRequest( char* httpRequest, unsigned int httpRequestSize, const char* host, const char* httpVerb, const char* httpQuery )
{
    if ( ( httpQuery != NULL ) && ( strlen( httpQuery ) > 0 ) )
    {
        return addHttpRequestLine( httpRequest, httpRequestSize, "%s %s?%s HTTP/1.1\r\n", httpVerb, host, httpQuery );
    }

    return addHttpRequestLine( httpRequest, httpRequestSize, "%s %s HTTP/1.1\r\n", httpVerb, host );
}

int addHttpRequestHeader( char* httpRequest, unsigned int httpRequestSize, const char* httpHeader, const char* httpRequestHeaderValue )
{
    if ( strlen( httpRequestHeaderValue ) == 0 )
    {
        return 0;
    }

    return addHttpRequestLine( httpRequest, httpRequestSize, "%s: %s\r\n", httpHeader, httpRequestHeaderValue );
}

int addHttpRequestHeaderInt( char* httpRequest, unsigned int httpRequestSize, const char* httpHeader, int httpRequestHeaderValue )
{
    return addHttpRequestLine( httpRequest, httpRequestSize, "%s: %d\r\n", httpHeader, httpRequestHeaderValue );
}

int getHttpResponseStatus( const char* httpResponse )
{
    char httpStatus[ 4 ] = { 0 };
    memcpy( httpStatus, httpResponse + magicOffset, magicOffset3 );

    return atoi( httpStatus );
}

const char* getHttpResponseBody( const char* httpResponse )
{
    /* If we don't find the header terminator, that means there's no body, so passing NULL or empty string to callback is ok */
    char* httpResponseBody = strstr( httpResponse, HTTP_HEADERS_END );
    if ( ( httpResponseBody != NULL ) && ( strlen( httpResponseBody ) >= magicOffset2 ) )
    {
        httpResponseBody += magicOffset2;
    }

    return httpResponseBody;
}
