// ****************************************************************************
// spcioctl.inc
// ****************************************************************************
// include source code for easy access of the ioctl and data transfer commands
// of the spectrum driver.
// ****************************************************************************



// ----- ioctl_GetParam -----------------------------------
int SpcGetParam (int hDrv, int32 lReg, int32 *plValue)
	{
	int32 lData[2];
	int16 nRet;

	lData[0] = lReg;
	nRet = ioctl (hDrv, GETPARAM, lData);
	*plValue = lData[1];

	return nRet;
	}



// ----- ioctl_SetParam -----------------------------------
int SpcSetParam (int hDrv, int32 lReg, int32 lValue) 
	{
	int32 lData[2];
	
	lData[0] = lReg;
	lData[1] = lValue;

	return ioctl(hDrv, SETPARAM, lData);
	}


// ----- ioctl_GetData ------------------------------------
int32 SpcGetData (int hDrv, int32 lCh, int32 lStart, int32 lLen, int16 nBytesPerSample, dataptr pvData)
	{
	int32 lRet;
	int32 lData[1];
	lData[0] = lCh;

	ioctl (hDrv, SETCH, lData);

 	lseek (hDrv, lStart * nBytesPerSample, SEEK_SET);
	lRet = read (hDrv, pvData, nBytesPerSample * lLen);

	return lRet;
	}



// ----- ioctl_SetData ------------------------------------
int32 SpcSetData (int hDrv, int32 lCh, int32 lStart, int32 lLen, int16 nBytesPerSample, dataptr pvData)
	{
	int32 lRet;
	int32 lData[1];
	
	lData[0] = lCh;

	ioctl (hDrv, SETCH, lData);

	lseek (hDrv, lStart * nBytesPerSample, SEEK_SET);
	lRet = write (hDrv, pvData, nBytesPerSample * lLen);

	return lRet;
	}
