/*
 *  pulseblaster.h - the header file with the ioctl definitions.
 *
 *  The declarations here have to be in a header file, because
 *  they need to be known both to the kernel module
 *  (in chardev.c) and the process calling ioctl (ioctl.c)
 */

#ifndef PULSEBLASTER_H
#define PULSEBLASTER_H

#include <linux/ioctl.h>
#define PULSEBLASTER_DEVICE_MAGIC_NUMBER 'p'

#define IOCTL_OUTB _IOW(PULSEBLASTER_DEVICE_MAGIC_NUMBER, 1, unsigned long)
#define IOCTL_INB _IOR(PULSEBLASTER_DEVICE_MAGIC_NUMBER, 2, unsigned long)

/* 
 * The name of the device file 
 */
#define PULSEBLASTER_DEVICE_FILE_NAME "pulseblaster"

#endif
