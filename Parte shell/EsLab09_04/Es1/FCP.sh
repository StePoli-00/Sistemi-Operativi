#FCP.sh /home/paolo 10
#!/bin/sh
echo FCP.sh START
if test $# -ne 2		#Controllo parametri
then echo Error\(01\): Parameters; exit 0
fi

case $1 in 			#Controllo se perorso assoluto
	/*);;
	*) echo Error\(02\): $1 not a directory; exit 1;;
esac

if test $2 -lt 1		#Controllo  se numero maggiore di 0
then echo Error\(03\): $2 not positive; exit 2
fi

#Settaggio della variabile PATH
PATH=`pwd`:$PATH
export PATH
#Chiamata alla funzione ricorsiva
FCR.sh $1 $2
echo FCP.sh END
