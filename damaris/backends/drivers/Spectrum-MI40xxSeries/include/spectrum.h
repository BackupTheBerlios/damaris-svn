HEAD  SpcInitPCIBoards  (int16 *pnCount, int16 *pnPCIVersion);
HEAD  SpcInitBoard      (int16 nNr, int16 nTyp);
HEAD  SpcSetParam       (int16 nNr, int32 lReg, int32 lValue);
HEAD  SpcGetParam       (int16 nNr, int32 lReg, int32 *plValue);
HEAD  SpcGetData        (int16 nNr, int16 nCh, int32 lStart, int32 lLen, dataptr pvData);
HEAD  SpcSetData        (int16 nNr, int16 nCh, int32 lStart, int32 lLen, dataptr pvData);
HEAD  SpcGetVersionInfo (char *pszBuffer, int nBufferLen);
