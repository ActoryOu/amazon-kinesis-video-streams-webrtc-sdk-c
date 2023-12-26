#include <string.h>
#include "signal_api.h"
#include "core_json.h"

static void updateUris(SignalIceServer_t *pIceServer, char * pUris, size_t urisLength);

static void updateUris(SignalIceServer_t *pIceServer, char * pUris, size_t urisLength)
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

    return SIGNAL_RESULT_OK;
}

SignalResult_t Signal_createSignal( SignalContext_t *pCtx, char * pAwsRegion, uint32_t awsRegionLength, char * pControlPlaneUrl, uint32_t controlPlaneUrlLength )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length = 0;

    /* input check */
    if (pCtx == NULL) {
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
            length = snprintf(&pCtx->controlPlaneUrl, AWS_CONTROL_PLANE_URL_MAX_LENGTH, "%s%s.%s%s", AWS_CONTROL_PLANE_URI_PREFIX, AWS_KINESIS_VIDEO_SERVICE_NAME, 
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

    return result;
}

SignalResult_t Signal_setChannelEndpointHttps( SignalContext_t *pCtx, char * pChannelEndpointHttps, uint32_t channelEndpointHttpsLength )
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

SignalResult_t Signal_setChannelArn( SignalContext_t *pCtx, char * pChannelArn, uint32_t channelArnLength )
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

SignalResult_t Signal_setClientId( SignalContext_t *pCtx, char * pClientId, uint32_t clientIdLength )
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

SignalResult_t Signal_getIceConfig( SignalContext_t *pCtx, char * pUrl, uint32_t * pUrlLength, char *pBody, uint32_t * pBodyLength )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    int length=0;

    /* input check */
    if (pCtx == NULL || pUrl == NULL || pBody == NULL) {
        result = SIGNAL_RESULT_BAD_PARAM;
    }

    if (result == SIGNAL_RESULT_OK) {
        // calculate the length of url
        length = pCtx->channelEndpointHttpsLength + strlen(AWS_GET_ICE_CONFIG_API_POSTFIX);

        if (length > *pUrlLength) {
            result = SIGNAL_RESULT_OUT_OF_MEMORY;
        } else {
            strncpy(pUrl, pCtx->channelEndpointHttps, pCtx->channelEndpointHttpsLength);
            strcat(pUrl, AWS_GET_ICE_CONFIG_API_POSTFIX);
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

SignalResult_t Signal_parseIceConfigMessage( SignalContext_t *pCtx, char * pMessage, uint32_t messageLength, SignalIceConfigMessage_t *pIceConfigMessage )
{
    SignalResult_t result = SIGNAL_RESULT_OK;
    JSONStatus_t jsonResult;
    size_t start = 0, next = 0;
    JSONPair_t pair = { 0 };
    char *pIceServerListBuffer;
    uint32_t iceServerListBufferLength;
    size_t iceServerListStart = 0, iceServerListNext = 0;
    char *pIceSingleServerBuffer;
    uint32_t iceSingleServerBufferLength;
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
