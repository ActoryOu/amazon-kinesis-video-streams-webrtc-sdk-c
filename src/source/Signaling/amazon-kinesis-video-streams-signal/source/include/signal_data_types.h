#ifndef SIGNAL_DATA_TYPES_H
#define SIGNAL_DATA_TYPES_H

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>

/*-----------------------------------------------------------*/

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

#define SIGNAL_STRING_UNKNOWN "UNKOWN"
#define SIGNAL_STRING_SINGLE_MASTER "SINGLE_MASTER"
#define SIGNAL_STRING_MASTER "MASTER"
#define SIGNAL_STRING_VIEWER "VIEWER"

#define AWS_DESCRIBE_SIGNALING_CHANNEL_API_POSTFIX "/describeSignalingChannel"
#define AWS_DESCRIBE_MEDIA_STORAGE_CONF_API_POSTFIX "/describeMediaStorageConfiguration"
#define AWS_CREATE_SIGNALING_CHANNEL_API_POSTFIX "/createSignalingChannel"
#define AWS_GET_SIGNALING_CHANNEL_ENDPOINT_API_POSTFIX "/getSignalingChannelEndpoint"
#define AWS_GET_ICE_CONFIG_API_POSTFIX "/v1/get-ice-server-config"
#define AWS_JOIN_STORAGE_SESSION_API_POSTFIX "/joinStorageSession"

// Parameterized string for Describe Channel API
#define AWS_DESCRIBE_CHANNEL_PARAM_JSON_TEMPLATE    "{\n\t\"ChannelName\": \"%.*s\"\n}"

// Parameterized string for Desceibe Media Storage Config API
#define AWS_DESCRIBE_MEDIA_STORAGE_CONF_PARAM_JSON_TEMPLATE "{\n\t\"ChannelARN\": \"%.*s\"\n}"

// Parameterized string for Create Channel API
#define AWS_CREATE_CHANNEL_PARAM_JSON_TEMPLATE_PREFIX                                                                                                           \
    "{\n\t\"ChannelName\": \"%.*s\","                                                                                                                  \
    "\n\t\"ChannelType\": \"%s\","                                                                                                                   \
    "\n\t\"SingleMasterConfiguration\": {"                                                                                                           \
    "\n\t\t\"MessageTtlSeconds\": %u\n\t}"

#define AWS_CREATE_CHANNEL_PARAM_JSON_TEMPLATE_POSTFIX "\n}"

#define AWS_CREATE_CHANNEL_PARAM_JSON_TAGS_PREFIX ",\n\t\"Tags\": ["
#define AWS_CREATE_CHANNEL_PARAM_JSON_TAGS_POSTFIX "\n\t]"
#define AWS_CREATE_CHANNEL_PARAM_JSON_TAGS_TEMPLATE \
    "\n\t\t{\n\t\t\t\"Key\": \"%.*s\"," \
    "\n\t\t\t\"Value\": \"%.*s\"\n\t\t}"

// Parameterized string for Get Channel Endpoint API
#define AWS_GET_CHANNEL_ENDPOINT_PARAM_JSON_TEMPLATE                                                                                                     \
    "{\n\t\"ChannelARN\": \"%.*s\","                                                                                                                   \
    "\n\t\"SingleMasterChannelEndpointConfiguration\": {"                                                                                            \
    "\n\t\t\"Protocols\": [%s],"                                                                                                                     \
    "\n\t\t\"Role\": \"%s\""                                                                                                                         \
    "\n\t}\n}"

// Parameterized string for Get Ice Server Config API
#define AWS_GET_ICE_CONFIG_PARAM_JSON_TEMPLATE                                                                                                           \
    "{\n\t\"ChannelARN\": \"%.*s\","                                                                                                                   \
    "\n\t\"ClientId\": \"%.*s\","                                                                                                                      \
    "\n\t\"Service\": \"TURN\""                                                                                                                      \
    "\n}"

