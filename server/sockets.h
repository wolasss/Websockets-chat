#ifndef SOCKETS_H
#define SOCKETS_H

int SOCsendMessage( int * a_soc, char * a_message, unsigned long long * bytes );
char *SOCreceiveMessage( int * a_soc, int bytes, char * aout_message, unsigned int * error);
char *SOCreceiveRequest ( int *a_soc, char *aout_message, int a_bufSize );
char *SOCreceiveFrame ( int * a_soc, char* aout_message, unsigned long long * aout_size );

#endif