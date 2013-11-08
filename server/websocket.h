#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 

int WEBSOChandshake( int a_soc );

char* WEBSOCcreateFrame(char* a_message, char* frame, unsigned long * aout_size);
char* WEBSOCcreateHandshakeResponse(char* key, char* buffer);
char* WEBSOCgenerateAcceptKey(char* key, char* res);
char* WEBSOCgetRequestKey(char* a_request, char* key);
char* WEBSOCdecodeFrame( char* a_frame, char* decoded, unsigned long * a_frameLength );

void WEBSOCsendMessage( int * a_soc, char* a_message ); 


#endif