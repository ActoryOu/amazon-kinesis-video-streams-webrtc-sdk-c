/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "signal_api.h"
#include "core_json.h"

/* AWS_GETENDPOINT_PROTOCOL_MAX_STRING is ["WSS", "HTTPS", "WEBRTC"] */
#define AWS_GETENDPOINT_PROTOCOL_MAX_STRING_LENGTH ( 25 ) /* with NULL terminator */
#define AWS_STRING_QUOTE_WSS "\"WSS\""
#define AWS_STRING_LENGTH_QUOTE_WSS ( 5 )
#define AWS_STRING_QUOTE_HTTPS "\"HTTPS\""
#define AWS_STRING_LENGTH_QUOTE_HTTPS ( 7 )
#define AWS_STRING_QUOTE_WEBRTC "\"WEBRTC\""
#define AWS_STRING_LENGTH_QUOTE_WEBRTC ( 8 )

static SignalChannelEndpointProtocol_t getProtocolFromString( const char *pProtocolString, size_t protocolStringLength )
{
    SignalChannelEndpointProtocol_t ret = SIGNAL_ENDPOINT_PROTOCOL_NONE;

    if ( strncmp(pProtocolString, "WSS", 3) == 0 ||
         strncmp(pProtocolString, "wss", 3) == 0 ) {
        ret = SIGNAL_ENDPOINT_PROTOCOL_WEBSOCKET_SECURE;
    } else if ( strncmp(pProtocolString, "HTTPS", 5) == 0 ||
         strncmp(pProtocolString, "https", 5) == 0 ) {
        ret = SIGNAL_ENDPOINT_PROTOCOL_HTTPS;
    } else if ( strncmp(pProtocolString, "WEBRTC", 6) == 0 ||
         strncmp(pProtocolString, "webrtc", 6) == 0 ) {
        ret = SIGNAL_ENDPOINT_PROTOCOL_WEBRTC;
    } else {
        /* Do nothing, coverity happy. */
    }

    return ret;
}

static void updateUris(SignalIceServer_t *pIceServer, const char * pUris, size_t urisLength)
{
    JSONStatus_t jsonResult = JSONSuccess;
    size_t start = 0, next = 0;
    JSONPair_t pair = { 0 };

    jsonResult = JSON_Iterate( pUris, urisLength, &start, &next, &pair );
    
    while (jsonResult == JSONSuccess) {
        pIceServer->pUris[pIceServer->urisNum] = pair.value;
        pIceServer->urisLength[pIceServer->urisNum] = pair.valueLength;
        pIceServer->urisNum++;
        
        jsonResult = JSON_Iterate( pUris, urisLength, &start, &next, &pair );
    }
}

SignalResult_t Signal_createSignal( SignalContext_t *pCtx, SignalCreate_t *pCreate )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length = 0;

    /* input check */
    if (pCtx == NULL || pCreate == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    /* Set region. */
    if (result == SIGNAL_RESULT_OK) {
        memset(pCtx, 0, sizeof(SignalContext_t));

        if (pCreate->pRegion == NULL) {
            // TODO: Add error handling
            pCtx->regionLength = strlen(AWS_DEFAULT_REGION);
            memcpy(pCtx->region, AWS_DEFAULT_REGION, pCtx->regionLength);
        } else {
            // TODO: Add error handling
            pCtx->regionLength = pCreate->regionLength;
            memcpy(pCtx->region, pCreate->pRegion, pCtx->regionLength);
        }
    }

    /* Set control plane. */
    if (result == SIGNAL_RESULT_OK) {
        if (pCreate->pControlPlaneUrl == NULL) {
            length = snprintf(pCtx->controlPlaneUrl, AWS_CONTROL_PLANE_URL_MAX_LENGTH, "%s%s.%s%s",
                              AWS_CONTROL_PLANE_URI_PREFIX, AWS_KINESIS_VIDEO_SERVICE_NAME, 
                              pCtx->region, AWS_CONTROL_PLANE_URI_POSTFIX);

            if (length < 0) { //LCOV_EXCL_BR_LINE
                result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
            }
            else if ((size_t)length >= AWS_CONTROL_PLANE_URL_MAX_LENGTH) {
                result = SIGNAL_RESULT_OUT_OF_MEMORY;
            }
            else {
                pCtx->controlPlaneUrlLength = length;
            }
        } else {
            // TODO: Add error handling
            pCtx->controlPlaneUrlLength = pCreate->controlPlaneUrlLength;
            memcpy(pCtx->controlPlaneUrl, pCreate->pControlPlaneUrl, pCtx->controlPlaneUrlLength);
        }
    }

    return result;
}

