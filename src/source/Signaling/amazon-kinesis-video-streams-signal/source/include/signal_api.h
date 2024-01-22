#ifndef SIGNAL_API_H
#define SIGNAL_API_H

/* Standard includes. */
#include "signal_data_types.h"

/*-----------------------------------------------------------*/

SignalResult_t Signal_createSignal( SignalContext_t *pCtx, char * pAwsRegion, uint32_t awsRegionLength, char * pControlPlaneUrl, uint32_t controlPlaneUrlLength, char * pChannelName, uint32_t channelNameLength );

SignalResult_t Signal_setChannelEndpointHttps( SignalContext_t *pCtx, char * pChannelEndpointHttps, uint32_t channelEndpointHttpsLength );

SignalResult_t Signal_setChannelArn( SignalContext_t *pCtx, char * pChannelArn, uint32_t channelArnLength );

SignalResult_t Signal_setClientId( SignalContext_t *pCtx, char * pClientId, uint32_t clientIdLength );

SignalResult_t Signal_getDescribeChannelRequest( SignalContext_t *pCtx, char * pUrl, uint32_t * pUrlLength, char *pBody, uint32_t * pBodyLength );

SignalResult_t Signal_parseDescribeChannelMessage( SignalContext_t *pCtx, char * pMessage, uint32_t messageLength, SignalDescribeChannel_t *pDescribeChannel );

SignalResult_t Signal_getMediaStorageConfigRequest( SignalContext_t *pCtx, char * pUrl, uint32_t * pUrlLength, char *pBody, uint32_t * pBodyLength );

SignalResult_t Signal_parseMediaStorageConfigMessage( SignalContext_t *pCtx, char * pMessage, uint32_t messageLength, SignalMediaStorageConfig_t *pMediaStorageConfig );

SignalResult_t Signal_getIceConfig( SignalContext_t *pCtx, char * pUrl, uint32_t * pUrlLength, char *pBody, uint32_t * pBodyLength );

SignalResult_t Signal_parseIceConfigMessage( SignalContext_t *pCtx, char * pMessage, uint32_t messageLength, SignalIceConfigMessage_t *pIceConfigMessage );

/*-----------------------------------------------------------*/

#endif /* SIGNAL_API_H */