#define AWS_JOIN_STORAGE_SESSION_MASTER_PARAM_JSON_TEMPLATE "{\n\t\"channelArn\": \"%.*s\"\n}"
#define AWS_JOIN_STORAGE_SESSION_VIEWER_PARAM_JSON_TEMPLATE                                                                                    \
    "{\n\t\"channelArn\": \"%.*s\","                                                                                                                   \
    "\n\t\"clientId\": \"%.*s\"\n}"

#define AWS_REGION_MAX_LENGTH ( 50 )
#define AWS_CONTROL_PLANE_URL_MAX_LENGTH ( 256 )
#define AWS_SIGNALING_CLIENT_ID_MAX_LENGTH ( 256 )

/**
 * Max ARN len in chars
 * https://docs.aws.amazon.com/kinesisvideostreams/latest/dg/API_DescribeSignalingChannel.html#API_DescribeSignalingChannel_RequestSyntax
 * https://docs.aws.amazon.com/kinesisvideostreams/latest/dg/API_CreateStream.html#KinesisVideo-CreateStream-request-KmsKeyId
 */
#define AWS_MAX_ARN_LEN ( 1024 )
#define AWS_MAX_CHANNEL_NAME_LEN ( 256 )

#define AWS_ICE_SERVER_MAX_NUM ( 5 )
#define AWS_ICE_SERVER_MAX_URIS ( 5 )

#define AWS_MESSAGE_TTL_SECONDS_BUFFER_MAX ( 5 )
#define AWS_MESSAGE_TTL_SECONDS_MIN ( 5 )
#define AWS_MESSAGE_TTL_SECONDS_MAX ( 120 )

typedef enum SignalResult
{
    SIGNAL_RESULT_OK,
    SIGNAL_RESULT_BASE = 0x52000000,
    SIGNAL_RESULT_BAD_PARAM,
    SIGNAL_RESULT_SNPRINTF_ERROR,
    SIGNAL_RESULT_OUT_OF_MEMORY,
    SIGNAL_RESULT_INVALID_JSON,
    SIGNAL_RESULT_NOT_EXPECT_RESPONSE,
    SIGNAL_RESULT_INVALID_TTL,
    SIGNAL_RESULT_INVALID_ENDPOINT,
    SIGNAL_RESULT_INVALID_CHANNEL_NAME,
    SIGNAL_RESULT_INVALID_CHANNEL_TYPE,
    SIGNAL_RESULT_PARSE_NEXT_LAYER,
} SignalResult_t;

typedef enum SignalChannelEndpointProtocol
{
    SIGNAL_ENDPOINT_PROTOCOL_NONE = 0,
    SIGNAL_ENDPOINT_PROTOCOL_WEBSOCKET_SECURE = 1,
    SIGNAL_ENDPOINT_PROTOCOL_HTTPS = 2,
    SIGNAL_ENDPOINT_PROTOCOL_WEBRTC = 4,
    SIGNAL_ENDPOINT_PROTOCOL_MAX = 0xFF,
} SignalChannelEndpointProtocol_t;

/**
 * @brief Channel type
 */
typedef enum SignalChannelType {
    SIGNAL_CHANNEL_TYPE_UNKNOWN,       //!< Channel type is unknown
    SIGNAL_CHANNEL_TYPE_SINGLE_MASTER, //!< Channel type is master
} SignalChannelType_t;

typedef enum SignalRole
{
    SIGNAL_ROLE_NONE = 0,
    SIGNAL_ROLE_MASTER,
    SIGNAL_ROLE_VIEWER,
} SignalRole_t;

typedef struct SignalChannelInfo
{
    const char * pChannelArn;
    size_t channelArnLength;
    const char * pChannelName;
    size_t channelNameLength;
    const char * pChannelStatus;
    size_t channelStatusLength;
    SignalChannelType_t channelType;
    const char * pVersion;
    size_t versionLength;
    const char * pCreationTime;
    size_t creationTimeLength;
    uint16_t messageTtlSeconds;
} SignalChannelInfo_t;

