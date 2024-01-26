/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "signal_api.h"
#include "core_json.h"

static void updateUris(SignalIceServer_t *pIceServer, const char * pUris, size_t urisLength);

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

SignalResult_t Signal_createSignal( SignalContext_t *pCtx, char * pAwsRegion, size_t awsRegionLength, char * pControlPlaneUrl, size_t controlPlaneUrlLength, char * pChannelName, size_t channelNameLength )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length = 0;

    /* input check */
    if (pCtx == NULL || pChannelName == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        memset(pCtx, 0, sizeof(SignalContext_t));

        if (pAwsRegion == NULL) {
            pCtx->regionLength = strlen(AWS_DEFAULT_REGION);
            memcpy(pCtx->region, AWS_DEFAULT_REGION, pCtx->regionLength);
        } else {
            pCtx->regionLength = awsRegionLength;
            memcpy(pCtx->region, pAwsRegion, pCtx->regionLength);
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        if (pControlPlaneUrl == NULL) {
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
            pCtx->controlPlaneUrlLength = controlPlaneUrlLength;
            memcpy(pCtx->controlPlaneUrl, pControlPlaneUrl, pCtx->controlPlaneUrlLength);
        }
    }

    if (result == SIGNAL_RESULT_OK) {
        pCtx->channelNameLength = channelNameLength;
        memcpy(pCtx->channelName, pChannelName, pCtx->channelNameLength);
    }

    return result;
}

SignalResult_t Signal_setChannelEndpointHttps( SignalContext_t *pCtx, char * pChannelEndpointHttps, size_t channelEndpointHttpsLength )
{
    SignalResult_t result = SIGNAL_RESULT_OK;

    /* input check */
    if (pCtx == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        pCtx->channelEndpointHttpsLength = channelEndpointHttpsLength;
        memcpy(pCtx->channelEndpointHttps, pChannelEndpointHttps, pCtx->channelEndpointHttpsLength);
    }

    return result;
}

SignalResult_t Signal_setChannelArn( SignalContext_t *pCtx, char * pChannelArn, size_t channelArnLength )
{
    SignalResult_t result = SIGNAL_RESULT_OK;

    /* input check */
    if (pCtx == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        pCtx->channelArnLength = channelArnLength;
        memcpy(pCtx->channelArn, pChannelArn, pCtx->channelArnLength);
    }

    return result;
}

SignalResult_t Signal_setClientId( SignalContext_t *pCtx, char * pClientId, size_t clientIdLength )
{
    SignalResult_t result = SIGNAL_RESULT_OK;

    /* input check */
    if (pCtx == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        pCtx->clientIdLength = clientIdLength;
        memcpy(pCtx->clientId, pClientId, pCtx->clientIdLength);
    }

    return result;
}

