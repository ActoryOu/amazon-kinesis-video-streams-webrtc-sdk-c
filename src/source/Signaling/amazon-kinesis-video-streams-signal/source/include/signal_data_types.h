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

// Parameterized string for Get Ice Server Config API
#define AWS_GET_ICE_CONFIG_PARAM_JSON_TEMPLATE                                                                                                           \
    "{\n\t\"ChannelARN\": \"%s\","                                                                                                                   \
    "\n\t\"ClientId\": \"%s\","                                                                                                                      \
    "\n\t\"Service\": \"TURN\""                                                                                                                      \
    "\n}"

/*-----------------------------------------------------------*/

#endif /* SIGNAL_DATA_TYPES_H */
