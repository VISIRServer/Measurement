/**** BEGIN LICENSE BLOCK ****
 * This file is a part of the VISIR(TM) (Virtual Systems in Reality)
 * Software package.
 * 
 * VISIR(TM) is used to open laboratories for remote operation and control
 * as a supplement and a complement to local use.
 * 
 * VISIR(TM) is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. No liability
 * can be imposed for any impact on any equipment by the software. See
 * the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **** END LICENSE BLOCK ****/

/*
 * Copyright (c) 2009 Johan Zackrisson
 * All Rights Reserved.
 */

#include "sendhttp.h"

#include "url.h"
#include "widestring.h"

#include <windows.h>
#include <winhttp.h>

#include <vector>

using namespace std;

#pragma comment(lib, "winhttp.lib")

HTTPRequest::HTTPRequest()
{
	mStatusCode = 0;
}

HTTPRequest::~HTTPRequest()
{
}

int HTTPRequest::Post(const std::string& urlstring, const std::string& payload)
{
	URL url;
	url.Set(Ascii2Wide(urlstring));

	DWORD dwStatusCode = 0;
//	DWORD dwSupportedSchemes;
//	DWORD dwFirstScheme;
//	DWORD dwSelectedScheme;
//	DWORD dwTarget;
	DWORD dwLastStatus = 0;
	DWORD dwSize = sizeof(DWORD);
	BOOL  bResults = FALSE;
	BOOL  bDone = FALSE;

	DWORD dwProxyAuthScheme = 0;
	HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;

	DWORD dwOpenFlags = 0;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen( L"OpenLabs Measureserver/1.0",  
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, 
		WINHTTP_NO_PROXY_BYPASS, 0 );

	INTERNET_PORT nPort = INTERNET_DEFAULT_HTTP_PORT;

	if (url.GetSchema() == L"https")
	{
		nPort = INTERNET_DEFAULT_HTTPS_PORT;
		dwOpenFlags = WINHTTP_FLAG_SECURE;
	}

	// Specify an HTTP server.
	if( hSession )
		hConnect = WinHttpConnect( hSession, 
		url.GetHostName().c_str(), 
		nPort, 0 );

	// Create an HTTP request handle.
	if( hConnect )
		hRequest = WinHttpOpenRequest( hConnect, 
		L"POST", 
		url.GetPath().c_str(),
		NULL, 
		WINHTTP_NO_REFERER, 
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		dwOpenFlags );

	// Continue to send a request until status code 
	// is not 401 or 407.
	if( hRequest == NULL )
		bDone = TRUE;

	while( !bDone )
	{
		//  If a proxy authentication challenge was responded to, reset
		//  those credentials before each SendRequest, because the proxy  
		//  may require re-authentication after responding to a 401 or  
		//  to a redirect. If you don't, you can get into a 
		//  407-401-407-401- loop.
		/*if( dwProxyAuthScheme != 0 )
			bResults = WinHttpSetCredentials( hRequest, 
			WINHTTP_AUTH_TARGET_PROXY, 
			dwProxyAuthScheme, 
			pGetRequest->szProxyUsername,
			pGetRequest->szProxyPassword,
			NULL );
			*/
		
		wstring wheaders = Ascii2Wide(mHeader);

		// Send a request.
		bResults = WinHttpSendRequest( hRequest,
			wheaders.c_str(),
			-1,
			(void*)payload.c_str(),
			payload.size(), 
			payload.size(), 
			0 );

		//ERROR_WINHTTP_CANNOT_CONNECT
		//WINHTTP_NO_REQUEST_DATA

		// End the request.
		if( bResults )
			bResults = WinHttpReceiveResponse( hRequest, NULL );

		// Resend the request in case of 
		// ERROR_WINHTTP_RESEND_REQUEST error.
		if( !bResults && GetLastError( ) == ERROR_WINHTTP_RESEND_REQUEST)
			continue;

		// Check the status code.
		if( bResults ) 
			bResults = WinHttpQueryHeaders( hRequest, 
			WINHTTP_QUERY_STATUS_CODE |
			WINHTTP_QUERY_FLAG_NUMBER,
			NULL, 
			&dwStatusCode, 
			&dwSize, 
			NULL );

		// read the result, independent of result code
		DWORD dwSize = 0;
		if (bResults) do
		{
			dwSize = 0;
			bResults = WinHttpQueryDataAvailable( hRequest, &dwSize);
			if (bResults && dwSize > 0)
			{
				DWORD dwDownloaded = 0;
				vector<char> data(dwSize);
				bResults = WinHttpReadData( hRequest, (LPVOID)&data[0], dwSize, &dwDownloaded);
				if (bResults)
				{
					mResponse.insert(mResponse.end(), data.begin(), data.end());
				}
			}

		} while(dwSize > 0 && !bResults);

		if( bResults )
		{
			mStatusCode = dwStatusCode;

			//handleStatusCode:
			switch( dwStatusCode )
			{
			case 200: 
				// The resource was successfully retrieved.
				// You can use WinHttpReadData to read the 
				// contents of the server's response.
				//printf( "The resource was successfully retrieved.\n" );
				bDone = TRUE;
				break;

			case 401:
				bDone = TRUE;
				/*
				// The server requires authentication.
				printf(" The server requires authentication. Sending credentials...\n" );

				// Obtain the supported and preferred schemes.
				bResults = WinHttpQueryAuthSchemes( hRequest, 
					&dwSupportedSchemes, 
					&dwFirstScheme, 
					&dwTarget );

				// Set the credentials before resending the request.
				if( bResults )
				{
					dwSelectedScheme = ChooseAuthScheme( dwSupportedSchemes);

					if( dwSelectedScheme == 0 )
						bDone = TRUE;
					else
						bResults = WinHttpSetCredentials( hRequest, 
						dwTarget, 
						dwSelectedScheme,
						pGetRequest->szServerUsername,
						pGetRequest->szServerPassword,
						NULL );
				}

				// If the same credentials are requested twice, abort the
				// request.  For simplicity, this sample does not check
				// for a repeated sequence of status codes.
				if( dwLastStatus == 401 )
					bDone = TRUE;
					*/

				break;

			case 407:
				// The proxy requires authentication.
				//printf( "The proxy requires authentication.  Sending credentials...\n" );
				bDone = TRUE;
				/*
				// Obtain the supported and preferred schemes.
				bResults = WinHttpQueryAuthSchemes( hRequest, 
					&dwSupportedSchemes, 
					&dwFirstScheme, 
					&dwTarget );

				// Set the credentials before resending the request.
				if( bResults )
					dwProxyAuthScheme = ChooseAuthScheme(dwSupportedSchemes);

				// If the same credentials are requested twice, abort the
				// request.  For simplicity, this sample does not check 
				// for a repeated sequence of status codes.
				if( dwLastStatus == 407 )
					bDone = TRUE;
				*/
				break;

			default:
				// The status code does not indicate success.
				//printf("Error. Status code %d returned.\n", dwStatusCode);
				bDone = TRUE;
			}
		}

		// Keep track of the last status code.
		dwLastStatus = dwStatusCode;

		// If there are any errors, break out of the loop.
		if( !bResults ) 
			bDone = TRUE;
	}

	// Report any errors.
	if( !bResults )
	{
		DWORD dwLastError = GetLastError( );
		printf( "Error %d has occurred.\n", dwLastError );
	}

	// Close any open handles.
	if( hRequest ) WinHttpCloseHandle( hRequest );
	if( hConnect ) WinHttpCloseHandle( hConnect );
	if( hSession ) WinHttpCloseHandle( hSession );


	return 1;
}

void HTTPRequest::SetHeaders(const std::string& header)
{
	mHeader = header;
}