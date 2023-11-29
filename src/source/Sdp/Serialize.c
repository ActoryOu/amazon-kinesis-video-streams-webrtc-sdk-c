#define LOG_CLASS "SDPSerialize"
#include "../Include_i.h"
#include "sdp_serializer.h"

STATUS serializeVersion( SdpSerializerContext_t * pCtx, UINT64 version )
{
    ENTERS();
    STATUS retStatus = STATUS_SUCCESS;
    SdpResult_t sdpResult = SDP_RESULT_OK;
    
    sdpResult = SdpSerializer_AddU32( pCtx, SDP_TYPE_VERSION, version );

    if( sdpResult == SDP_RESULT_OK )
    {
        retStatus = STATUS_SUCCESS;
    }
    else if( sdpResult == SDP_RESULT_OUT_OF_MEMORY )
    {
        retStatus = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        retStatus = STATUS_INTERNAL_ERROR;
    }

CleanUp:

    LEAVES();
    return retStatus;
}

STATUS serializeOrigin( SdpSerializerContext_t * pCtx, PSdpOrigin pSDPOrigin )
{
    ENTERS();
    STATUS retStatus = STATUS_SUCCESS;
    SdpResult_t sdpResult = SDP_RESULT_OK;
    SdpOriginator_t origin;
    
    CHK(pSDPOrigin != NULL, STATUS_NULL_ARG);

    if (pSDPOrigin->userName[0] != '\0' && pSDPOrigin->sdpConnectionInformation.networkType[0] != '\0' &&
        pSDPOrigin->sdpConnectionInformation.addressType[0] != '\0' && pSDPOrigin->sdpConnectionInformation.connectionAddress[0] != '\0') {
        origin.pUserName = pSDPOrigin->userName;
        origin.userNameLength = strlen( pSDPOrigin->userName );
        origin.sessionId = pSDPOrigin->sessionId;
        origin.sessionVersion = pSDPOrigin->sessionVersion;

        CHK(strcmp(pSDPOrigin->sdpConnectionInformation.networkType, "IN") == 0, STATUS_INVALID_ARG);
        
        origin.connectionInfo.networkType = SDP_NETWORK_IN;
    }

    CHK(strcmp(pSDPOrigin->sdpConnectionInformation.addressType, "IP4") == 0 ||
        strcmp(pSDPOrigin->sdpConnectionInformation.addressType, "IP6") == 0, STATUS_INVALID_ARG);

    if (strcmp( pSDPOrigin->sdpConnectionInformation.addressType, "IP4" ) == 0) {
        origin.connectionInfo.addressType = SDP_ADDRESS_IPV4;
    } else {
        origin.connectionInfo.addressType = SDP_ADDRESS_IPV6;
    }

    origin.connectionInfo.pAddress = pSDPOrigin->sdpConnectionInformation.connectionAddress;
    origin.connectionInfo.addressLength = strlen( pSDPOrigin->sdpConnectionInformation.connectionAddress );

    sdpResult = SdpSerializer_AddOriginator( pCtx,
                                             SDP_TYPE_ORIGINATOR,
                                             &( origin ) );

    if( sdpResult == SDP_RESULT_OK )
    {
        retStatus = STATUS_SUCCESS;
    }
    else if( sdpResult == SDP_RESULT_OUT_OF_MEMORY )
    {
        retStatus = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        retStatus = STATUS_INTERNAL_ERROR;
    }

CleanUp:

    LEAVES();
    return retStatus;
}

static SdpResult_t serializeSessionName( SdpSerializerContext_t * pCtx, char * sessionName )
{
    ENTERS();
    STATUS retStatus = STATUS_SUCCESS;
    SdpResult_t sdpResult = SDP_RESULT_OK;

    /* Input check. */
    if ((sessionName != NULL) || (sessionName[0] != '\0')) {
        sdpResult = SdpSerializer_AddBuffer( pCtx,
                                          SDP_TYPE_SESSION_NAME,
                                          sessionName,
                                          strlen( sessionName ) );

        if( sdpResult == SDP_RESULT_OK )
        {
            retStatus = STATUS_SUCCESS;
        }
        else if( sdpResult == SDP_RESULT_OUT_OF_MEMORY )
        {
            retStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            retStatus = STATUS_INTERNAL_ERROR;
        }
    }

CleanUp:

    LEAVES();
    return retStatus;
}