SignalResult_t Signal_getDescribeChannelRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalDescribeChannelRequest_t * pDescribeChannelRequest )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL || pDescribeChannelRequest == NULL ||
        pDescribeChannelRequest->pChannelName == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%s",
                          (int) pCtx->controlPlaneUrlLength, pCtx->controlPlaneUrl,
                          AWS_DESCRIBE_SIGNALING_CHANNEL_API_POSTFIX);

        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pUrlLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pUrlLength = length;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        // Prepare the body for the call
        length = snprintf(pBody, *pBodyLength, AWS_DESCRIBE_CHANNEL_PARAM_JSON_TEMPLATE,
                          (int) pDescribeChannelRequest->channelNameLength, pDescribeChannelRequest->pChannelName);
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pBodyLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength = length;
        }
    }

    return result;
}

SignalResult_t Signal_parseDescribeChannelMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalDescribeChannel_t *pDescribeChannel )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    JSONStatus_t jsonResult;
    size_t start = 0, next = 0;
    JSONPair_t pair = { 0 };
    const char *pChannelInfoBuffer;
    size_t channelInfoBufferLength;
    size_t channelInfoStart = 0, channelInfoNext = 0;
    char ttlSecondsBuffer[AWS_MESSAGE_CHANNEL_TTL_SECONDS_BUFFER_MAX] = { 0 };

    /* input check */
    if (pCtx == NULL || pMessage == NULL || pDescribeChannel == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        jsonResult = JSON_Validate(pMessage, messageLength);

        if (jsonResult != JSONSuccess) {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        memset(pDescribeChannel, 0, sizeof(SignalDescribeChannel_t));

        /* Check if it's ChannelInfo. */
        jsonResult = JSON_Iterate( pMessage, messageLength, &start, &next, &pair );

        if (jsonResult == JSONSuccess) {
            if (pair.jsonType != JSONObject || 
                pair.keyLength != strlen("ChannelInfo") || 
                strncmp(pair.key, "ChannelInfo", pair.keyLength)) {
                /* Not an ice server list meesage. */
                result = SIGNAL_RESULT_NOT_EXPECT_RESPONSE;
            } else {
                pChannelInfoBuffer = pair.value;
                channelInfoBufferLength = pair.valueLength;
            }
        } else {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        jsonResult = JSON_Iterate(pChannelInfoBuffer, channelInfoBufferLength, &channelInfoStart, &channelInfoNext, &pair);

        while (jsonResult == JSONSuccess) {
            if (strncmp(pair.key, "ChannelARN", pair.keyLength) == 0) {
                pDescribeChannel->pChannelArn = pair.value;
                pDescribeChannel->channelArnLength = pair.valueLength;
            } else if (strncmp(pair.key, "ChannelName", pair.keyLength) == 0) {
                if (pair.valueLength < AWS_MAX_CHANNEL_NAME_LEN) {
                    pDescribeChannel->pChannelName = pair.value;
                    pDescribeChannel->channelNameLength = pair.valueLength;

                    /* Store channel name in context. */
                    pCtx->channelNameLength = pDescribeChannel->channelNameLength;
                    memcpy(pCtx->channelName, pDescribeChannel->pChannelName, pCtx->channelNameLength);
                    pCtx->channelName[pCtx->channelNameLength] = '\0';
                } else {
                    result = SIGNAL_RESULT_INVALID_CHANNEL_NAME;
                }
            } else if (strncmp(pair.key, "ChannelStatus", pair.keyLength) == 0) {
                pDescribeChannel->pChannelStatus = pair.value;
                pDescribeChannel->channelStatusLength = pair.valueLength;
            } else if (strncmp(pair.key, "ChannelType", pair.keyLength) == 0) {
                pDescribeChannel->pChannelType = pair.value;
                pDescribeChannel->channelTypeLength = pair.valueLength;

                /* Store channel type in context. */
                if (strncmp(pDescribeChannel->pChannelType, "SINGLE_MASTER", pDescribeChannel->channelTypeLength) != 0) {
                    result = SIGNAL_RESULT_INVALID_CHANNEL_TYPE;
                }
            } else if (strncmp(pair.key, "CreationTime", pair.keyLength) == 0) {
                // TODO: In the future parse out the creation time but currently we don't need it
            } else if (strncmp(pair.key, "SingleMasterConfiguration", pair.keyLength) == 0) {
                const char *pBuffer = pair.value;
                size_t bufferLength = pair.valueLength;
                size_t bufferStart = 0, bufferNext = 0;

                jsonResult = JSON_Iterate( pBuffer, bufferLength, &bufferStart, &bufferNext, &pair );

                if (jsonResult == JSONSuccess) {
                    if (strncmp(pair.key, "MessageTtlSeconds", pair.keyLength) == 0) {
                        if( pair.valueLength >= AWS_MESSAGE_CHANNEL_TTL_SECONDS_BUFFER_MAX ) {
                            /* Unexpect TTL value from cloud. */
                            result = SIGNAL_RESULT_INVALID_TTL;
                            break;
                        }

                        strncpy(ttlSecondsBuffer, pair.value, pair.valueLength);
                        pDescribeChannel->messageTtlSeconds = (uint32_t) strtoul(ttlSecondsBuffer, NULL, 10);

                        if (pDescribeChannel->messageTtlSeconds < AWS_MESSAGE_CHANNEL_TTL_SECONDS_MIN || pDescribeChannel->messageTtlSeconds > AWS_MESSAGE_CHANNEL_TTL_SECONDS_MAX) {
                            /* Unexpect TTL value from cloud. */
                            result = SIGNAL_RESULT_INVALID_TTL;
                        }
                    } else {
                        // unknown attribute
                        result = SIGNAL_RESULT_INVALID_JSON;
                    }
                } else {
                    // invalid single master configuration
                    result = SIGNAL_RESULT_INVALID_JSON;
                }
            } else if (strncmp(pair.key, "Version", pair.keyLength) == 0) {
                pDescribeChannel->pVersion = pair.value;
                pDescribeChannel->versionLength = pair.valueLength;
            } else {
                /* Skip unknown attributes. */
            }
            
            if (result != SIGNAL_RESULT_OK) {
                break;
            }

            jsonResult = JSON_Iterate(pChannelInfoBuffer, channelInfoBufferLength, &channelInfoStart, &channelInfoNext, &pair);
        }
    }

    return result;
}

SignalResult_t Signal_getMediaStorageConfigRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalMediaStorageConfigRequest_t * pMediaStorageConfigRequest )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL || pMediaStorageConfigRequest == NULL ||
        pMediaStorageConfigRequest->pChannelArn == NULL ) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%s",
                          (int) pCtx->controlPlaneUrlLength, pCtx->controlPlaneUrl,
                          AWS_DESCRIBE_MEDIA_STORAGE_CONF_API_POSTFIX);

        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pUrlLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pUrlLength = length;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        // Prepare the body for the call
        length = snprintf(pBody, *pBodyLength, AWS_DESCRIBE_MEDIA_STORAGE_CONF_PARAM_JSON_TEMPLATE,
                          (int) pMediaStorageConfigRequest->channelArnLength, pMediaStorageConfigRequest->pChannelArn);
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pBodyLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength = length;
        }
    }

    return result;
}

