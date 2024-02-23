#ifndef SIGNAL_API_H
#define SIGNAL_API_H

/* Standard includes. */
#include "signal_data_types.h"

/*-----------------------------------------------------------*/

SignalResult_t Signal_createSignal( SignalContext_t *pCtx, SignalCreate_t *pCreate );

SignalResult_t Signal_constructDescribeSignalingChannelRequest( SignalContext_t *pCtx, SignalDescribeSignalingChannelRequest_t * pDescribeSignalingChannelRequest, SignalRequest_t *pRequestBuffer );

SignalResult_t Signal_parseDescribeSignalingChannelResponse( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalDescribeSignalingChannelResponse_t *pDescribeSignalingChannelResponse );

SignalResult_t Signal_constructDescribeMediaStorageConfigRequest( SignalContext_t *pCtx, SignalDescribeMediaStorageConfigRequest_t * pDescribeMediaStorageConfigRequest, SignalRequest_t *pRequestBuffer );

SignalResult_t Signal_parseDescribeMediaStorageConfigResponse( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalDescribeMediaStorageConfigResponse_t *pDescribeMediaStorageConfigResponse );

SignalResult_t Signal_constructCreateSignalingChannelRequest( SignalContext_t *pCtx, SignalCreateSignalingChannelRequest_t * pCreateSignalingChannelRequest, SignalRequest_t *pRequestBuffer );

SignalResult_t Signal_parseCreateSignalingChannelResponse( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalCreateSignalingChannelResponse_t *pCreateSignalingChannelResponse );

SignalResult_t Signal_constructGetSignalingChannelEndpointRequest( SignalContext_t *pCtx, SignalGetSignalingChannelEndpointRequest_t * pGetSignalingChannelEndpointRequest, SignalRequest_t *pRequestBuffer );

SignalResult_t Signal_parseGetSignalingChannelEndpointResponse( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalGetSignalingChannelEndpointResponse_t * pGetSignalingChannelEndpointResponse );

SignalResult_t Signal_constructGetIceServerConfigRequest( SignalContext_t *pCtx, SignalGetIceServerConfigRequest_t * pGetIceServerConfigReqeust, SignalRequest_t *pRequestBuffer);

SignalResult_t Signal_parseGetIceServerConfigResponse( SignalContext_t *pCtx, char * pMessage, size_t messageLength, SignalGetIceServerConfigResponse_t *pGetIceConfigResponse );

SignalResult_t Signal_constuctJoinStorageSessionRequest( SignalContext_t *pCtx, SignalJoinStorageSessionRequest_t * pJoinStorageSessionRequest, SignalRequest_t *pRequestBuffer );

SignalResult_t Signal_constructDeleteSignalingChannelRequest( SignalContext_t *pCtx, SignalDeleteSignalingChannelRequest_t * pDeleteSignalingChannelRequest, SignalRequest_t *pRequestBuffer );

SignalResult_t Signal_constructConnectWssEndpointRequest( SignalContext_t *pCtx, SignalConnectWssEndpointRequest_t * pConnectWssEndpointRequest, SignalRequest_t *pRequestBuffer );

SignalResult_t Signal_constructWssMessage( SignalWssSendMessage_t * pWssSendMessage, char * pBuffer, size_t * pBufferLength );

SignalResult_t Signal_parseWssRecvMessage( char * pMessage, size_t messageLength, SignalWssRecvMessage_t * pWssRecvMessage );

/*-----------------------------------------------------------*/

#endif /* SIGNAL_API_H */
