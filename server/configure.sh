#!/bin/bash

#-------------------------
#	Programowanie obiektowe
#	Adam Wolski 106582 I1.1
#	@Czw 15:10
#-------------------------

projectname="server"

mains_number=`find . -name 'main*.c' | wc -w`

if test $(($mains_number)) -gt 1 
then
	mains=`find . -name 'main*.c' -maxdepth 1`
	echo -e "Ej ej! Nie oszukuj mnie! Jest więcej niż jeden main: \n\n $mains \n\n Którego mam kompilować?"
	read main
	echo "A zatem $main będzie się radośnie kompilował."
else
	main=`find . -name 'main*.c' -maxdepth 1`
fi

touch Makefile
clr_main=`basename $main | cut -d\. -f1` 
main_gpp=`gcc -MM $main -g -Wall -pthread`

obiekty=`find . -name "*.c" -maxdepth 1 | grep -v "main" | sed "s/\.c/\.o/g" | tr '\n' ' ' `
echo $obiekty;
echo -e "all: $projectname\n\n$projectname: $clr_main.o $obiekty\n\tgcc $clr_main.o $obiekty -o $projectname -pthread -g -lcrypto -Wall\n" > Makefile

elem_main=`gcc -MM $main -Wall`
echo -e "$elem_main" >> Makefile
echo -e "\tgcc -c $clr_main.c -o $clr_main.o -lcrypto -g -Wall -pthread\n" >> Makefile

for parameter in `find . -name "*.c" -maxdepth 1 | grep -v "main"` 
do
	elem=`gcc -MM $parameter`
	clr_param=`basename $parameter | cut -d\. -f1`
	echo -e "$elem" >> Makefile
	echo -e "\tgcc -c $clr_param.c -o $clr_param.o -g\n" >> Makefile
done

echo -e "clean: \n\trm *.o\n\trm $projectname" >> Makefile

echo "Makefile zostal wygenerowany."

