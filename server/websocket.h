#ifndef WEBSOCKET_H
#define WEBSOCKET_H

int WEBSOChandshake( int a_soc );

char *WEBSOCcreateFrame(char *a_message, char *frame, unsigned long long *aout_size);
char *WEBSOCcreateHandshakeResponse(char *key, char *buffer);
char *WEBSOCgenerateAcceptKey(char *key, char *res);
char *WEBSOCgetRequestKey(char *a_request, char *key);
char *WEBSOCdecodeFrame( char *a_frame, char *decoded, unsigned long long *a_frameLength );

int WEBSOCsendMessage( int *a_soc, char *a_message );


#endif