#include <pebble_fonts.h>
#include "TextUtils.h"

GSize text_utils_max_content_size(const char *text, const GSize maxSize)
{
  return text_utils_max_content_size_with_font(text, maxSize, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
}

GSize text_utils_max_content_size_with_font(const char *text, const GSize maxSize, GFont const font)
{

  return text_utils_max_content_size_with_options(text, maxSize ,font,
      GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
}

GSize text_utils_max_content_size_with_options(const char *text, const GSize maxSize, GFont const font,
  GTextOverflowMode overflowMode, GTextAlignment alignment)
{
  GRect frame = (GRect) {.origin = GPointZero, .size = maxSize};

  return graphics_text_layout_get_content_size(text, font, frame, overflowMode, alignment);
}

char* text_utils_get_shared_buffer()
{
  static char buffer[SHARED_TEXT_BUFFER_LEN];

  return buffer;
}
