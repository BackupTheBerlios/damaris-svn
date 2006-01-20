#!/bin/sh
#
# Startup script to load nmr hardware drivers
#
# chkconfig: 2345 03 92
#
# description: loads nmr hardware drivers
#
# Script Author:        Achim Gaedke <achim.gaedke@physik.tu-darmstadt.de>
# Created:              February 28th,  2005
# adapted from Mandrake Linux 10.1 scripts

RETVAL=0

NMR_MODULE_DIR=/lib/modules/`uname -r`/nmr
SPECTRUM_MOD=$NMR_MODULE_DIR/spc.ko
PULSEBLASTER_MOD=$NMR_MODULE_DIR/pulseblaster.ko
LSMOD=/sbin/lsmod
RMMOD=/sbin/rmmod
INSMOD=/sbin/insmod
LSPCI=/sbin/lspci
PULSEBLASTER_IOPORT=""
PULSEBLASTER_DEV="/dev/pulseblaster"
PULSEBLASTER_MAJOR=""
SPECTRUM_DEV="/dev/spc0"
SPECTRUM_MAJOR=""
NMR_GROUP="nmr"

pb_findio() {
	lspci -vn |
	     while read line; do
		if echo "$line"|grep "ff00: 10e8:8852$" >/dev/null; then
		    read line
		    read line
		    echo "$line"|sed 's/I\/O ports at \([0-9a-f]*\).*/0x\1/'
		    break
		fi
	     done
}

find_major() {
 grep " $1\$" /proc/devices |sed "s/ *\([0-9]*\) $1/\1/"
}

start() {
	if $LSMOD | egrep "^spc|^pulseblaster">/dev/null ; then
	   printf "nothing done, pulseblaster or spc already loaded...\n"
	   return 
	fi
	if test \! \( -f $SPECTRUM_MOD -a -f $PULSEBLASTER_MOD \); then
	    printf "pulseblaster or specturm module not available...\n"
	    return
	fi
	PULSEBLASTER_IOPORT=`pb_findio`
	if test -z "$PULSEBLASTER_IOPORT"; then
	    printf "pulseblaster IO ports not found...\n"
	    RETVAL=2
	    return
	fi
	$INSMOD $SPECTRUM_MOD
	$INSMOD $PULSEBLASTER_MOD base_address=$PULSEBLASTER_IOPORT

	# now find major device numbers
	PULSEBLASTER_MAJOR=`find_major "pulseblaster"`
	if test -z "$PULSEBLASTER_MAJOR"; then
	    printf "could not determine pulseblaster's major device number"
	fi
	SPECTRUM_MAJOR=`find_major "spec"`
	if test -z "$SPECTRUM_MAJOR"; then
	    printf "could not determine spectrum's major device number"
	fi
	
	mknod $PULSEBLASTER_DEV c $PULSEBLASTER_MAJOR 0
	mknod $SPECTRUM_DEV c $SPECTRUM_MAJOR 0
	if grep "^$NMR_GROUP:" /etc/group>/dev/null; then
	    chgrp $NMR_GROUP $PULSEBLASTER_DEV
	    chgrp $NMR_GROUP $SPECTRUM_DEV
	    chmod g=rw,o= $PULSEBLASTER_DEV
	    chmod g=rw,o= $SPECTRUM_DEV	     
	else
	    echo It is recommended to create a group named $NMR_GROUP
	    chmod a=rw $PULSEBLASTER_DEV
	    chmod a=rw $SPECTRUM_DEV
	fi
}

stop() {
	if $LSMOD | egrep "^spc ">/dev/null ; then
	  $RMMOD spc
	  test -c $SPECTRUM_DEV && rm $SPECTRUM_DEV
	fi
	if $LSMOD | egrep "^pulseblaster ">/dev/null ; then
	  $RMMOD pulseblaster
	  test -c $PULSEBLASTER_DEV && rm $PULSEBLASTER_DEV
	fi
       
}

status () {
	if $LSMOD | egrep "^spc ">/dev/null ; then
	   echo spectrum module loaded
	else
	   echo spectrum module not loaded
	fi
	if $LSMOD | egrep "^pulseblaster ">/dev/null ; then
	   echo pulseblaster module loaded
	else
	   echo pulseblaster module not loaded
	fi
}

case "$1" in
        start)
                start
                ;;
        stop)
                stop
                ;;
        restart)
                stop
                start
                ;;
        status)
                status
                ;;
        *)
                printf "Usage: %s {start|stop|restart|status}\n" "$0"
                RETVAL=1
esac
exit $RETVAL