SignalResult_t Signal_parseMediaStorageConfigMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalMediaStorageConfig_t *pMediaStorageConfig )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    JSONStatus_t jsonResult;
    size_t start = 0, next = 0;
    JSONPair_t pair = { 0 };
    const char *pMediaStorageConfigBuffer;
    size_t mediaStorageConfigBufferLength;
    size_t mediaStorageConfigStart = 0, mediaStorageConfigNext = 0;

    /* input check */
    if (pCtx == NULL || pMessage == NULL || pMediaStorageConfig == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        jsonResult = JSON_Validate(pMessage, messageLength);

        if (jsonResult != JSONSuccess) {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        memset(pMediaStorageConfig, 0, sizeof(SignalMediaStorageConfig_t));

        /* Check if it's MediaStorageConfiguration. */
        jsonResult = JSON_Iterate( pMessage, messageLength, &start, &next, &pair );

        if (jsonResult == JSONSuccess) {
            if (pair.jsonType != JSONArray || 
                pair.keyLength != strlen("MediaStorageConfiguration") || 
                strncmp(pair.key, "MediaStorageConfiguration", pair.keyLength)) {
                /* Not an ice server list meesage. */
                result = SIGNAL_RESULT_NOT_EXPECT_RESPONSE;
            } else {
                pMediaStorageConfigBuffer = pair.value;
                mediaStorageConfigBufferLength = pair.valueLength;
            }
        } else {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        jsonResult = JSON_Iterate(pMediaStorageConfigBuffer, mediaStorageConfigBufferLength, &mediaStorageConfigStart, &mediaStorageConfigNext, &pair);

        while (jsonResult == JSONSuccess) {
            if (strncmp(pair.key, "Status", pair.keyLength) == 0) {
                pMediaStorageConfig->pStatus = pair.value;
                pMediaStorageConfig->statusLength = pair.valueLength;
            } else if (strncmp(pair.key, "StreamARN", pair.keyLength) == 0) {
                pMediaStorageConfig->pStreamArn = pair.value;
                pMediaStorageConfig->streamArnLength = pair.valueLength;
            } else {
                /* Skip unknown attributes. */
            }
            
            jsonResult = JSON_Iterate(pMediaStorageConfigBuffer, mediaStorageConfigBufferLength, &mediaStorageConfigStart, &mediaStorageConfigNext, &pair);
        }
    }

    return result;
}

SignalResult_t Signal_getCreateChannelRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalCreateChannel_t * pCreateChannel )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;
    char *pCurBody = pBody;
    size_t remainingLength = 0;
    int i;
    SignalTag_t *pCurTags = pCreateChannel->pTags;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL ||
        (pCreateChannel->tagsCount > 0 && pCreateChannel->pTags == NULL) ||
        pCreateChannel->channelInfo.channelNameLength >= AWS_MAX_CHANNEL_NAME_LEN) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%s",
                          (int) pCtx->controlPlaneUrlLength, pCtx->controlPlaneUrl,
                          AWS_CREATE_SIGNALING_CHANNEL_API_POSTFIX);

        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pUrlLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pUrlLength = length;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        // Prepare the body for the call
        remainingLength = *pBodyLength;

        length = snprintf(pCurBody, remainingLength, AWS_CREATE_CHANNEL_PARAM_JSON_TEMPLATE_PREFIX,
                          (int) pCreateChannel->channelInfo.channelNameLength, pCreateChannel->channelInfo.pChannelName,
                          pCreateChannel->channelInfo.channelType == SIGNAL_CHANNEL_TYPE_SINGLE_MASTER? SIGNAL_STRING_SINGLE_MASTER:SIGNAL_STRING_UNKNOWN,
                          pCreateChannel->channelInfo.messageTtlSeconds);
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= remainingLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength = length;
            remainingLength -= length;
            pCurBody += length;
        }
    }

    /* Append tags prefix. */
    if (result == SIGNAL_RESULT_OK && pCreateChannel->tagsCount > 0) {
        length = snprintf(pCurBody, remainingLength, AWS_CREATE_CHANNEL_PARAM_JSON_TAGS_PREFIX);
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= remainingLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength += length;
            remainingLength -= length;
            pCurBody += length;
        }
    }

    /* Append tags content. */
    if (result == SIGNAL_RESULT_OK && pCreateChannel->tagsCount > 0) {
        for (i = 0; i < pCreateChannel->tagsCount; i++) {
            if (i == 0) {
                length = snprintf(pCurBody, remainingLength, AWS_CREATE_CHANNEL_PARAM_JSON_TAGS_TEMPLATE,
                                  (int) (pCurTags + i)->nameLength, (pCurTags + i)->pName,
                                  (int) (pCurTags + i)->valueLength, (pCurTags + i)->pValue);
            } else {
                length = snprintf(pCurBody, remainingLength, "," AWS_CREATE_CHANNEL_PARAM_JSON_TAGS_TEMPLATE,
                                  (int) (pCurTags + i)->nameLength, (pCurTags + i)->pName,
                                  (int) (pCurTags + i)->valueLength, (pCurTags + i)->pValue);
            }

            if (length < 0) { //LCOV_EXCL_BR_LINE
                result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
            }
            else if (length >= remainingLength) {
                result = SIGNAL_RESULT_OUT_OF_MEMORY;
            }
            else {
                *pBodyLength += length;
                remainingLength -= length;
                pCurBody += length;
            }
        }
    }

    /* Append tags postfix. */
    if (result == SIGNAL_RESULT_OK && pCreateChannel->tagsCount > 0) {
        length = snprintf(pCurBody, remainingLength, AWS_CREATE_CHANNEL_PARAM_JSON_TAGS_POSTFIX);
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= remainingLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength += length;
            remainingLength -= length;
            pCurBody += length;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        // Prepare the body for the call
        length = snprintf(pCurBody, remainingLength, AWS_CREATE_CHANNEL_PARAM_JSON_TEMPLATE_POSTFIX);
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= remainingLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength += length;
            remainingLength -= length;
            pCurBody += length;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        /* Store channel information in context. */
        pCtx->channelNameLength = pCreateChannel->channelInfo.channelNameLength;
        memcpy(pCtx->channelName, pCreateChannel->channelInfo.pChannelName, pCtx->channelNameLength);
        pCtx->channelName[pCtx->channelNameLength] = '\0';
    }

    return result;
}

