#ifndef SIGNAL_API_H
#define SIGNAL_API_H

/* Standard includes. */
#include "signal_data_types.h"

/*-----------------------------------------------------------*/

SignalResult_t Signal_createSignal( SignalContext_t *pCtx, SignalCreate_t *pCreate );

SignalResult_t Signal_getDescribeChannelRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalDescribeChannelRequest_t * pDescribeChannelRequest );

SignalResult_t Signal_parseDescribeChannelMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalDescribeChannel_t *pDescribeChannel );

SignalResult_t Signal_getMediaStorageConfigRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalMediaStorageConfigRequest_t * pMediaStorageConfigRequest );

SignalResult_t Signal_parseMediaStorageConfigMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalMediaStorageConfig_t *pMediaStorageConfig );

SignalResult_t Signal_getCreateChannelRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalCreateChannel_t * pCreateChannel );

SignalResult_t Signal_parseCreateChannelMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalCreateChannel_t *pCreateChannel );

SignalResult_t Signal_getChannelEndpointRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalGetChannelEndpointRequest_t * pChannelEndpointRequest );

SignalResult_t Signal_parseChannelEndpointMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalEndpoints_t * pEndpoints );

SignalResult_t Signal_getIceConfig( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalIceConfigRequest_t * pIceConfigReqeust);

SignalResult_t Signal_parseIceConfigMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalIceConfigMessage_t *pIceConfigMessage );

SignalResult_t Signal_getJoinStorageSessionRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalJoinStorageSessionRequest_t * pJoinStorageSessionRequest );

/*-----------------------------------------------------------*/

#endif /* SIGNAL_API_H */
