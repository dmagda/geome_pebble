#include <Windows.h>
#include "string_resources.h"
#include "TextUtils.h"
#include "ContactsDataSource.h"

#define TEXT_LAYER_MAX_HEIGHT 20.0
#define PADDING_BETWEEN_TEXT_AND_LOGO 5.0

static GBitmap *logoImage = NULL;
static GFont hattoriFont = NULL;

static Window *window = NULL;
static BitmapLayer *logoLayer = NULL;
static TextLayer *textLayer = NULL;
static Layer *contentLayer = NULL;

static AppTimer *transitionTimer;


static void window_load(Window *window);
static void window_unload(Window *window);

static void add_window_layers();
static void remove_window_layers();

static void start_fetching_contacts();


//---- Public Methods --------

void welcome_window_show()
{
  if (window == NULL) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }

  window_stack_push(window, true);
}

void welcome_window_destroy()
{
  if (window) {
    window_destroy(window);
    window = NULL;
  }
}

void welcome_window_stop_transitioning()
{
  if (transitionTimer) {
    app_timer_cancel(transitionTimer);
    transitionTimer = NULL;
  }
}

//------- Window Methods --------

static void window_load(Window *window)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Welcome window is loaded");

  add_window_layers();
  start_fetching_contacts();
}

static void window_unload(Window *window)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Welcome window is unloaded");

  welcome_window_stop_transitioning();

  remove_window_layers();
}

//-------- Layers -------------------

static void add_window_layers()
{
  Layer *windowLayer = window_get_root_layer(window);
  GRect windowBounds = layer_get_frame(windowLayer);

  //image creation
  logoImage = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GEOME_WINDOW_LOGO);
  GRect logoLayerFrame = (GRect) {
    .origin = (GPoint){.x = (windowBounds.size.w - logoImage->bounds.size.w) / 2, .y = 0},
    .size = logoImage->bounds.size
  };

  logoLayer = bitmap_layer_create(logoLayerFrame);
  bitmap_layer_set_bitmap(logoLayer, logoImage);
  bitmap_layer_set_alignment(logoLayer, GAlignCenter);

  //text creation
  hattoriFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HATTORI_HANZO_26));
  GSize textSize = text_utils_max_content_size_with_font(APP_NAME_TEXT,
    (GSize){.w = windowBounds.size.w, .h = TEXT_LAYER_MAX_HEIGHT}, hattoriFont);
  GRect textFrame = (GRect) {
    .origin = (GPoint){.x = (windowBounds.size.w - textSize.w) / 2,
                       .y = logoLayerFrame.size.h + PADDING_BETWEEN_TEXT_AND_LOGO},
    .size = textSize
  };

  textLayer = text_layer_create(textFrame);
  text_layer_set_text(textLayer, APP_NAME_TEXT);
  text_layer_set_font(textLayer, hattoriFont);

  //content layer creation
  GSize contentLayerSize = (GSize) {.w = windowBounds.size.w, .h = textFrame.origin.y + textFrame.size.h};
  GRect contentLayerRect = (GRect) {
    .origin = (GPoint){.x = 0, .y = (windowBounds.size.h - contentLayerSize.h) / 2},
    .size = contentLayerSize
  };

  contentLayer = layer_create(contentLayerRect);

  //adding layer to its parents
  layer_add_child(contentLayer, bitmap_layer_get_layer(logoLayer));
  layer_add_child(contentLayer, text_layer_get_layer(textLayer));
  layer_add_child(windowLayer, contentLayer);
}

static void remove_window_layers()
{
  if (textLayer) {
    text_layer_destroy(textLayer);
    textLayer = NULL;
  }

  if (logoLayer) {
    bitmap_layer_destroy(logoLayer);
    logoLayer = NULL;
  }

  if (logoImage) {
    gbitmap_destroy(logoImage);
    logoImage = NULL;
  }

  if (contentLayer) {
    layer_destroy(contentLayer);
    contentLayer = NULL;
  }

  if (hattoriFont) {
    fonts_unload_custom_font(hattoriFont);
    hattoriFont = NULL;
  }
}

//------------ Contacts Loading and Timer ------------

static void timer_fired(void* data)
{
  transitionTimer = NULL;

  contacts_window_show();
  window_stack_remove(window, false);
}

static void start_fetching_contacts()
{
  //IMPL_NOTE: if uncomment the line below then 'Still allocated <4294967292B>' error starts appearing
  //from time to time. However, there are no such an issue arises when executing contacts_ds_fetch_contacts
  //in ContactsWindow multiple times.
  //In addition calling this method on Android upon start up will lead to a situation that a message
  //to a phone is not delivered.
  //contacts_ds_fetch_contacts();

  transitionTimer = app_timer_register(1500 /* 1.5 secs */, timer_fired, NULL);
}