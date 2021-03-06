#!/bin/sh
### BEGIN INIT INFO
# Provides:          damaris-backends
# Required-Start:    $syslog
# Required-Stop:     $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Startup script to load nmr hardware drivers for use with damaris backends
# Description:       loads available nmr hardware drivers and grants rights to nmr group
#                    these drivers will be typically used by damaris backends and accompanying programs
### END INIT INFO

# Script Author:        Achim Gaedke <achim.gaedke@physik.tu-darmstadt.de>
# Created:              February 28th,  2005, adapted to LSB June 2008

PATH=/sbin:/usr/sbin:/bin:/usr/bin

RETVAL=0
# failure value for nonexisting hardware
HARDWARE_FAIL_RETVAL=0

NMR_MODULE_DIR=/lib/modules/`uname -r`/kernel/damaris
NMR_GROUP="nmr"

# tools
RMMOD=/sbin/rmmod
MODPROBE=/sbin/modprobe
KERNELCONF=/boot/config-`uname -r`

# spectrum related stuff
SPECTRUM_NAME=spc
CONFIG_SMP="`grep 'CONFIG_SMP *=' $KERNELCONF | sed 's/.*=//'`"
if test "x$CONFIG_SMP" = "xy"; then
  SPECTRUM_SMP_EXT="_smp";
else
  SPECTRUM_SMP_EXT="";
fi
SPECTRUM_DEV=/dev/${SPECTRUM_NAME}0
SPECTRUM_MOD_FILE=$NMR_MODULE_DIR/${SPECTRUM_NAME}${SPECTRUM_SMP_EXT}.ko

# pulseblaster related stuff
PULSEBLASTER_NAME=pulseblaster
PULSEBLASTER_MOD_FILE=$NMR_MODULE_DIR/$PULSEBLASTER_NAME.ko
PULSEBLASTER_DEV=/dev/$PULSEBLASTER_NAME

start() {

        SPECTRUM_RETVAL=0
        # find out whether spectrum module is running
	if egrep "^spc${SPECTRUM_SMP_EXT} " /proc/modules >/dev/null ; then
	     echo "spectrum module already loaded"
	else
	     # ok, load it
             if test -f $SPECTRUM_MOD_FILE; then
		if ! $MODPROBE ${SPECTRUM_NAME}${SPECTRUM_SMP_EXT} >/dev/null 2>&1; then
		        echo "spectrum module failed to load"
			SPECTRUM_RETVAL=1
		fi
	     else
		echo "spectrum module not found"
		SPECTRUM_RETVAL=1
	     fi
        fi

        PULSEBLASTER_RETVAL=0
        # find out whether pulseblaster module is running
        if egrep "^pulseblaster " /proc/modules >/dev/null ; then
	     echo "pulseblaster module already loaded"
	else
	     # ok, load it
             if test -f $PULSEBLASTER_MOD_FILE; then
		if ! $MODPROBE $PULSEBLASTER_NAME >/dev/null 2>&1; then
			echo "pulseblaster module failed to load"
			PULSEBLASTER_RETVAL=1
		fi
	     else
		echo "pulseblaster module not found"
		PULSEBLASTER_RETVAL=1
	     fi
        fi

	# have to wait for a short time, let udev do the work
	if [ -x /sbin/udevsettle ]; then
	/sbin/udevsettle
	fi
	if [ -x /sbin/udevadm ]; then
	/sbin/udevadm settle
	fi
	if ! test $PULSEBLASTER_RETVAL -ne 0 -o -c ${PULSEBLASTER_DEV}0 ; then
		echo "no pulseblaster board is found, only the debug device at ${PULSEBLASTER_DEV}_debug is available"
	fi

	# with udev, we should have links and also the rights set correctly
        if test  $PULSEBLASTER_RETVAL -ne 0  -o $SPECTRUM_RETVAL -ne 0; then
             RETVAL=$HARDWARE_FAIL_RETVAL
        fi

}

stop() {
	if egrep "^spc$SPECTRUM_SMP_EXT " /proc/modules >/dev/null ; then
	  # test wheter spectrum driver is still used
	  # !!!! the usage counter of the spc kernel module is not maintained!!!!
	  if test "`lsof $SPECTRUM_DEV`x" = "x"; then
		$MODPROBE -r ${SPECTRUM_NAME}${SPECTRUM_SMP_EXT} || RETVAL=1
	  else
	        echo "spectrum driver still in use (use lsof)"
                RETVAL=1
	  fi
	fi
	if egrep "^${PULSEBLASTER_NAME} " /proc/modules > /dev/null ; then
	  # test wheter it is still used
	  $MODPROBE -r ${PULSEBLASTER_NAME} || RETVAL=1
	fi
	# have to wait for a short time, let udev do the work
	if [ -x /sbin/udevsettle ]; then
	/sbin/udevsettle
	fi
	if [ -x /sbin/udevadm ]; then
	/sbin/udevadm settle
	fi
}

status () {
	if egrep "^${SPECTRUM_NAME}$SPECTRUM_SMP_EXT " /proc/modules > /dev/null ; then
	   echo spectrum module loaded
	else
	   echo spectrum module not loaded
           RETVAL=3
	fi
	if egrep "^${PULSEBLASTER_NAME} " /proc/modules > /dev/null ; then
	   echo pulseblaster module loaded
	else
	   echo pulseblaster module not loaded
           RETVAL=3
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
