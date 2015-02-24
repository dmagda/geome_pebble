
#include "Windows.h"
#include "ContactsDataSource.h"
#include "StatusLayer.h"
#include "string_resources.h"

static Window *window;
static MenuLayer *menuLayer = NULL;
static StatusLayer *statusLayer = NULL;

static void window_load(Window *window);
static void window_unload(Window *window);

static void show_status_layer_with_text(const char* text);
static void show_error_on_status_layer(MessageChannelResult reason);
static void remove_status_layer();

static void show_menu_layer();
static void remove_menu_layer();
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data);
static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data);
static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data);

static void start_fetching_contacts();
static void on_contacts_fetched(Contact **contacts, int count);
static void on_contacts_fetching_failed(MessageChannelResult reason);

//---- public methods ---

void contacts_window_show()
{
  if (window == NULL) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload
    });
  }

  window_stack_push(window, false);
}

void contacts_window_destroy()
{
  if (window) {
    window_destroy(window);
    window = NULL;
  }
}


//---- Window ---

static void window_load(Window *currentWindow) {

  int contactsCount = 0;
  contacts_ds_get_contacts(NULL, &contactsCount);

  if (contactsCount != 0) {
    show_menu_layer();

  } else {
    start_fetching_contacts();
  }
}

static void window_unload(Window *currentWindow) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Contacts window is unloaded");

  contacts_ds_clear_handlers();

  remove_status_layer();
  remove_menu_layer();
}

//----------- Status Layer -----------------------------


static void show_status_layer_with_text(const char* text)
{
  if (statusLayer == NULL) {
    Layer *windowLayer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(windowLayer);

    statusLayer = status_layer_create((GRect) {.origin = GPointZero, .size = bounds.size});
    status_layer_set_text(statusLayer, text);
    status_layer_set_image(statusLayer, RESOURCE_ID_IMAGE_SAND_WATCH);

    layer_add_child(windowLayer, status_layer_get_layer(statusLayer));

  } else {
    status_layer_set_text(statusLayer, text);
  }
}


static void show_error_on_status_layer(MessageChannelResult reason)
{
  if (reason == MSG_CHN_GENERAL_ERROR) {
    status_layer_show_error_with_custom_text(statusLayer, UNABLE_TO_LOAD_CONTACTS_TEXT);
  } else {
    status_layer_show_error(statusLayer, reason);
  }
}

static void remove_status_layer()
{
  if (statusLayer) {
    status_layer_destroy(statusLayer);
    statusLayer = NULL;
  }
}


//---------- Menu Layer -----------------

static void show_menu_layer()
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  menuLayer = menu_layer_create(bounds);

  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(menuLayer, NULL, (MenuLayerCallbacks){
    .get_num_rows = menu_get_num_rows_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(menuLayer, window);

  // Add it to the window for display
  layer_add_child(window_layer, menu_layer_get_layer(menuLayer));
}

static void remove_menu_layer()
{
  if (menuLayer) {
    menu_layer_destroy(menuLayer);
    menuLayer = NULL;
  }
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  int count;

  contacts_ds_get_contacts(NULL, &count);

  return (uint16_t)count;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  Contact **contacts;

  contacts_ds_get_contacts(&contacts, NULL);

  menu_cell_title_draw(ctx, cell_layer, contacts[cell_index->row]->name);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Contact is selected");

    Contact **contacts;

    contacts_ds_get_contacts(&contacts, NULL);

    sending_location_window_show(contacts[cell_index->row]);
}

//---------- Contacts Processing -------------

static void start_fetching_contacts()
{
  contacts_ds_set_handlers((ContactsDataSourceHandlers) {
    .success = on_contacts_fetched,
    .failure = on_contacts_fetching_failed
  });

  show_status_layer_with_text(LOADING_CONTACTS_TEXT);
  MessageChannelResult result = contacts_ds_fetch_contacts();

  if (result != MSG_CHN_OK) {
    show_error_on_status_layer(result);
    contacts_ds_clear_handlers();
  }
}

static void on_contacts_fetched(Contact **contacts, int count)
{
  remove_status_layer();
  show_menu_layer();

  contacts_ds_clear_handlers();
}

static void on_contacts_fetching_failed(MessageChannelResult reason)
{
  show_error_on_status_layer(reason);
  contacts_ds_clear_handlers();
}