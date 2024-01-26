#ifndef SIGNAL_API_H
#define SIGNAL_API_H

/* Standard includes. */
#include "signal_data_types.h"

/*-----------------------------------------------------------*/

SignalResult_t Signal_createSignal( SignalContext_t *pCtx, char * pAwsRegion, size_t awsRegionLength, char * pControlPlaneUrl, size_t controlPlaneUrlLength, char * pChannelName, size_t channelNameLength );

SignalResult_t Signal_setChannelEndpointHttps( SignalContext_t *pCtx, char * pChannelEndpointHttps, size_t channelEndpointHttpsLength );

SignalResult_t Signal_setChannelArn( SignalContext_t *pCtx, char * pChannelArn, size_t channelArnLength );

SignalResult_t Signal_setClientId( SignalContext_t *pCtx, char * pClientId, size_t clientIdLength );

SignalResult_t Signal_getDescribeChannelRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength );

SignalResult_t Signal_parseDescribeChannelMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalDescribeChannel_t *pDescribeChannel );

SignalResult_t Signal_getMediaStorageConfigRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength );

SignalResult_t Signal_parseMediaStorageConfigMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalMediaStorageConfig_t *pMediaStorageConfig );

SignalResult_t Signal_getCreateChannelRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalCreateChannel_t * pCreateChannel );

SignalResult_t Signal_parseCreateChannelMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalCreateChannel_t *pCreateChannel );

SignalResult_t Signal_getIceConfig( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength );

SignalResult_t Signal_parseIceConfigMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalIceConfigMessage_t *pIceConfigMessage );

/*-----------------------------------------------------------*/

#endif /* SIGNAL_API_H */
