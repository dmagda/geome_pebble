#include "LocationDataSource.h"

#define DETERMINE_LOCATION_VALUE 1

static LocationDataSourceDetermineLocationSuccessHandler determineLocationSuccessHandler = NULL;
static LocationDataSourceDetermineLocationFailureHandler determineLocationFailureHandler = NULL;
static LocationDataSourceSendLocationSuccessHandler sendLocationSuccessHandler = NULL;
static LocationDataSourceSendLocationFailureHandler sendLocationFailureHandler = NULL;

static Contact *locationReceiver;


//----------------- Public Methods --------------

void location_ds_set_handlers(LocationDataSourceHandlers handlers)
{
  determineLocationSuccessHandler = handlers.determineLocationSuccess;
  determineLocationFailureHandler = handlers.determineLocationFailure;
  sendLocationSuccessHandler = handlers.sendLocationSuccess;
  sendLocationFailureHandler = handlers.sendLocationFailure;
}

void location_ds_clear_handlers()
{
  determineLocationSuccessHandler = NULL;
  determineLocationFailureHandler = NULL;
  sendLocationSuccessHandler = NULL;
  sendLocationFailureHandler = NULL;
  locationReceiver = NULL;
}

void location_ds_destroy()
{
  location_ds_clear_handlers();
}



MessageChannelResult location_ds_determine_current_location()
{
  DictionaryIterator *iterator;

  AppMessageResult boxResult = app_message_outbox_begin(&iterator);

  if (boxResult != APP_MSG_OK) {
    return message_channel_convert_result_code(boxResult);
  }

  Tuplet tuple = TupletInteger(MSG_CHN_DETERMINE_LOCATION_KEY, DETERMINE_LOCATION_VALUE);
  DictionaryResult dictResult = dict_write_tuplet(iterator, &tuple);
  if (dictResult != DICT_OK) {
    //no way to release the iterator in API
    return MSG_CHN_GENERAL_ERROR;
  }

  dict_write_end(iterator);

  boxResult = app_message_outbox_send();

  return message_channel_convert_result_code(boxResult);
}

void location_ds_location_determined(Tuple *resultTuple)
{
  int accuracy = resultTuple->value->int32;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Location is determined, accuracy = %d", accuracy);

  if (determineLocationSuccessHandler) {
    determineLocationSuccessHandler(accuracy);
  }
}

void location_ds_location_determinitaion_failed(MessageChannelResult reason)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to determine location");

  if (determineLocationFailureHandler) {
    determineLocationFailureHandler(reason);
  }
}


MessageChannelResult location_ds_send_location_to_contact(Contact *contact)
{
  DictionaryIterator *iterator;

  AppMessageResult boxResult = app_message_outbox_begin(&iterator);

  if (boxResult != APP_MSG_OK) {
    return message_channel_convert_result_code(boxResult);
  }

  Tuplet tuple = TupletInteger(MSG_CHN_SEND_LOCATION_KEY, contact->id);
  DictionaryResult dictResult = dict_write_tuplet(iterator, &tuple);
  if (dictResult != DICT_OK) {
    //no way to release the iterator in API
    return MSG_CHN_GENERAL_ERROR;
  }

  dict_write_end(iterator);

  locationReceiver = contact;

  boxResult = app_message_outbox_send();

  return message_channel_convert_result_code(boxResult);
}

void location_ds_location_sent(Tuple *contactTuple)
{
  int contactId = contactTuple->value->int32;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Location is sent to contact %d", contactId);

  if (locationReceiver != NULL && locationReceiver->id == contactId && sendLocationSuccessHandler) {
    sendLocationSuccessHandler(contactId);
  }
}

void location_ds_location_sending_failed(Tuple *contactTuple, MessageChannelResult reason)
{
  int contactId = contactTuple != NULL ? contactTuple->value->int32 : 0;

  APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to send location to contact %d", contactId);

  if (locationReceiver != NULL && (locationReceiver->id == contactId || contactId == 0) && sendLocationFailureHandler) {
    sendLocationFailureHandler(locationReceiver->id, reason);
  }
}


//-------- Private Methods -------------
