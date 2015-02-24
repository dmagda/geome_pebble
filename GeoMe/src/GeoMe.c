#include <pebble.h>

#include "Windows.h"
#include "MessagingChannel.h"
#include "ContactsDataSource.h"
#include "LocationDataSource.h"


static void on_auth_requred()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Authorization required");

  welcome_window_stop_transitioning();
  authorization_window_show();

  contacts_ds_destroy();
  location_ds_destroy();
}

static void on_auth_completed()
{
  authorization_window_hide();
}

static void deinit(void) {
  message_channel_destroy();

  welcome_window_destroy();
  contacts_window_destroy();
  sending_location_window_destroy();
  authorization_window_destroy();
}

int main(void) {
  message_channel_init((MessageChannelAuthHandlers) {
    .authCompleted = on_auth_completed,
    .authRequired = on_auth_requred
  });

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing");

  welcome_window_show();

  app_event_loop();
  deinit();
}
