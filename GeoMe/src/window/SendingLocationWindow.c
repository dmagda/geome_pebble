#include "Windows.h"
#include "LocationDataSource.h"
#include "string_resources.h"
#include "StatusLayer.h"
#include "TextUtils.h"

#define LOCATION_AUTO_SEND_ACCURACY_THRESHOLD 60

static Window *window = NULL;
static StatusLayer *statusLayer = NULL;

static Contact *contact;
static AppTimer *autoSendTimer = NULL;

static bool isLocationSentByClick = false;

static void window_load(Window *window);
static void window_unload(Window *window);
static void window_config_provider(void *context);
static void middle_button_click_handler(ClickRecognizerRef recognizer, void *context);

static void create_status_layer();
static void remove_status_layer();

static void determine_location_success(int accuracy);
static void determine_location_failure(MessageChannelResult reason);

static void start_sending_location();
static void send_location_success(int contactId);
static void send_location_failure(int contactId, MessageChannelResult reason);
static void show_send_location_error(MessageChannelResult reason);

static void auto_send_timer_fired(void* data);

//------------- Public Methods --------------

void sending_location_window_show(Contact *contactToSend)
{
  contact = contactToSend;

  if (window == NULL) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }

  window_stack_push(window, true);
}

void sending_location_window_destroy()
{
  if (window) {
    window_destroy(window);
    window = NULL;
  }
}


//---------- Window ----------------------

static void window_load(Window *currentWindow)
{
  create_status_layer();

  location_ds_set_handlers((LocationDataSourceHandlers) {
    .determineLocationSuccess = determine_location_success,
    .determineLocationFailure = determine_location_failure,
    .sendLocationSuccess = send_location_success,
    .sendLocationFailure = send_location_failure
  });

  MessageChannelResult result = location_ds_determine_current_location();

  if (result == MSG_CHN_OK) {
    status_layer_set_text(statusLayer, DETERMINE_LOCATION_TEXT);
    status_layer_set_image(statusLayer, RESOURCE_ID_IMAGE_SAND_WATCH);

  } else {
    status_layer_show_error(statusLayer, result);
  }
}

static void window_unload(Window *currentWindow)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending location window is unloaded");

  if (autoSendTimer) {
    app_timer_cancel(autoSendTimer);
    autoSendTimer = NULL;
  }

  location_ds_clear_handlers();

  remove_status_layer();

  contact = NULL;
  isLocationSentByClick = false;
}

static void window_config_provider(void *context)
{
  window_single_click_subscribe(BUTTON_ID_SELECT, middle_button_click_handler);
}

static void middle_button_click_handler(ClickRecognizerRef recognizer, void *context)
{
  if (!isLocationSentByClick) {
    start_sending_location();
    isLocationSentByClick = true;
  }
}

//----------- Status Layer ------------------------

static void create_status_layer()
{
  Layer *windowLayer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(windowLayer);

  statusLayer = status_layer_create((GRect) {.origin = GPointZero, .size = bounds.size});

  layer_add_child(windowLayer, status_layer_get_layer(statusLayer));
}


static void remove_status_layer()
{
  if (statusLayer) {
    status_layer_destroy(statusLayer);
    statusLayer = NULL;
  }
}


//------------ Determine Location ----------------

static void determine_location_success(int accuracy)
{
  if (accuracy <= LOCATION_AUTO_SEND_ACCURACY_THRESHOLD) {
    //IMPL_NOTE: otherwise we will get APP_MSG_BUSY if try to send a message in a messages' callback
    autoSendTimer = app_timer_register(1000 /* 1 sec */, auto_send_timer_fired, NULL);

  } else {
    char *buffer = text_utils_get_shared_buffer();
    snprintf(buffer, SHARED_TEXT_BUFFER_LEN, LOW_ACCURACY_ALERT, accuracy);

    status_layer_set_text(statusLayer, buffer);
    status_layer_remove_image(statusLayer);

    window_set_click_config_provider(window, window_config_provider);
  }
}

static void determine_location_failure(MessageChannelResult reason)
{
  status_layer_show_error(statusLayer, reason);
  location_ds_clear_handlers();
}


//----------- Sending Location -------------------

static bool is_current_contact(int contactId) {
  return window != NULL && contact != NULL && contact->id == contactId;
}

static void start_sending_location()
{
  MessageChannelResult result = location_ds_send_location_to_contact(contact);

  if (result == MSG_CHN_OK) {
    status_layer_set_text(statusLayer, SENDING_LOCATION_TEXT);
    status_layer_set_image(statusLayer, RESOURCE_ID_IMAGE_SAND_WATCH);

  } else {
    location_ds_clear_handlers();
    show_send_location_error(result);
  }
}

static void send_location_success(int contactId)
{
  if (!is_current_contact(contactId)) {
    return;
  }

  location_ds_clear_handlers();

  status_layer_set_text(statusLayer, SUCCESSFULLY_SENT_LOCATION);
  status_layer_set_image(statusLayer, RESOURCE_ID_IMAGE_CHECK_MARK);

  vibes_short_pulse();
}

static void send_location_failure(int contactId, MessageChannelResult reason)
{
  if (!is_current_contact(contactId)) {
    return;
  }

  location_ds_clear_handlers();
  show_send_location_error(reason);
}

static void show_send_location_error(MessageChannelResult reason)
{
  if (reason == MSG_CHN_GENERAL_ERROR) {
    status_layer_show_error_with_custom_text(statusLayer, UNABLE_TO_SEND_LOCATION);
  } else {
    status_layer_show_error(statusLayer, reason);
  }
}

static void auto_send_timer_fired(void* data)
{
  autoSendTimer = NULL;
  start_sending_location();
}




