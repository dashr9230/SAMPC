//----------------------------------------------------
//
//	HTTP/1.0 Client Procedures.
//	(c) 2002-2005 Kye Bitossi
//
//  Basic web document fetcher.
//
//  Version: $Id: httpclient.cpp,v 1.4 2006/05/03 17:32:37 kyeman Exp $
//
//----------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
# include <windows.h>
#else
# include <errno.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

#include "httpclient.h"
#include "runutil.h"

//----------------------------------------------------

CHttpClient::CHttpClient()
{
	memset(&m_Request,0,sizeof(HTTP_REQUEST));
	memset(&m_Response,0,sizeof(HTTP_RESPONSE));
	m_iError = HTTP_SUCCESS; // Request is successful until otherwise indicated
	m_iSocket = (-1);

// Winsock init
#ifdef WIN32
	WORD				wVersionRequested;
	WSADATA				wsaData;
	wVersionRequested = MAKEWORD(2,2);
	WSAStartup(wVersionRequested,&wsaData);
#endif
}

//----------------------------------------------------

CHttpClient::~CHttpClient()
{
// Winsock cleanup
#ifdef WIN32
	WSACleanup();
#endif
}

//----------------------------------------------------

int CHttpClient::ProcessURL(int iType, char *szURL, char *szPostData, char *szReferer)
{
	InitRequest(iType,szURL,szPostData,szReferer);

	Process();

	return m_iError;
}

//----------------------------------------------------

bool CHttpClient::GetHeaderValue(char *szHeaderName,char *szReturnBuffer, int iBufSize)
{
	char *szHeaderStart;
	char *szHeaderEnd;
	int iLengthSearchHeader = strlen(szHeaderName);
	int iCopyLength;

	szHeaderStart = Util_stristr(m_Response.header,szHeaderName);
	if(!szHeaderStart) {
		return false;
	}
	szHeaderStart+=iLengthSearchHeader+1;

	szHeaderEnd = strchr(szHeaderStart,'\n');
	if(!szHeaderEnd) {
		szHeaderEnd = m_Response.header + strlen(m_Response.header); // (END OF STRING)
	}

	iCopyLength = szHeaderEnd - szHeaderStart;
	if(iBufSize < iCopyLength) {
		return false;
	}

	memcpy(szReturnBuffer,szHeaderStart,iCopyLength);
	szReturnBuffer[iCopyLength] = '\0';
	return true;
}

//----------------------------------------------------

bool CHttpClient::Connect(char *szHost, int iPort)
{
	struct sockaddr_in	sa;
	struct hostent		*hp;

	// Hostname translation
	if((hp=(struct hostent *)gethostbyname(szHost)) == NULL ) {
		m_iError=HTTP_ERROR_BAD_HOST;
		return false;
	}

	// Prepare a socket	
	memset(&sa,0,sizeof(sa));
	memcpy(&sa.sin_addr,hp->h_addr,hp->h_length);
	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons((unsigned short)iPort);

	if((m_iSocket=socket(AF_INET,SOCK_STREAM,0)) < 0) {
		m_iError=HTTP_ERROR_NO_SOCKET;
		return false;
	}

	// Try to connect
	if(connect(m_iSocket,(struct sockaddr *)&sa,sizeof sa) < 0)	{
		CloseConnection();
		m_iError=HTTP_ERROR_CANT_CONNECT;
		return false;
	}

	return true;
}

//----------------------------------------------------

void CHttpClient::CloseConnection()
{
#ifdef WIN32
	closesocket(m_iSocket);
#else
	close(m_iSocket);
#endif
}

//----------------------------------------------------

bool CHttpClient::Send(char *szData)
{	
	if(send(m_iSocket,szData,strlen(szData),0) < 0) {	
		m_iError = HTTP_ERROR_CANT_WRITE;
		return false;
	}
	return true;
}

//----------------------------------------------------

int CHttpClient::Recv(char *szBuffer, int iBufferSize)
{	
	return recv(m_iSocket,szBuffer,iBufferSize,0);
}

//----------------------------------------------------

void CHttpClient::InitRequest(int iType, char *szURL, char *szPostData, char *szReferer)
{
	char		 port[60];		// port string
	char		 *port_char;	// position of ':' if any
	unsigned int slash_pos;		// position of first '/' numeric
	char		 *slash_ptr;
	char		 szUseURL[512]; // incase we have to cat something to it.

	memset((void*)&m_Request,0,sizeof(HTTP_REQUEST));

	// Set the request type
	m_Request.rtype = iType;

	// Copy the URL to use
	strcpy(szUseURL,szURL);

	// Copy the referer
	if(szReferer) {
		strcpy(m_Request.referer,szReferer);
	}

	if(iType==HTTP_POST) {
		strcpy(m_Request.data,szPostData);
	}

	// Copy hostname from URL
	slash_ptr = strchr(szUseURL,'/');

	if(!slash_ptr) {
		strcat(szUseURL,"/");
		slash_ptr = strchr(szUseURL,'/');
	}

	slash_pos=(slash_ptr-szUseURL);
	memcpy(m_Request.host,szUseURL,slash_pos);
	m_Request.host[slash_pos]='\0';

	// Copy the rest of the url to the file string.
	strcpy(m_Request.file,strchr(szUseURL,'/'));

	// Any special port used in the URL?
	if((port_char=strchr(m_Request.host,':'))!=NULL) {
		strcpy(port,port_char+1);
		*port_char='\0';
		m_Request.port = atoi(port);
	}
	else {
		m_Request.port = 80;
	}
}