SignalResult_t Signal_parseCreateChannelMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalCreateChannel_t *pCreateChannel )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    JSONStatus_t jsonResult;
    size_t start = 0, next = 0;
    JSONPair_t pair = { 0 };

    /* input check */
    if (pCtx == NULL || pMessage == NULL || pCreateChannel == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        jsonResult = JSON_Validate(pMessage, messageLength);

        if (jsonResult != JSONSuccess) {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        pCreateChannel->channelInfo.pChannelArn = NULL;
        pCreateChannel->channelInfo.channelArnLength = 0;

        /* Check if it's IceServerList. */
        jsonResult = JSON_Iterate( pMessage, messageLength, &start, &next, &pair );

        if (jsonResult == JSONSuccess) {
            if (strncmp(pair.key, "ChannelARN", pair.keyLength) == 0) {
                pCreateChannel->channelInfo.pChannelArn = pair.value;
                pCreateChannel->channelInfo.channelArnLength = pair.valueLength;
            }
        } else {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    return result;
}

SignalResult_t Signal_getChannelEndpointRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalGetChannelEndpointRequest_t * pChannelEndpointRequest )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;
    char protocols[AWS_GETENDPOINT_PROTOCOL_MAX_STRING_LENGTH] = { 0 };
    int protocolIndex = 0;
    uint8_t isFirstProtocol = true;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL || pChannelEndpointRequest == NULL ||
        pChannelEndpointRequest->pChannelArn == NULL ||
        (pChannelEndpointRequest->role != SIGNAL_ROLE_MASTER && pChannelEndpointRequest->role != SIGNAL_ROLE_VIEWER)) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%s",
                          (int) pCtx->controlPlaneUrlLength, pCtx->controlPlaneUrl,
                          AWS_GET_SIGNALING_CHANNEL_ENDPOINT_API_POSTFIX);

        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pUrlLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pUrlLength = length;
        }
    }

    /* Prepare protocol string. */
    if (result == SIGNAL_RESULT_OK) {
        if ((pChannelEndpointRequest->protocolsBitsMap & SIGNAL_ENDPOINT_PROTOCOL_WEBSOCKET_SECURE) != 0) {
            strcpy(protocols, AWS_STRING_QUOTE_WSS);
            protocolIndex += AWS_STRING_LENGTH_QUOTE_WSS;

            isFirstProtocol = false;
        }

        if ((pChannelEndpointRequest->protocolsBitsMap & SIGNAL_ENDPOINT_PROTOCOL_HTTPS) != 0) {
            if (isFirstProtocol == false) {
                strcpy(protocols + protocolIndex, ", ");
                protocolIndex += 2;
            }
            strcpy(protocols + protocolIndex, AWS_STRING_QUOTE_HTTPS);
            protocolIndex += AWS_STRING_LENGTH_QUOTE_HTTPS;

            isFirstProtocol = false;
        }

        if ((pChannelEndpointRequest->protocolsBitsMap & SIGNAL_ENDPOINT_PROTOCOL_WEBRTC) != 0) {
            if (isFirstProtocol == false) {
                strcpy(protocols + protocolIndex, ", ");
                protocolIndex += 2;
            }
            strcpy(protocols + protocolIndex, AWS_STRING_QUOTE_WEBRTC);
            protocolIndex += AWS_STRING_LENGTH_QUOTE_WEBRTC;

            isFirstProtocol = false;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        // Prepare the body for the call
        length = snprintf(pBody, *pBodyLength, AWS_GET_CHANNEL_ENDPOINT_PARAM_JSON_TEMPLATE,
                          (int) pChannelEndpointRequest->channelArnLength, pChannelEndpointRequest->pChannelArn,
                          protocols,
                          pChannelEndpointRequest->role == SIGNAL_ROLE_MASTER? SIGNAL_STRING_MASTER:SIGNAL_STRING_VIEWER);
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pBodyLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength = length;
        }
    }

    return result;
}