SignalResult_t Signal_getDescribeChannelRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%.*s",
                          (int) pCtx->controlPlaneUrlLength, pCtx->controlPlaneUrl,
                          (int) strlen(AWS_DESCRIBE_SIGNALING_CHANNEL_API_POSTFIX), AWS_DESCRIBE_SIGNALING_CHANNEL_API_POSTFIX);

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
        length = snprintf(pBody, *pBodyLength, AWS_DESCRIBE_CHANNEL_PARAM_JSON_TEMPLATE, pCtx->channelName);
        
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
    char ttlSeconds[AWS_MESSAGE_TTL_SECONDS_BUFFER_MAX] = { 0 };

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
                result = SIGNAL_RESULT_INVALID_JSON;
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

                /* Store channel ARN in context. */
                pCtx->channelArnLength = pDescribeChannel->channelArnLength;
                memcpy(pCtx->channelArn, pDescribeChannel->pChannelArn, pCtx->channelArnLength);
            } else if (strncmp(pair.key, "ChannelName", pair.keyLength) == 0) {
                pDescribeChannel->pChannelName = pair.value;
                pDescribeChannel->channelNameLength = pair.valueLength;

                /* Store channel name in context. */
                pCtx->channelNameLength = pDescribeChannel->channelNameLength;
                memcpy(pCtx->channelName, pDescribeChannel->pChannelName, pCtx->channelNameLength);
            } else if (strncmp(pair.key, "ChannelStatus", pair.keyLength) == 0) {
                pDescribeChannel->pChannelStatus = pair.value;
                pDescribeChannel->channelStatusLength = pair.valueLength;
            } else if (strncmp(pair.key, "ChannelType", pair.keyLength) == 0) {
                pDescribeChannel->pChannelType = pair.value;
                pDescribeChannel->channelTypeLength = pair.valueLength;

                /* Store channel type in context. */
                if (strncmp(pDescribeChannel->pChannelType, "SINGLE_MASTER", pDescribeChannel->channelTypeLength) == 0) {
                    pCtx->channelType = SIGNAL_CHANNEL_TYPE_SINGLE_MASTER;
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
                        if( pair.valueLength >= AWS_MESSAGE_TTL_SECONDS_BUFFER_MAX ) {
                            /* Unexpect TTL value from cloud. */
                            result = SIGNAL_RESULT_INVALID_JSON;
                            break;
                        }

                        strncpy(ttlSeconds, pair.value, pair.valueLength);
                        pDescribeChannel->messageTtlSeconds = (uint16_t) strtoul(ttlSeconds, NULL, 10);

                        if (pDescribeChannel->messageTtlSeconds < AWS_MESSAGE_TTL_SECONDS_MIN || pDescribeChannel->messageTtlSeconds > AWS_MESSAGE_TTL_SECONDS_MAX) {
                            /* Unexpect TTL value from cloud. */
                            result = SIGNAL_RESULT_INVALID_JSON;
                        } else {
                            pCtx->messageTtlSeconds = pDescribeChannel->messageTtlSeconds;
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

SignalResult_t Signal_getMediaStorageConfigRequest( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%.*s",
                          (int) pCtx->controlPlaneUrlLength, pCtx->controlPlaneUrl,
                          (int) strlen(AWS_DESCRIBE_MEDIA_STORAGE_CONF_API_POSTFIX), AWS_DESCRIBE_MEDIA_STORAGE_CONF_API_POSTFIX);

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
        length = snprintf(pBody, *pBodyLength, AWS_DESCRIBE_MEDIA_STORAGE_CONF_PARAM_JSON_TEMPLATE, pCtx->channelArn);
        
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
                result = SIGNAL_RESULT_INVALID_JSON;
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
        (pCreateChannel->tagsCount > 0 && pCreateChannel->pTags == NULL)) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%.*s",
                          (int) pCtx->controlPlaneUrlLength, pCtx->controlPlaneUrl,
                          (int) strlen(AWS_CREATE_SIGNALING_CHANNEL_API_POSTFIX), AWS_CREATE_SIGNALING_CHANNEL_API_POSTFIX);

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
                          pCreateChannel->channelInfo.channelType == SIGNAL_CHANNEL_TYPE_SINGLE_MASTER? SIGNA_CHANNEL_TYPE_SINGLE_MASTER_STRING:SIGNA_CHANNEL_TYPE_UNKNOWN_STRING,
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
    if (result == SIGNAL_RESULT_OK) {
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
    if (result == SIGNAL_RESULT_OK) {
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

        pCtx->channelType = pCreateChannel->channelInfo.channelType;

        pCtx->messageTtlSeconds = pCreateChannel->channelInfo.messageTtlSeconds;
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

                /* Store channel information in context. */
                pCtx->channelArnLength = pCreateChannel->channelInfo.channelArnLength;
                memcpy(pCtx->channelArn, pCreateChannel->channelInfo.pChannelArn, pCtx->channelArnLength);
            }
        } else {
            result = SIGNAL_RESULT_INVALID_JSON;
        }
    }

    return result;
}

SignalResult_t Signal_getIceConfig( SignalContext_t *pCtx, char * pUrl, size_t * pUrlLength, char *pBody, size_t * pBodyLength )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = snprintf(pUrl, *pUrlLength, "%.*s%.*s",
                          (int) pCtx->channelEndpointHttpsLength, pCtx->channelEndpointHttps,
                          (int) strlen(AWS_GET_ICE_CONFIG_API_POSTFIX), AWS_GET_ICE_CONFIG_API_POSTFIX);

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
        length = snprintf(pBody, *pBodyLength, AWS_GET_ICE_CONFIG_PARAM_JSON_TEMPLATE, pCtx->channelArn,
                          pCtx->clientId);
        
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
                result = SIGNAL_RESULT_INVALID_JSON;
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
                    }
                    
                    pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pPassword = pair.value;
                    pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].passwordLength = pair.valueLength;
                } else if (strncmp(pair.key, "Ttl", pair.keyLength) == 0) {
                    if (pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pTtl != NULL) {
                        pIceConfigMessage->iceServerNum++;
                    }
                    
                    pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pTtl = pair.value;
                    pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].ttlLength = pair.valueLength;
                } else if (strncmp(pair.key, "Uris", pair.keyLength) == 0) {
                    if (pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pUris[0] != NULL) {
                        pIceConfigMessage->iceServerNum++;
                    }
                    
                    updateUris(&pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum], pair.value, pair.valueLength);
                } else if (strncmp(pair.key, "Username", pair.keyLength) == 0) {
                    if (pIceConfigMessage->iceServer[pIceConfigMessage->iceServerNum].pUserName != NULL) {
                        pIceConfigMessage->iceServerNum++;
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
