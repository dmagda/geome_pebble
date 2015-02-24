#include "StatusLayer.h"
#include "TextUtils.h"
#include "string_resources.h"

#define CONTENT_PADDING_X 4
#define IMAGE_PADDING 15


struct StatusLayer
{
  Layer *rootLayer;
  Layer *contentLayer;
  TextLayer *textLayer;
  BitmapLayer *bitmapLayer;

  GBitmap *bitmap;
  uint32_t bitmapResourceId;
};


void create_content_layer(StatusLayer *statusLayer);
void update_content_layer(StatusLayer *statusLayer);
int16_t get_content_max_width(StatusLayer *statusLayer);

void remove_bitmap_layer(StatusLayer *statusLayer);

//---------- Public Methods --------------

StatusLayer* status_layer_create(GRect frame)
{
  StatusLayer *statusLayer = (StatusLayer*)malloc(sizeof(StatusLayer));
  if (!statusLayer) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OOM in StatusLayer creation function");
    return NULL;
  }
  memset(statusLayer, 0, sizeof(StatusLayer));

  statusLayer->rootLayer = layer_create(frame);

  return statusLayer;
}

void status_layer_destroy(StatusLayer *statusLayer)
{
  if (!statusLayer) {
    return;
  }

  if (statusLayer->rootLayer) {
    layer_destroy(statusLayer->rootLayer);
    statusLayer->rootLayer = NULL;
  }

  if (statusLayer->contentLayer) {
    layer_destroy(statusLayer->contentLayer);
    statusLayer->contentLayer = NULL;
  }

  if (statusLayer->textLayer) {
    text_layer_destroy(statusLayer->textLayer);
    statusLayer->textLayer = NULL;
  }

  remove_bitmap_layer(statusLayer);

  free(statusLayer);
}


void status_layer_set_text(StatusLayer *statusLayer, const char* text)
{
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  GSize maxSize = (GSize){.w = get_content_max_width(statusLayer),
    layer_get_bounds(statusLayer->rootLayer).size.h};

  GSize textSize = text_utils_max_content_size_with_options(text, maxSize, font,
    GTextOverflowModeWordWrap, GTextAlignmentCenter);

  //IMPL_NOTE: by some reason invalid max rectangle is returned
  textSize.h += 3;
  textSize.w = maxSize.w;

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Text size (%d, %d)", textSize.w, textSize.h);

  if (statusLayer->textLayer == NULL) {
    statusLayer->textLayer = text_layer_create((GRect){.origin = GPointZero, .size = textSize});
    text_layer_set_font(statusLayer->textLayer, font);
    text_layer_set_overflow_mode(statusLayer->textLayer, GTextOverflowModeWordWrap);
    text_layer_set_text_alignment(statusLayer->textLayer, GTextAlignmentCenter);
    text_layer_set_text(statusLayer->textLayer, text);

    create_content_layer(statusLayer);

    layer_add_child(statusLayer->contentLayer, text_layer_get_layer(statusLayer->textLayer));

  } else {
    text_layer_set_text(statusLayer->textLayer, text);
    text_layer_set_size(statusLayer->textLayer, textSize);
  }

  update_content_layer(statusLayer);
}

void status_layer_set_image(StatusLayer *statusLayer, uint32_t resourceId)
{
  if (statusLayer->bitmap && statusLayer->bitmapResourceId == resourceId) {
    return;
  }

  remove_bitmap_layer(statusLayer);
  create_content_layer(statusLayer);

  statusLayer->bitmapResourceId = resourceId;

  statusLayer->bitmap = gbitmap_create_with_resource(resourceId);
  GRect bitmapFrame = (GRect){.origin = GPointZero, .size = statusLayer->bitmap->bounds.size};

  statusLayer->bitmapLayer = bitmap_layer_create(bitmapFrame);
  bitmap_layer_set_bitmap(statusLayer->bitmapLayer, statusLayer->bitmap);
  bitmap_layer_set_alignment(statusLayer->bitmapLayer, GAlignCenter);

  layer_add_child(statusLayer->contentLayer, bitmap_layer_get_layer(statusLayer->bitmapLayer));

  update_content_layer(statusLayer);
}

void status_layer_remove_image(StatusLayer *statusLayer)
{
  remove_bitmap_layer(statusLayer);

  if (statusLayer->contentLayer) {
    update_content_layer(statusLayer);
  }
}