SignalResult_t Signal_parseChannelEndpointMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalEndpoints_t * pEndpoints )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    JSONStatus_t jsonResult;
    size_t start = 0, next = 0;
    JSONPair_t pair = { 0 };
    const char *pResourceEndpointListBuffer;
    size_t resourceEndpointListBufferLength;
    size_t resourceEndpointStart = 0, resourceEndpointNext = 0;
    const char *pEndpointsBuffer;
    size_t endpointsBufferLength;
    size_t endpointsStart = 0, endpointsNext = 0;
    const char * pTempEndpoint;
    size_t tempEndpointLength;
    SignalChannelEndpointProtocol_t protocol;

    /* input check */
    if (pCtx == NULL || pMessage == NULL || pEndpoints == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        jsonResult = JSON_Validate(pMessage, messageLength);

        if (jsonResult != JSONSuccess) {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        memset(pEndpoints, 0, sizeof(SignalEndpoints_t));

        /* Check if it's resourceEndpoint. */
        jsonResult = JSON_Iterate( pMessage, messageLength, &start, &next, &pair );

        if (jsonResult == JSONSuccess) {
            if (pair.jsonType != JSONArray || 
                pair.keyLength != strlen("ResourceEndpointList") || 
                strncmp(pair.key, "ResourceEndpointList", pair.keyLength)) {
                /* Not an ice server list meesage. */
                result = SIGNAL_RESULT_NOT_EXPECT_RESPONSE;
            } else {
                pResourceEndpointListBuffer = pair.value;
                resourceEndpointListBufferLength = pair.valueLength;
            }
        } else {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    while (result == SIGNAL_RESULT_OK) {
        jsonResult = JSON_Iterate( pResourceEndpointListBuffer, resourceEndpointListBufferLength, &resourceEndpointStart, &resourceEndpointNext, &pair );

        if (jsonResult == JSONSuccess) {
            pEndpointsBuffer = pair.value;
            endpointsBufferLength = pair.valueLength;
            endpointsStart = 0;
            endpointsNext = 0;
            pTempEndpoint = NULL;
            tempEndpointLength = 0;
            protocol = SIGNAL_ENDPOINT_PROTOCOL_NONE;

            jsonResult = JSON_Iterate( pEndpointsBuffer, endpointsBufferLength, &endpointsStart, &endpointsNext, &pair );
            while (jsonResult == JSONSuccess) {
                if (strncmp(pair.key, "Protocol", pair.keyLength) == 0) {
                    protocol = getProtocolFromString(pair.value, pair.valueLength);
                } else if (strncmp(pair.key, "ResourceEndpoint", pair.keyLength) == 0) {
                    pTempEndpoint = pair.value;
                    tempEndpointLength = pair.valueLength;
                }

                jsonResult = JSON_Iterate( pEndpointsBuffer, endpointsBufferLength, &endpointsStart, &endpointsNext, &pair );
            }

            if (pTempEndpoint != NULL && protocol != SIGNAL_ENDPOINT_PROTOCOL_NONE) {
                switch (protocol) {
                    case SIGNAL_ENDPOINT_PROTOCOL_WEBSOCKET_SECURE:
                        pEndpoints->pEndpointWebsocketSecure = pTempEndpoint;
                        pEndpoints->endpointWebsocketSecureLength = tempEndpointLength;
                        break;
                    case SIGNAL_ENDPOINT_PROTOCOL_HTTPS:
                        pEndpoints->pEndpointHttps = pTempEndpoint;
                        pEndpoints->endpointHttpsLength = tempEndpointLength;
                        break;
                    case SIGNAL_ENDPOINT_PROTOCOL_WEBRTC:
                    default:
                        pEndpoints->pEndpointWebrtc = pTempEndpoint;
                        pEndpoints->endpointWebrtcLength = tempEndpointLength;
                        break;
                }
            }
        } else {
            /* All parsed. */
            break;
        }
    }

    return result;
}

SignalResult_t Signal_getIceConfig( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalIceConfigRequest_t * pIceConfigReqeust )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL || pIceConfigReqeust == NULL ||
        pIceConfigReqeust->pClientId == NULL || pIceConfigReqeust->pEndpointHttps == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%s",
                          (int) pIceConfigReqeust->endpointHttpsLength, pIceConfigReqeust->pEndpointHttps,
                          AWS_GET_ICE_CONFIG_API_POSTFIX);

        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pUrlLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pUrlLength = length;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        // Prepare the body for the call
        length = snprintf(pBody, *pBodyLength, AWS_GET_ICE_CONFIG_PARAM_JSON_TEMPLATE,
                          (int) pIceConfigReqeust->channelArnLength, pIceConfigReqeust->pChannelArn,
                          (int) pIceConfigReqeust->clientIdLength, pIceConfigReqeust->pClientId);
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pBodyLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength = length;
        }
    }

    return result;
}

