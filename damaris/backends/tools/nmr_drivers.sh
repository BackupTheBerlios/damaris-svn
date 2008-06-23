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
		return
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
		return
	     fi
        fi

	# with udev, we should have links and also the rights set correctly
        if test  $PULSEBLASTER_RETVAL -ne 0  -o $SPECTRUM_RETVAL -ne 0; then
             return $HARDWARE_FAIL_RETVAL
        fi

	# have to wait for a short time, let udev do the work
	/sbin/udevsettle
	if ! test -c ${PULSEBLASTER_DEV}0; then
		echo "no pulseblaster board is found, only the debug device at ${PULSEBLASTER_DEV}_debug is available"
	fi


}

stop() {
	if egrep "^spc$SPECTRUM_SMP_EXT " /proc/modules >/dev/null ; then
	  # test wheter it is still used
	  if test "`lsof $SPECTRUM_DEV`x" = "x"; then
		$MODPROBE -r ${SPECTRUM_NAME}${SPECTRUM_SMP_EXT}
	  else
	        echo "spectrum driver still in use (use lsof)"
	  fi
	fi
	if egrep "^${PULSEBLASTER_NAME} " /proc/modules > /dev/null ; then
	  # test wheter it is still used
	  $MODPROBE -r ${PULSEBLASTER_NAME}
	fi
       
}

status () {
	if egrep "^${SPECTRUM_NAME}$SPECTRUM_SMP_EXT " /proc/modules > /dev/null ; then
	   echo spectrum module loaded
	else
	   echo spectrum module not loaded
	fi
	if egrep "^${PULSEBLASTER_NAME} " /proc/modules > /dev/null ; then
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