static SdpResult_t serializeTimeDescription( SdpSerializerContext_t * pCtx, PSdpTimeDescription pSDPTimeDescription )
{
    ENTERS();
    STATUS retStatus = STATUS_SUCCESS;
    SdpResult_t sdpResult = SDP_RESULT_OK;
    SdpTimeDescription_t time;

    CHK(pSDPTimeDescription != NULL, STATUS_NULL_ARG);

    time.startTime = pSDPTimeDescription->startTime;
    time.stopTime = pSDPTimeDescription->stopTime;
    sdpResult = SdpSerializer_AddTimeActive( pCtx,
                                             SDP_TYPE_TIME_ACTIVE,
                                             &( time ) );

    if( sdpResult == SDP_RESULT_OK )
    {
        retStatus = STATUS_SUCCESS;
    }
    else if( sdpResult == SDP_RESULT_OUT_OF_MEMORY )
    {
        retStatus = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        retStatus = STATUS_INTERNAL_ERROR;
    }

CleanUp:

    LEAVES();
    return retStatus;
}

static SdpResult_t serializeAttribute( SdpSerializerContext_t * pCtx, PSdpAttributes pSDPAttributes )
{
    ENTERS();
    STATUS retStatus = STATUS_SUCCESS;
    SdpResult_t sdpResult = SDP_RESULT_OK;
    SdpAttribute_t attribute;

    CHK(pSDPAttributes != NULL, STATUS_NULL_ARG);
    
    attribute.pAttributeName = pSDPAttributes->attributeName;
    attribute.attributeNameLength = strlen( pSDPAttributes->attributeName );

    if (pSDPAttributes->attributeValue[0] == '\0') {
        attribute.pAttributeValue = NULL;
        attribute.attributeValueLength = 0;
    } else {
        attribute.pAttributeValue = pSDPAttributes->attributeValue;
        attribute.attributeValueLength = strlen( pSDPAttributes->attributeValue );
    }

    sdpResult = SdpSerializer_AddAttribute( pCtx,
                                            SDP_TYPE_ATTRIBUTE,
                                            &( attribute ) );

    if( sdpResult == SDP_RESULT_OK )
    {
        retStatus = STATUS_SUCCESS;
    }
    else if( sdpResult == SDP_RESULT_OUT_OF_MEMORY )
    {
        retStatus = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        retStatus = STATUS_INTERNAL_ERROR;
    }

CleanUp:

    LEAVES();
    return retStatus;
}

static SdpResult_t serializeMediaName( SdpSerializerContext_t * pCtx, char * pMediaName )
{
    ENTERS();
    STATUS retStatus = STATUS_SUCCESS;
    SdpResult_t sdpResult = SDP_RESULT_OK;

    CHK(pMediaName != NULL, STATUS_NULL_ARG);
    
    sdpResult =  SdpSerializer_AddBuffer( pCtx,
                                          SDP_TYPE_MEDIA,
                                          pMediaName,
                                          strlen( pMediaName ) );

    if( sdpResult == SDP_RESULT_OK )
    {
        retStatus = STATUS_SUCCESS;
    }
    else if( sdpResult == SDP_RESULT_OUT_OF_MEMORY )
    {
        retStatus = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        retStatus = STATUS_INTERNAL_ERROR;
    }

CleanUp:

    LEAVES();
    return retStatus;
}