void status_layer_show_error(StatusLayer *statusLayer, MessageChannelResult reason)
{
  char *text;

  switch(reason) {
    case MSG_CHN_NOT_AUTHORIZED:
      text = ERROR_NOT_AUTHORIZED_TEXT;
      break;

    case MSG_CHN_PHONE_NOT_CONNECTED:
      text = ERROR_PHONE_NOT_CONNECTED_TEXT;
      break;

    case MSG_CHN_PHONE_APP_NOT_RUNNING:
      text = ERROR_PHONE_APP_NOT_RUNNING_TEXT;
      break;

    case MSG_CHN_HTTP_TIMEOUT:
      text = ERROR_HTTP_REQUEST_TIMEOUT_TEXT;
      break;

    case MSG_CHN_LOCATION_NOT_DETERMINED:
      text = ERROR_LOCATION_NOT_DETERMINED_TEXT;
      break;

    default:
      text = ERROR_GENERAL_TEXT;
  }

  status_layer_show_error_with_custom_text(statusLayer, text);
}

void status_layer_show_error_with_custom_text(StatusLayer *statusLayer, const char *text)
{
  status_layer_set_text(statusLayer, text);
  status_layer_set_image(statusLayer, RESOURCE_ID_IMAGE_ERROR);
}


Layer* status_layer_get_layer(StatusLayer *statusLayer)
{
  return statusLayer->rootLayer;
}


//--------------- Private Methods -----------

void create_content_layer(StatusLayer *statusLayer)
{
  if (statusLayer->contentLayer) {
    return;
  }

  int16_t width = get_content_max_width(statusLayer);

  statusLayer->contentLayer = layer_create((GRect){
      .origin = (GPoint){.x = (layer_get_bounds(statusLayer->rootLayer).size.w - width) / 2, .y = 0},
      .size = (GSize){.w = width, .h = 0}
  });

  layer_add_child(statusLayer->rootLayer, statusLayer->contentLayer);
}

void update_content_layer(StatusLayer *statusLayer) {
  int16_t contentHeight = 0;
  int16_t upBottomPadding = 0;

  GRect contentLayerFrame = layer_get_frame(statusLayer->contentLayer);

  if (statusLayer->textLayer) {
    Layer *textLayersLayer = text_layer_get_layer(statusLayer->textLayer);
    GRect frame = layer_get_frame(textLayersLayer);

    frame.origin.y = upBottomPadding;

    contentHeight = frame.origin.y + frame.size.h;

    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Text frame origin = (%d, %d), size = (%d, %d)", frame.origin.x, frame.origin.y,
    //    frame.size.w, frame.size.h);

    layer_set_frame(textLayersLayer, frame);
  }

  if (statusLayer->bitmapLayer) {
    Layer *bitmapLayersLayer = bitmap_layer_get_layer(statusLayer->bitmapLayer);
    GRect frame = layer_get_frame(bitmapLayersLayer);

    frame.origin.x = (contentLayerFrame.size.w - frame.size.w) / 2;
    frame.origin.y = contentHeight > 0 ? contentHeight + IMAGE_PADDING : upBottomPadding;

    contentHeight = frame.origin.y + frame.size.h;

    layer_set_frame(bitmapLayersLayer, frame);
  }

  contentHeight += upBottomPadding;

  contentLayerFrame.size.h = contentHeight;
  contentLayerFrame.origin.y = (layer_get_bounds(statusLayer->rootLayer).size.h - contentHeight) / 2;

  //APP_LOG(APP_LOG_LEVEL_ERROR, "Content frame origin = (%d, %d), size = (%d, %d)", contentLayerFrame.origin.x,
  //  contentLayerFrame.origin.y, contentLayerFrame.size.w, contentLayerFrame.size.h);

  layer_set_frame(statusLayer->contentLayer, contentLayerFrame);

  layer_mark_dirty(statusLayer->rootLayer);
}


void remove_bitmap_layer(StatusLayer *statusLayer)
{
  if (statusLayer->bitmapLayer) {
    layer_remove_from_parent(bitmap_layer_get_layer(statusLayer->bitmapLayer));
    bitmap_layer_destroy(statusLayer->bitmapLayer);
    statusLayer->bitmapLayer = NULL;
  }

  if (statusLayer->bitmap) {
    gbitmap_destroy(statusLayer->bitmap);
    statusLayer->bitmap = NULL;
    statusLayer->bitmapResourceId = 0;
  }
}

int16_t get_content_max_width(StatusLayer *statusLayer)
{
  return layer_get_bounds(statusLayer->rootLayer).size.w - CONTENT_PADDING_X * 2;
}



