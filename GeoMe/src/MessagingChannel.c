
#include "ContactsDataSource.h"
#include "LocationDataSource.h"
#include "Windows.h"

#define APP_MESSAGE_OUTBOX_SIZE 32

static  MessageChannelAuthCompletedHandler authCompletedHandler = NULL;
static  MessageChannelAuthRequiredHandler authRequiredHandler = NULL;

static void destroy_existed_data_sources();


static void in_received_handler(DictionaryIterator *iterator, void *context)
{
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Message received from phone");

  Tuple *authorizationTuple = dict_find(iterator, MSG_CHN_AUTHORIZATION_WORKFLOW_KEY);

  Tuple *contactsFetchingErrorTuple = dict_find(iterator, MSG_CHN_CONTACTS_FETCHING_ERROR_KEY);
  Tuple *contactsCountTuple = dict_find(iterator, MSG_CHN_CONTACTS_COUNT_KEY);

  Tuple *contactIdTuple = dict_find(iterator, MSG_CHN_CONTACT_ID_KEY);
  Tuple *contactNameTuple = dict_find(iterator, MSG_CHN_CONTACT_NAME_KEY);

  Tuple *locationDetermineSuccessTuple = dict_find(iterator, MSG_CHN_LOCATION_DETERMINED_KEY);
  Tuple *locationDetermineFailureTuple = dict_find(iterator, MSG_CHN_LOCATION_DETERMINATION_ERROR_KEY);

  Tuple *locationSendSuccessTuple = dict_find(iterator, MSG_CHN_LOCATION_SENT_KEY);
  Tuple *locationSendErrorTuple = dict_find(iterator, MSG_CHN_LOCATION_SENDING_ERROR_KEY);

  if (contactsCountTuple) {
    contacts_ds_contacts_count_received(contactsCountTuple);

  } else if (contactIdTuple && contactNameTuple) {
    contacts_ds_contact_received(contactIdTuple, contactNameTuple);

  } else if (contactsFetchingErrorTuple) {
    contacts_ds_failed_to_fetch_contacts(contactsFetchingErrorTuple->value->int32);

  } else if (locationSendSuccessTuple) {
    location_ds_location_sent(locationSendSuccessTuple);

  } else if (locationSendErrorTuple) {
    location_ds_location_sending_failed(contactIdTuple, locationSendErrorTuple->value->int32);

  } else if (locationDetermineSuccessTuple) {
    location_ds_location_determined(locationDetermineSuccessTuple);

  } else if (locationDetermineFailureTuple) {
    location_ds_location_determinitaion_failed(locationDetermineFailureTuple->value->int32);

  } else if (authorizationTuple) {
    MessageChannelResult value = authorizationTuple->value->int32;

    if (value == MSG_CHN_OK) {
      if (authCompletedHandler != NULL) {
        authCompletedHandler();
      }

    } else {
      if (authRequiredHandler != NULL) {
        authRequiredHandler();
      }
    }
  }

}

static void in_dropped_handler(AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Incoming message is dropped");

  MessageChannelResult chReason = message_channel_convert_result_code(reason);

  contacts_ds_failed_to_fetch_contacts(chReason);
  location_ds_location_sending_failed(NULL, chReason);
}


static void out_sent_handler(DictionaryIterator *iterator, void *context)
{
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Message is sent");
}

static void out_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send a message");

  Tuple *contactsFetchTuple = dict_find(iterator, MSG_CHN_FETCH_CONTACTS_KEY);
  Tuple *sendLocationTuple = dict_find(iterator, MSG_CHN_SEND_LOCATION_KEY);
  Tuple *determineLocationTuple = dict_find(iterator, MSG_CHN_DETERMINE_LOCATION_KEY);

  MessageChannelResult chReason = message_channel_convert_result_code(reason);

  if (contactsFetchTuple) {
    contacts_ds_failed_to_fetch_contacts(chReason);

  } else if (sendLocationTuple) {
    location_ds_location_sending_failed(sendLocationTuple, chReason);

  } else if (determineLocationTuple) {
    location_ds_location_determinitaion_failed(chReason);
  }
}

void message_channel_init(MessageChannelAuthHandlers handlers)
{
  authCompletedHandler = handlers.authCompleted;
  authRequiredHandler = handlers.authRequired;

  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);

  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE);
}

void message_channel_destroy()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Destroying messaging channel");

  app_message_deregister_callbacks();
  destroy_existed_data_sources();
}

static void destroy_existed_data_sources() {
  contacts_ds_destroy();
  location_ds_destroy();
}

MessageChannelResult message_channel_convert_result_code(AppMessageResult appMessageResult)
{
  switch (appMessageResult) {
    case APP_MSG_OK:
      return MSG_CHN_OK;

    case APP_MSG_NOT_CONNECTED:
      return MSG_CHN_PHONE_NOT_CONNECTED;

    case APP_MSG_APP_NOT_RUNNING:
      return MSG_CHN_PHONE_APP_NOT_RUNNING;

    default:
      return MSG_CHN_GENERAL_ERROR;
  }
}
