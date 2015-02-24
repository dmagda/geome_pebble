#pragma once

#include <pebble.h>

//Message Channel result codes.
typedef enum {
  //! All good, operation was successful.
  MSG_CHN_OK = 0,

  //! The watches are not authorized in GeoMe, registration required.
  MSG_CHN_NOT_AUTHORIZED = 1,

  //! The other end was not connected.
  MSG_CHN_PHONE_NOT_CONNECTED = 2,

  //! The local application was not running.
  MSG_CHN_PHONE_APP_NOT_RUNNING = 3,

  //! Http timeout is fired
  MSG_CHN_HTTP_TIMEOUT = 4,

  //! Current user location is not determined
  MSG_CHN_LOCATION_NOT_DETERMINED = 5,

  //! General error
  MSG_CHN_GENERAL_ERROR = 6,

} MessageChannelResult;

typedef enum {
  MSG_CHN_FETCH_CONTACTS_KEY = 1,
  MSG_CHN_CONTACTS_FETCHING_ERROR_KEY = 2,
  MSG_CHN_CONTACTS_COUNT_KEY = 3,

  MSG_CHN_CONTACT_ID_KEY = 4,
  MSG_CHN_CONTACT_NAME_KEY = 5,

  MSG_CHN_DETERMINE_LOCATION_KEY = 6,
  MSG_CHN_LOCATION_DETERMINED_KEY = 7,
  MSG_CHN_LOCATION_DETERMINATION_ERROR_KEY = 8,

  MSG_CHN_SEND_LOCATION_KEY = 9,
  MSG_CHN_LOCATION_SENT_KEY = 10,
  MSG_CHN_LOCATION_SENDING_ERROR_KEY = 11,

  MSG_CHN_AUTHORIZATION_WORKFLOW_KEY = 12
} MessageChannelKeys;


typedef void (*MessageChannelAuthCompletedHandler)();
typedef void (*MessageChannelAuthRequiredHandler)();

typedef struct {
    MessageChannelAuthCompletedHandler authCompleted;
    MessageChannelAuthRequiredHandler authRequired;
} MessageChannelAuthHandlers;

//General messaging functions
void message_channel_init(MessageChannelAuthHandlers handlers);
void message_channel_destroy();


MessageChannelResult message_channel_convert_result_code(AppMessageResult appMessageResult);