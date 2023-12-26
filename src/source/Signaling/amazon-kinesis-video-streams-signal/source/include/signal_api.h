#ifndef SIGNAL_API_H
#define SIGNAL_API_H

/* Standard includes. */
#include "signal_data_types.h"

#define AWS_REGION_MAX_LENGTH ( 50 )
#define AWS_CONTROL_PLANE_URL_MAX_LENGTH ( 256 )
#define AWS_SIGNALING_CLIENT_ID_MAX_LENGTH ( 256 )

/**
 * Max ARN len in chars
 * https://docs.aws.amazon.com/kinesisvideostreams/latest/dg/API_DescribeSignalingChannel.html#API_DescribeSignalingChannel_RequestSyntax
 * https://docs.aws.amazon.com/kinesisvideostreams/latest/dg/API_CreateStream.html#KinesisVideo-CreateStream-request-KmsKeyId
 */
#define AWS_MAX_ARN_LEN 2048

#define AWS_ICE_SERVER_MAX_NUM ( 5 )
#define AWS_ICE_SERVER_MAX_URIS ( 5 )

typedef struct SignalContext
{
    char region[AWS_REGION_MAX_LENGTH];
    uint32_t regionLength;
    char controlPlaneUrl[AWS_CONTROL_PLANE_URL_MAX_LENGTH];
    uint32_t controlPlaneUrlLength;
    char channelEndpointHttps[AWS_CONTROL_PLANE_URL_MAX_LENGTH];
    uint32_t channelEndpointHttpsLength;
    char channelArn[AWS_MAX_ARN_LEN];
    uint32_t channelArnLength;
    char clientId[AWS_SIGNALING_CLIENT_ID_MAX_LENGTH];
    uint32_t clientIdLength;
} SignalContext_t;

typedef struct SignalIceServer
{
    char * pPassword;
    uint32_t passwordLength;
    char * pTtl;
    uint32_t ttlLength;
    char * pUris[AWS_ICE_SERVER_MAX_URIS];
    uint32_t urisLength[AWS_ICE_SERVER_MAX_URIS];
    uint32_t urisNum;
    char * pUserName;
    uint32_t userNameLength;
} SignalIceServer_t;

typedef struct SignalIceConfigMessage
{
    SignalIceServer_t iceServer[AWS_ICE_SERVER_MAX_NUM];
    uint32_t iceServerNum;
} SignalIceConfigMessage_t;

/*-----------------------------------------------------------*/

SignalResult_t Signal_createSignal( SignalContext_t *pCtx, char * pAwsRegion, uint32_t awsRegionLength, char * pControlPlaneUrl, uint32_t controlPlaneUrlLength );

SignalResult_t Signal_setChannelEndpointHttps( SignalContext_t *pCtx, char * pChannelEndpointHttps, uint32_t channelEndpointHttpsLength );

SignalResult_t Signal_setChannelArn( SignalContext_t *pCtx, char * pChannelArn, uint32_t channelArnLength );

SignalResult_t Signal_setClientId( SignalContext_t *pCtx, char * pClientId, uint32_t clientIdLength );

SignalResult_t Signal_getIceConfig( SignalContext_t *pCtx, char * pUrl, uint32_t * pUrlLength, char *pBody, uint32_t * pBodyLength );

SignalResult_t Signal_parseIceConfigMessage( SignalContext_t *pCtx, char * pMessage, uint32_t messageLength, SignalIceConfigMessage_t *pIceConfigMessage );

/*-----------------------------------------------------------*/

#endif /* SIGNAL_API_H */
