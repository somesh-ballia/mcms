#ifndef  __ART_REQUEST_STRUCTS_H__
#define  __ART_REQUEST_STRUCTS_H__

typedef struct
{
  APIS32	enNetworkType;
  APIU32 	unVideoTxMaxNumberOfBitsPer10ms;
  APIU32    ConnectContent; //(TRUE/FALSE)
  APIU32    nMediaMode;              // mediaModeEnum
} TOpenArtReq;

//FIPS 140
typedef struct
{
   APIU32 unSimulationErrCode;
} TArtFips140Req;

typedef struct
{
  APIS32 bIsNeedToCollectInfoFromArt;
} TCloseArtReq;


#endif
