#ifndef SOCKETS_H
#define SOCKETS_H

void SOCsendMessage ( int * a_soc, char* a_message, unsigned long * a_length );
char* SOCreceiveMessage ( int * a_soc, char* aout_message, unsigned long * aout_size );

#endif