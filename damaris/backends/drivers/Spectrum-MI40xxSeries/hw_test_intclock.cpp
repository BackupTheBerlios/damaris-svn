/*
 * simple test program simplified from
 * mi40xx.cpp example program (c) Spectrum GmbH
 *
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "dlltyp.h"
#include "regs.h"
#include "spcerr.h"
#include "spcioctl.inc"


// ----- main task -----
int main(int argc, char **argv)
{
    int     hDrv;
    //int16   nCount, nPCIBusVersion
    int16   nChannels;
    int32   lChEnable, lStatus, lMemsize;
    int32   lErrorCode, lErrorReg, lErrorValue;
    ptr16   pnData[4];
    ptr16   pnTmp;
    int32   i, k;
    int16   j, nErr;


    hDrv = open ("/dev/spc0", O_RDWR);
    if (hDrv <= 0)
    {
        printf ("device not found\n");
        return -1;
    }


    nChannels = 2;
    lChEnable = CHANNEL0 | CHANNEL1;

    // ----- set memsize for recording ----
    lMemsize = 2*1024;

    // ----- setup board for recording -----
    for (j=0; j<nChannels; j++)
        SpcSetParam (hDrv, SPC_AMP0 + 100*j,    5000);          // 5 V sensitivity
    for (j=0; j<nChannels; j++)
        SpcSetParam (hDrv, SPC_50OHM0 + 100*j,  1);             // 50 Ohm input impedance

    SpcSetParam (hDrv, SPC_CHENABLE,            lChEnable);     // Enable channels for recording
    SpcSetParam (hDrv, SPC_POSTTRIGGER,         lMemsize/2);    // Trigger event in middle of data
    SpcSetParam (hDrv, SPC_MEMSIZE,             lMemsize);      // Memory size

    SpcSetParam (hDrv, SPC_PLL_ENABLE,          1);             // Internal PLL enabled for clock
    SpcSetParam (hDrv, SPC_EXTERNALCLOCK,       0);             // Internal clock used
    SpcSetParam (hDrv, SPC_CLOCKOUT,       	0);             // no clock out
    SpcSetParam (hDrv, SPC_REFERENCECLOCK,      0);             // Internal clock used
    SpcSetParam (hDrv, SPC_CLOCK50OHM,       	0);             // clock 50 Ohm
    printf ("Using external internal clock\n");
    SpcSetParam (hDrv, SPC_SAMPLERATE,          10000000);      // Samplerate: 10 MHz
    SpcSetParam (hDrv, SPC_EXTERNOUT,           0);             // No clock output

    SpcSetParam (hDrv, SPC_TRIGGERMODE,         TM_SOFTWARE);   // Software trigger is used
    SpcSetParam (hDrv, SPC_PULSEWIDTH,          0);             // Not used for software mode
    SpcSetParam (hDrv, SPC_TRIGGEROUT,          0);             // No trigger output



    // ----- start the board -----
    printf("Starting recording\n");
    nErr = SpcSetParam (hDrv, SPC_COMMAND,      SPC_START);     // start the board



    // ----- driver error: request error and end program -----
    if (nErr != ERR_OK)
    {
        SpcGetParam (hDrv, SPC_LASTERRORCODE,   &lErrorCode);
        SpcGetParam (hDrv, SPC_LASTERRORREG,    &lErrorReg);
        SpcGetParam (hDrv, SPC_LASTERRORVALUE,  &lErrorValue);

        printf ("Driver error: %i in register %i at value %i\n", lErrorCode, lErrorReg, lErrorValue);

        return -1;
    }



    // ----- Wait for Status Ready  -----
    printf("Wating\n");
    do
    {
        SpcGetParam (hDrv, SPC_STATUS,      &lStatus);
    }
    while (lStatus != SPC_READY);

    printf ("Board is ready, recording has finished\n");



    // ----- allocate memory for data -----
    for (i=0; i<nChannels; i++)
        pnData[i] = (ptr16) malloc (lMemsize * sizeof(int16));

    // ----- data is muxed and must be splitted -----
    pnTmp = (ptr16) malloc (lMemsize * nChannels * sizeof(int16));
    SpcGetData (hDrv, 0, 0, nChannels * lMemsize, 2, (dataptr) pnTmp);
    for (i=0; i<lMemsize; i++)
        for (k=0; k<nChannels; k++)
            pnData[k][i] = pnTmp[nChannels * i + k];
    free (pnTmp);



    // ----- output samples  -----
    printf ("# Data (Integer):\n# Ch0  Ch1\n");
    for (i=0; i<lMemsize; i++)
    {
        for (j=0; j<nChannels; j++)
            printf ("% 5i ", pnData[j][i]);
        printf ("\n");
    }

    // ----- free data memory -----
    for (i=0; i<nChannels; i++)
        free (pnData[i]);

    close (hDrv);


    return 0;
}
