#ifndef SOCKETS_H
#define SOCKETS_H

void SOCsendMessage ( int * a_soc, unsigned char* a_message, unsigned long * a_length );
unsigned char* SOCreceiveMessage ( int * a_soc, unsigned char* aout_message, unsigned long * aout_size );

#endif