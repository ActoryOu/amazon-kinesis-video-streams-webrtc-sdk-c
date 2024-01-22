#ifndef SIGNAL_DATA_TYPES_H
#define SIGNAL_DATA_TYPES_H

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>

/*-----------------------------------------------------------*/

typedef enum SignalResult
{
    SIGNAL_RESULT_OK,
    SIGNAL_RESULT_BASE = 0x52000000,
    SIGNAL_RESULT_BAD_PARAM,
    SIGNAL_RESULT_SNPRINTF_ERROR,
    SIGNAL_RESULT_OUT_OF_MEMORY,
    SIGNAL_RESULT_INVALID_JSON,
    SIGNAL_RESULT_PARSE_NEXT_LAYER,
} SignalResult_t;

/**
 * Default AWS region
 */
#define AWS_DEFAULT_REGION "us-west-2"

/**
 * Control plane prefix
 */
#define AWS_CONTROL_PLANE_URI_PREFIX "https://"

/**
 * KVS service name
 */
#define AWS_KINESIS_VIDEO_SERVICE_NAME "kinesisvideo"

/**
 * Control plane postfix
 */
#define AWS_CONTROL_PLANE_URI_POSTFIX ".amazonaws.com"

#define AWS_GET_ICE_CONFIG_API_POSTFIX "/v1/get-ice-server-config"
#define AWS_DESCRIBE_SIGNALING_CHANNEL_API_POSTFIX "/describeSignalingChannel"
#define AWS_DESCRIBE_MEDIA_STORAGE_CONF_API_POSTFIX "/describeMediaStorageConfiguration"

// Parameterized string for Get Ice Server Config API
#define AWS_GET_ICE_CONFIG_PARAM_JSON_TEMPLATE                                                                                                           \
    "{\n\t\"ChannelARN\": \"%s\","                                                                                                                   \
    "\n\t\"ClientId\": \"%s\","                                                                                                                      \
    "\n\t\"Service\": \"TURN\""                                                                                                                      \
    "\n}"

// Parameterized string for Describe Channel API
#define AWS_DESCRIBE_CHANNEL_PARAM_JSON_TEMPLATE    "{\n\t\"ChannelName\": \"%s\"\n}"

// Parameterized string for Desceibe Media Storage Config API
#define AWS_DESCRIBE_MEDIA_STORAGE_CONF_PARAM_JSON_TEMPLATE "{\n\t\"ChannelARN\": \"%s\"\n}"

#define AWS_REGION_MAX_LENGTH ( 50 )
#define AWS_CONTROL_PLANE_URL_MAX_LENGTH ( 256 )
#define AWS_SIGNALING_CLIENT_ID_MAX_LENGTH ( 256 )

/**
 * Max ARN len in chars
 * https://docs.aws.amazon.com/kinesisvideostreams/latest/dg/API_DescribeSignalingChannel.html#API_DescribeSignalingChannel_RequestSyntax
 * https://docs.aws.amazon.com/kinesisvideostreams/latest/dg/API_CreateStream.html#KinesisVideo-CreateStream-request-KmsKeyId
 */
#define AWS_MAX_ARN_LEN ( 2048 )
#define AWS_MAX_CHANNEL_NAME_LEN ( 256 )

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
    char channelName[AWS_MAX_CHANNEL_NAME_LEN];
    uint32_t channelNameLength;
    char channelArn[AWS_MAX_ARN_LEN];
    uint32_t channelArnLength;
    char clientId[AWS_SIGNALING_CLIENT_ID_MAX_LENGTH];
    uint32_t clientIdLength;
} SignalContext_t;

typedef struct SignalDescribeChannel
{
    const char * pChannelArn;
    uint32_t channelArnLength;
    const char * pChannelName;
    uint32_t channelNameLength;
    const char * pChannelStatus;
    uint32_t channelStatusLength;
    const char * pChannelType;
    uint32_t channelTypeLength;
    const char * pVersion;
    uint32_t versionLength;
    const char * pCreationTime;
    uint32_t creationTimeLength;
    const char * pMessageTtlSeconds;
    uint32_t messageTtlSecondsLength;
} SignalDescribeChannel_t;

typedef struct SignalMediaStorageConfig
{
    const char * pStatus;
    uint32_t statusLength;
    const char * pStreamArn;
    uint32_t streamArnLength;
} SignalMediaStorageConfig_t;

typedef struct SignalIceServer
{
    const char * pPassword;
    uint32_t passwordLength;
    const char * pTtl;
    uint32_t ttlLength;
    const char * pUris[AWS_ICE_SERVER_MAX_URIS];
    uint32_t urisLength[AWS_ICE_SERVER_MAX_URIS];
    uint32_t urisNum;
    const char * pUserName;
    uint32_t userNameLength;
} SignalIceServer_t;

typedef struct SignalIceConfigMessage
{
    SignalIceServer_t iceServer[AWS_ICE_SERVER_MAX_NUM];
    uint32_t iceServerNum;
} SignalIceConfigMessage_t;

/*-----------------------------------------------------------*/

#endif /* SIGNAL_DATA_TYPES_H */
