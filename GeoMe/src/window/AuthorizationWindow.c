#include "Windows.h"
#include "string_resources.h"
#include "StatusLayer.h"
#include "MessagingChannel.h"

static Window *window = NULL;
static StatusLayer *statusLayer = NULL;


static void window_load(Window *window);
static void window_unload(Window *window);

static void create_status_layer();
static void remove_status_layer();


//---- Public Methods --------

void authorization_window_show()
{
  if (window == NULL) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }

  window_stack_pop_all(false); //remove all the windows from the window stack
  window_stack_push(window, false);
}

void authorization_window_hide()
{
  contacts_window_show();
  window_stack_remove(window, false);
}

void authorization_window_destroy()
{
  if (window) {
    window_destroy(window);
    window = NULL;
  }
}

//------- Window Methods --------

static void window_load(Window *window)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Authorization window is loaded");

  create_status_layer();
  status_layer_set_text(statusLayer, ERROR_NOT_AUTHORIZED_TEXT);
}

static void window_unload(Window *window)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Authorization window is unloaded");

  remove_status_layer();
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