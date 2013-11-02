#ifndef WEBSOCKET_H
#define WEBSOCKET_H

int WEBSOChandshake( int a_soc );
void WEBSOC_createFrame(unsigned char* a_message, unsigned char * frame, unsigned long * aout_size);
void WEBSOCcreateHandshakeResponse(unsigned char* key, char* buffer);
void WEBSOCgenerateAcceptKey(unsigned char* key, unsigned char* res);
void WEBSOCgetRequestKey(unsigned char* a_request, unsigned char* key);
void WEBSOCsendMessage( int * a_soc, unsigned char* a_message ); 
unsigned char* WEBSOCdecodeFrame( unsigned char* a_frame, unsigned long * a_frameLength );

#endif