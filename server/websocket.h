#ifndef WEBSOCKET_H
#define WEBSOCKET_H

int WEBSOChandshake( int a_soc );
unsigned char* WEBSOC_createFrame(unsigned char* a_message, unsigned char * frame, unsigned long * aout_size);
unsigned char* WEBSOCcreateHandshakeResponse(unsigned char* key, char* buffer);
unsigned char* WEBSOCgenerateAcceptKey(unsigned char* key, unsigned char* res);
unsigned char* WEBSOCgetRequestKey(unsigned char* a_request, unsigned char* key);
void WEBSOCsendMessage( int * a_soc, unsigned char* a_message ); 
unsigned char* WEBSOCdecodeFrame( unsigned char* a_frame, unsigned char* decoded, unsigned long * a_frameLength );

#endif