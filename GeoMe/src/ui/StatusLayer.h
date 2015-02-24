#pragma once

#include <pebble.h>
#include "MessagingChannel.h"

struct StatusLayer;
typedef struct StatusLayer StatusLayer;


StatusLayer* status_layer_create(GRect frame);

void status_layer_destroy(StatusLayer *statusLayer);


void status_layer_set_text(StatusLayer *statusLayer, const char* text);

void status_layer_set_image(StatusLayer *statusLayer, uint32_t resourceId);

void status_layer_remove_image(StatusLayer *statusLayer);


void status_layer_show_error(StatusLayer *statusLayer, MessageChannelResult reason);

void status_layer_show_error_with_custom_text(StatusLayer *statusLayer, const char *text);


Layer* status_layer_get_layer(StatusLayer *statusLayer);