SignalResult_t Signal_parseIceConfigMessage( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalIceConfigMessage_t *pIceConfigMessage )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    JSONStatus_t jsonResult;
    size_t start = 0, next = 0;
    JSONPair_t pair = { 0 };
    const char *pIceServerListBuffer;
    size_t iceServerListBufferLength;
    size_t iceServerListStart = 0, iceServerListNext = 0;
    const char *pIceSingleServerBuffer;
    size_t iceSingleServerBufferLength;
    size_t iceSingleServerStart = 0, iceSingleServerNext = 0;
    char ttlSecondsBuffer[AWS_MESSAGE_ICE_SERVER_TTL_SECONDS_BUFFER_MAX] = { 0 };

    /* input check */
    if (pCtx == NULL || pMessage == NULL || pIceConfigMessage == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        jsonResult = JSON_Validate(pMessage, messageLength);

        if (jsonResult != JSONSuccess) {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        memset(pIceConfigMessage, 0, sizeof(SignalIceConfigMessage_t));

        /* Check if it's IceServerList. */
        jsonResult = JSON_Iterate( pMessage, messageLength, &start, &next, &pair );

        if (jsonResult == JSONSuccess) {
            if (pair.jsonType != JSONArray || 
                pair.keyLength != strlen("IceServerList") || 
                strncmp(pair.key, "IceServerList", pair.keyLength)) {
                /* Not an ice server list meesage. */
                result = SIGNAL_RESULT_NOT_EXPECT_RESPONSE;
            } else {
                pIceServerListBuffer = pair.value;
                iceServerListBufferLength = pair.valueLength;
            }
        } else {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    while (result == SIGNAL_RESULT_OK && pIceConfigMessage->iceServerNum < AWS_ICE_SERVER_MAX_NUM) {
        jsonResult = JSON_Iterate( pIceServerListBuffer, iceServerListBufferLength, &iceServerListStart, &iceServerListNext, &pair );

        if (jsonResult == JSONSuccess) {
            pIceSingleServerBuffer = pair.value;
            iceSingleServerBufferLength = pair.valueLength;
            iceSingleServerStart = 0;
            iceSingleServerNext = 0;

            jsonResult = JSON_Iterate( pIceSingleServerBuffer, iceSingleServerBufferLength, &iceSingleServerStart, &iceSingleServerNext, &pair );
            while (jsonResult == JSONSuccess) {

                if (strncmp(pair.key, "Password", pair.keyLength) == 0) {
                    if (pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pPassword != NULL) {
                        pIceConfigMessage->iceServerNum++;

                        if (pIceConfigMessage->iceServerNum >= AWS_ICE_SERVER_MAX_NUM) {
                            /* Ignore following servers. */
                            break;
                        }
                    }
                    
                    pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pPassword = pair.value;
                    pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].passwordLength = pair.valueLength;
                } else if (strncmp(pair.key, "Ttl", pair.keyLength) == 0) {
                    if( pair.valueLength >= AWS_MESSAGE_ICE_SERVER_TTL_SECONDS_BUFFER_MAX ) {
                        /* Unexpect TTL value from cloud. */
                        result = SIGNAL_RESULT_INVALID_TTL;
                        break;
                    }

                    if (pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].messageTtlSeconds != 0) {
                        pIceConfigMessage->iceServerNum++;

                        if (pIceConfigMessage->iceServerNum >= AWS_ICE_SERVER_MAX_NUM) {
                            /* Ignore following servers. */
                            break;
                        }
                    }

                    strncpy(ttlSecondsBuffer, pair.value, pair.valueLength);
                    pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].messageTtlSeconds = (uint32_t) strtoul(ttlSecondsBuffer, NULL, 10);

                    if (pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].messageTtlSeconds < AWS_MESSAGE_ICE_SERVER_TTL_SECONDS_MIN ||
                        pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].messageTtlSeconds > AWS_MESSAGE_ICE_SERVER_TTL_SECONDS_MAX) {
                        /* Unexpect TTL value from cloud. */
                        result = SIGNAL_RESULT_INVALID_TTL;
                    }
                } else if (strncmp(pair.key, "Uris", pair.keyLength) == 0) {
                    if (pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pUris[0] != NULL) {
                        pIceConfigMessage->iceServerNum++;

                        if (pIceConfigMessage->iceServerNum >= AWS_ICE_SERVER_MAX_NUM) {
                            /* Ignore following servers. */
                            break;
                        }
                    }
                    
                    updateUris(&pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum], pair.value, pair.valueLength);
                } else if (strncmp(pair.key, "Username", pair.keyLength) == 0) {
                    if (pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pUserName != NULL) {
                        pIceConfigMessage->iceServerNum++;

                        if (pIceConfigMessage->iceServerNum >= AWS_ICE_SERVER_MAX_NUM) {
                            /* Ignore following servers. */
                            break;
                        }
                    }
                    
                    pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pUserName = pair.value;
                    pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].userNameLength = pair.valueLength;
                } else {
                    /* Skip unknown messages. */
                }

                jsonResult = JSON_Iterate( pIceSingleServerBuffer, iceSingleServerBufferLength, &iceSingleServerStart, &iceSingleServerNext, &pair );
            }

        } else {
            /* All parsed. */
            pIceConfigMessage->iceServerNum++;
            break;
        }
    }

    return result;
}