typedef struct SignalEndpoints
{
    const char * pEndpointWebsocketSecure;
    size_t endpointWebsocketSecureLength;
    const char * pEndpointHttps;
    size_t endpointHttpsLength;
    const char * pEndpointWebrtc;
    size_t endpointWebrtcLength;
} SignalEndpoints_t;

typedef struct SignalContext
{
    char region[AWS_REGION_MAX_LENGTH];
    size_t regionLength;
    char controlPlaneUrl[AWS_CONTROL_PLANE_URL_MAX_LENGTH];
    size_t controlPlaneUrlLength;
    SignalRole_t roleType;
    char channelName[AWS_MAX_CHANNEL_NAME_LEN];
    size_t channelNameLength;
} SignalContext_t;

/**
 * Tag declaration
 */
typedef struct SignalTag {
    char * pName;
    size_t nameLength;
    char * pValue;
    size_t valueLength;
} SignalTag_t;

typedef struct SignalCreate {
    char * pRegion;
    size_t regionLength;
    char * pControlPlaneUrl;
    size_t controlPlaneUrlLength;
} SignalCreate_t;

typedef struct SignalDescribeChannelRequest
{
    char * pChannelName;
    size_t channelNameLength;
} SignalDescribeChannelRequest_t;

typedef struct SignalCreateChannel {
    SignalChannelInfo_t channelInfo;
    SignalTag_t * pTags;
    size_t tagsCount;
} SignalCreateChannel_t;

typedef struct SignalDescribeChannel
{
    const char * pChannelArn;
    size_t channelArnLength;
    const char * pChannelName;
    size_t channelNameLength;
    const char * pChannelStatus;
    size_t channelStatusLength;
    const char * pChannelType;
    size_t channelTypeLength;
    const char * pVersion;
    size_t versionLength;
    const char * pCreationTime;
    size_t creationTimeLength;
    uint16_t messageTtlSeconds;
} SignalDescribeChannel_t;

typedef struct SignalMediaStorageConfigRequest
{
    char * pChannelArn;
    size_t channelArnLength;
} SignalMediaStorageConfigRequest_t;

typedef struct SignalMediaStorageConfig
{
    const char * pStatus;
    size_t statusLength;
    const char * pStreamArn;
    size_t streamArnLength;
} SignalMediaStorageConfig_t;

typedef struct SignalIceServer
{
    const char * pPassword;
    size_t passwordLength;
    const char * pTtl;
    size_t ttlLength;
    const char * pUris[AWS_ICE_SERVER_MAX_URIS];
    size_t urisLength[AWS_ICE_SERVER_MAX_URIS];
    uint32_t urisNum;
    const char * pUserName;
    size_t userNameLength;
} SignalIceServer_t;

typedef struct SignalIceConfigMessage
{
    SignalIceServer_t iceServer[AWS_ICE_SERVER_MAX_NUM];
    uint32_t iceServerNum;
} SignalIceConfigMessage_t;

typedef struct SignalGetChannelEndpointRequest
{
    char * pChannelArn;
    size_t channelArnLength;
    uint8_t protocolsBitsMap; /* support multiple propocols. */
    SignalRole_t role;
} SignalGetChannelEndpointRequest_t;

typedef struct SignalIceConfigRequest
{
    char * pChannelArn;
    size_t channelArnLength;
    char * pEndpointHttps;
    size_t endpointHttpsLength;
    char * pClientId;
    size_t clientIdLength;
} SignalIceConfigRequest_t;

typedef struct SignalJoinStorageSessionRequest
{
    const char * pEndpointWebrtc;
    size_t endpointWebrtcLength;
    char * pChannelArn;
    size_t channelArnLength;
    SignalRole_t role;
    char * pClientId;
    size_t clientIdLength;
} SignalJoinStorageSessionRequest_t;

/*-----------------------------------------------------------*/

#endif /* SIGNAL_DATA_TYPES_H */
