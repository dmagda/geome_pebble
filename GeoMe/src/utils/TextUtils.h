#pragma once

#include <pebble.h>

#define SHARED_TEXT_BUFFER_LEN 128

GSize text_utils_max_content_size(const char *text, const GSize maxSize);

GSize text_utils_max_content_size_with_font(const char *text, const GSize maxSize, GFont const font);

GSize text_utils_max_content_size_with_options(const char *text, const GSize maxSize, GFont const font,
  GTextOverflowMode overflowMode, GTextAlignment alignment);

char* text_utils_get_shared_buffer();