SignalResult_t Signal_getJoinStorageSessionRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalJoinStorageSessionRequest_t * pJoinStorageSessionRequest )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL || pJoinStorageSessionRequest == NULL ||
        pJoinStorageSessionRequest->pChannelArn == NULL || pJoinStorageSessionRequest->pEndpointWebrtc == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%s",
                          (int) pJoinStorageSessionRequest->endpointWebrtcLength, pJoinStorageSessionRequest->pEndpointWebrtc,
                          AWS_JOIN_STORAGE_SESSION_API_POSTFIX);

        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pUrlLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pUrlLength = length;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        // Prepare the body for the call
        if (pJoinStorageSessionRequest->role == SIGNAL_ROLE_MASTER) {
            snprintf(pBody, *pBodyLength, AWS_JOIN_STORAGE_SESSION_MASTER_PARAM_JSON_TEMPLATE,
                     (int) pJoinStorageSessionRequest->channelArnLength, pJoinStorageSessionRequest->pChannelArn);
        } else {
            snprintf(pBody, *pBodyLength, AWS_JOIN_STORAGE_SESSION_VIEWER_PARAM_JSON_TEMPLATE,
                     (int) pJoinStorageSessionRequest->channelArnLength, pJoinStorageSessionRequest->pChannelArn,
                     (int) pJoinStorageSessionRequest->clientIdLength, pJoinStorageSessionRequest->pClientId);
        }
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pBodyLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength = length;
        }
    }

    return result;
}