//----------------------------------------------------

void CHttpClient::Process()
{
	int   header_len;
	char  request_head[16384];

	if(!Connect(m_Request.host,m_Request.port)) {
		return;
	}

	// Build the HTTP Header
	switch(m_Request.rtype)
	{
		case HTTP_GET:
			header_len = strlen(m_Request.file)+strlen(m_Request.host)+
				(strlen(GET_FORMAT)-8)+strlen(USER_AGENT)+
				strlen(m_Request.referer);
			sprintf(request_head,GET_FORMAT,m_Request.file,USER_AGENT,m_Request.referer,m_Request.host);
			break;

		case HTTP_HEAD:
			header_len = strlen(m_Request.file)+strlen(m_Request.host)+
				(strlen(HEAD_FORMAT)-8)+strlen(USER_AGENT)+
				strlen(m_Request.referer);
			sprintf(request_head,HEAD_FORMAT,m_Request.file,USER_AGENT,m_Request.referer,m_Request.host);
			break;

		case HTTP_POST:
			header_len = strlen(m_Request.data)+strlen(m_Request.file)+
				strlen(m_Request.host)+strlen(POST_FORMAT)+
				strlen(USER_AGENT)+strlen(m_Request.referer);
			sprintf(request_head,POST_FORMAT,m_Request.file,USER_AGENT,m_Request.referer,m_Request.host,strlen(m_Request.data),m_Request.data);
			break;	
	}

	//OutputDebugString(request_head);

	if(!Send(request_head))	return;

	HandleEntity();
}

//----------------------------------------------------

#define RECV_BUFFER_SIZE 2048

void CHttpClient::HandleEntity()
{
	int				bytes_total		= 0;
	int				bytes_read		= 0;
	char			buffer[RECV_BUFFER_SIZE];
	char			response[MAX_ENTITY_LENGTH];

	char			header[1024];
	char			*head_end;
	char			*pcontent_buf;
	char			content_len_str[256];

	bool			header_got		= false;
	bool			has_content_len = false;
	int				header_len		= 0;
	int				content_len		= 0;

	while((bytes_read=Recv(buffer,RECV_BUFFER_SIZE)) > 0)
	{
		bytes_total+=bytes_read;
		memcpy(response+(bytes_total-bytes_read),buffer,(unsigned int)bytes_read);
	
		if(!header_got)
		{
			if((head_end=strstr(response,"\r\n\r\n"))!=NULL
				|| (head_end=strstr(response,"\n\n"))!=NULL)
			{

				header_got=true;

				header_len=(head_end-response);
				memcpy(header,response,header_len);
				header[header_len]='\0';

				if((*(response+header_len))=='\n') /* LFLF */
				{
					bytes_total-=(header_len+2);
					memmove(response,(response+(header_len+2)),bytes_total);
				}
				else /* assume CRLFCRLF */
				{
					bytes_total-=(header_len+4);
					memmove(response,(response+(header_len+4)),bytes_total);
				}

				/* find the content-length if available */
				if((pcontent_buf=Util_stristr(header,"CONTENT-LENGTH:"))!=NULL)
				{
					has_content_len=true;

					pcontent_buf+=16;
					while(*pcontent_buf!='\n' && *pcontent_buf)
					{
						*pcontent_buf++;
						content_len++;
					}

					pcontent_buf-=content_len;
					memcpy(content_len_str,pcontent_buf,content_len);

					if(content_len_str[content_len-1] == '\r') {
					   content_len_str[content_len-1]='\0';
					}
					else {
					   content_len_str[content_len]='\0';
					}

					content_len=atoi(content_len_str);
	
					if(content_len > MAX_ENTITY_LENGTH) {
						CloseConnection();
						m_iError = HTTP_ERROR_CONTENT_TOO_BIG;
						return;
					}
				}
			}
		}

		if(header_got && has_content_len)
			if(bytes_total>=content_len) break;
	}

	CloseConnection();
	
	response[bytes_total]='\0';

	// check the returned header
	unsigned long *magic = (unsigned long *)header;

	if(*magic != 0x50545448) { // 'HTTP'
		m_iError = HTTP_ERROR_MALFORMED_RESPONSE;
		return;
	}

	// Now fill in the response code
	char response_code_str[4];
	response_code_str[0] = *(header+9);
	response_code_str[1] = *(header+10);
	response_code_str[2] = *(header+11);
	response_code_str[3] = '\0';
	m_Response.response_code = atoi(response_code_str);

	// Copy over the document entity strings and sizes
	memcpy(m_Response.header,header,header_len+1);
	m_Response.header_len	= header_len;
	memcpy(m_Response.response,response,bytes_total+1);
	m_Response.response_len	= bytes_total;

	/*
	char s[4096];
	m_Response.response[bytes_total] = '\0';
	sprintf(s,"Code: %u\n\n%s\n%s\n",m_Response.response_code,m_Response.header,m_Response.response);
	OutputDebugString(s);*/

	// Try and determine the document type
	m_Response.content_type = CONTENT_TYPE_HTML; // default to html

	char szContentType[256];

	if(GetHeaderValue((char*)"CONTENT-TYPE:",szContentType,256) == true) {
		if(strstr(szContentType,"text/html") != NULL) {
			m_Response.content_type = CONTENT_TYPE_HTML;
		}
		else if(strstr(szContentType,"text/plain") != NULL) {
			m_Response.content_type = CONTENT_TYPE_TEXT;
		} else {
			m_Response.content_type = CONTENT_TYPE_UNKNOWN;
		}
	}
}


//----------------------------------------------------