static SdpResult_t serializeMediaConnectionInformation( SdpSerializerContext_t * pCtx, PSdpConnectionInformation pSdpConnectionInformation )
{
    ENTERS();
    STATUS retStatus = STATUS_SUCCESS;
    SdpResult_t sdpResult = SDP_RESULT_OK;
    SdpConnectionInfo_t connInfo;

    CHK(pSdpConnectionInformation != NULL, STATUS_NULL_ARG);

    if (pSdpConnectionInformation->networkType[0] != '\0') {
        /* Append connection info */
        CHK(strcmp( pSdpConnectionInformation->networkType, "IN") == 0, STATUS_INVALID_ARG);
        connInfo.networkType = SDP_NETWORK_IN;

        CHK(strcmp(pSdpConnectionInformation->addressType, "IP4") == 0 ||
            strcmp(pSdpConnectionInformation->addressType, "IP6") == 0, STATUS_INVALID_ARG);

        if (strcmp( pSdpConnectionInformation->addressType, "IP4") == 0) {
            connInfo.addressType = SDP_ADDRESS_IPV4;
        } else {
            connInfo.addressType = SDP_ADDRESS_IPV6;
        }

        connInfo.pAddress = pSdpConnectionInformation->connectionAddress;
        connInfo.addressLength = strlen( pSdpConnectionInformation->connectionAddress );

        sdpResult = SdpSerializer_AddConnectionInfo( pCtx,
                                                     SDP_TYPE_CONNINFO,
                                                     &( connInfo ) );

        if( sdpResult == SDP_RESULT_OK )
        {
            retStatus = STATUS_SUCCESS;
        }
        else if( sdpResult == SDP_RESULT_OUT_OF_MEMORY )
        {
            retStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            retStatus = STATUS_INTERNAL_ERROR;
        }
    }

CleanUp:

    LEAVES();
    return retStatus;
}

STATUS serializeSessionDescription(PSessionDescription pSessionDescription, PCHAR sdpBytes, PUINT32 sdpBytesLength)
{
    ENTERS();
    STATUS retStatus = STATUS_SUCCESS;
    SdpResult_t sdpResult = SDP_RESULT_OK;
    SdpSerializerContext_t ctx;
    const CHAR * pSdpMessage;
    UINT32 sdpMessageLength;
    UINT32 i, j;

    CHK(SdpSerializer_Init( &( ctx ), sdpBytes, *sdpBytesLength )==SDP_RESULT_OK, STATUS_INTERNAL_ERROR);

    /* Append version. */
    CHK_STATUS(serializeVersion( &( ctx ), pSessionDescription->version ));

    /* Append originator. */
    CHK_STATUS(serializeOrigin( &( ctx ), &( pSessionDescription->sdpOrigin ) ));

    /* Append session name. */
    CHK_STATUS(serializeSessionName( &( ctx ), pSessionDescription->sessionName ));

    /* Append time description. */
    for (i = 0; i < pSessionDescription->timeDescriptionCount; i++) {
        CHK_STATUS(serializeTimeDescription( &( ctx ), &( pSessionDescription->sdpTimeDescription[i] ) ));
    }

    /* Append session attributes. */
    for (i = 0; i < pSessionDescription->sessionAttributesCount; i++) {
        CHK_STATUS(serializeAttribute(&( ctx ), &( pSessionDescription->sdpAttributes[i])));
    }

    /* Append media. */
    for (i = 0; i < pSessionDescription->mediaCount; i++) {
        CHK_STATUS(serializeMediaName(&( ctx ), pSessionDescription->mediaDescriptions[i].mediaName));
        CHK_STATUS(serializeMediaConnectionInformation(&( ctx ), &(pSessionDescription->mediaDescriptions[i].sdpConnectionInformation)));
        
        /* Append media attributes. */
        for (j = 0; j < pSessionDescription->mediaDescriptions[i].mediaAttributesCount; j++) {
            CHK_STATUS(serializeAttribute(&( ctx ), &(pSessionDescription->mediaDescriptions[i].sdpAttributes[j])));
        }
    }

    CHK(SdpSerializer_Finalize( &(ctx), &(pSdpMessage), &(sdpMessageLength))==SDP_RESULT_OK, STATUS_INTERNAL_ERROR);
    
    *sdpBytesLength = sdpMessageLength + 1U; // NULL terminator

CleanUp:
    LEAVES();
    return retStatus;
}