SignalResult_t Signal_getDeleteChannelRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalDeleteChannelRequest_t * pDeleteChannelRequest )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL || pDeleteChannelRequest == NULL ||
        pDeleteChannelRequest->pChannelArn == NULL || pDeleteChannelRequest->pVersion == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%s",
                          (int) pCtx->controlPlaneUrlLength, pCtx->controlPlaneUrl,
                          AWS_DELETE_SIGNALING_CHANNEL_API_POSTFIX);

        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pUrlLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pUrlLength = length;
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        // Prepare the body for the call
        snprintf(pBody, *pBodyLength, AWS_DELETE_CHANNEL_PARAM_JSON_TEMPLATE,
                 (int) pDeleteChannelRequest->channelArnLength, pDeleteChannelRequest->pChannelArn,
                 (int) pDeleteChannelRequest->versionLength, pDeleteChannelRequest->pVersion);
        
        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pBodyLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pBodyLength = length;
        }
    }

    return result;
}

SignalResult_t Signal_getConnectWssEndpointRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength, SignalConnectWssEndpointRequest_t * pConnectWssEndpointRequest )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pConnectWssEndpointRequest == NULL ||
        pConnectWssEndpointRequest->pChannelArn == NULL || pConnectWssEndpointRequest->pEndpointWebsocketSecure == NULL ||
        (pConnectWssEndpointRequest->role != SIGNAL_ROLE_MASTER && pConnectWssEndpointRequest->role != SIGNAL_ROLE_VIEWER)) {
        result = SIGNAL_RESULT_BAD_PARAM;
    } else if (pConnectWssEndpointRequest->role == SIGNAL_ROLE_VIEWER && pConnectWssEndpointRequest->pClientId == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        if (pConnectWssEndpointRequest->role == SIGNAL_ROLE_MASTER) {
            length = snprintf(pUrl, *pUrlLength, "%.*s?%s=%.*s",
                              (int) pConnectWssEndpointRequest->endpointWebsocketSecureLength, pConnectWssEndpointRequest->pEndpointWebsocketSecure,
                              AWS_SIGNALING_CHANNEL_ARN_PARAM_NAME,
                              (int) pConnectWssEndpointRequest->channelArnLength, pConnectWssEndpointRequest->pChannelArn);
        } else {
            length = snprintf(pUrl, *pUrlLength, "%.*s?%s=%.*s&%s=%.*s",
                              (int) pConnectWssEndpointRequest->endpointWebsocketSecureLength, pConnectWssEndpointRequest->pEndpointWebsocketSecure,
                              AWS_SIGNALING_CHANNEL_ARN_PARAM_NAME,
                              (int) pConnectWssEndpointRequest->channelArnLength, pConnectWssEndpointRequest->pChannelArn,
                              AWS_SIGNALING_CLIENT_ID_PARAM_NAME,
                              (int) pConnectWssEndpointRequest->clientIdLength, pConnectWssEndpointRequest->pClientId);
        }

        if (length < 0) { //LCOV_EXCL_BR_LINE
            result = SIGNAL_RESULT_SNPRINTF_ERROR; // LCOV_EXCL_LINE
        }
        else if (length >= *pUrlLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        }
        else {
            *pUrlLength = length;
        }
    }

    /* No body for connecting websocket secure endpoint.
     * Reserve body parameters for future use. */
    (void) pBody;
    (void) pBodyLength;

    return result;
}
