/*
  Pet Sounds - Pebble Watch Face

  Created by John Spahr (johnspahr.org)

  Thanks to: "PebbleFaces" example, the watchface creation guide hosted by Rebble, the Rebble team, and Twebe Bebe for the request!
*/
#include <pebble.h>

static Window *window;            // window object
static BitmapLayer *bitmap_layer; // layer to display the bitmap
static GBitmap *pet_sounds;       // color pet sounds cover bitmapß
static TextLayer *s_time_layer;   // the time (text layer)
static TextLayer *s_date_layer;   // the date (text layer)
static GFont s_time_font;         // custom time font
static GFont s_date_font;         // custom date font

bool isConnected = true; // global boolean value that is used to track if the watch is connected to the phone

static void update_time()
{
  // get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // write the current time and date into buffers
  static char s_buffer[8];
  static char date_buffer[8];

  // display this time on the TextLayer...
  if (isConnected)
  {
    // if watch is connected to phone, just display time
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  }
  else
  {
    // if watch is NOT connected to phone, show brackets around time to indicate this
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "[%H:%M]" : "[%I:%M]", tick_time);
  }
  text_layer_set_text(s_time_layer, s_buffer); // set text layer to display time

  strftime(date_buffer, sizeof(date_buffer), clock_is_24h_style() ? "%d/%m" : "%m/%d", tick_time); // if on 24h time, assume date follows non-American format... otherwise, use american format for date
  text_layer_set_text(s_date_layer, date_buffer);                                                  // set text layer to display date (woohoo)
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time(); // update the time on timer tick
}

static void load_background()
{
  pet_sounds = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PET_SOUNDS);
  bitmap_layer_set_bitmap(bitmap_layer, pet_sounds); // set bitmap layer image
}

static void bluetooth_callback(bool connected)
{
  if (connected)
  {
    // when connected...
    isConnected = true; // indicate that watch is connected to phone globally
    update_time();      // update time in case disconnection brackets are visible
  }
  else
  {
    // otherwise...
    isConnected = false;  // inidcate that watch is NOT connected to phone globally
    vibes_double_pulse(); // issue a vibrating alert on disconnect
    update_time();        // update time to display disconnection brackets
  }
}

static void window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window); // get current window layer
  GRect bounds = layer_get_bounds(window_layer);       // get screen bounds

  bitmap_layer = bitmap_layer_create(bounds);                          // create bitmap layer
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer)); // add bitmap layer to window

  // create time text layer...
  s_time_layer = text_layer_create(
      GRect(PBL_IF_COLOR_ELSE(3, 0), PBL_IF_ROUND_ELSE(6, 0), bounds.size.w, bounds.size.h));

  // create date text layer...
  s_date_layer = text_layer_create(
      GRect(PBL_IF_COLOR_ELSE(3, 0), PBL_IF_ROUND_ELSE(34, 30), bounds.size.w, bounds.size.h));

  // create GFonts for text layers...
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_COOPER_28));

  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CLARENDON_18));

  // time text layer setup...
  text_layer_set_background_color(s_time_layer, GColorClear);        // set clear background color
  text_layer_set_text_color(s_time_layer, GColorWhite);              // white text color
  text_layer_set_text(s_time_layer, "00:00");                        // set text to 00:00
  text_layer_set_font(s_time_layer, s_time_font);                    // set font to Cooper, 28pt
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter); // center align text

  // date text layer setup...
  text_layer_set_background_color(s_date_layer, GColorClear);        // set clear background color
  text_layer_set_text_color(s_date_layer, GColorYellow);             // yellow text color
  text_layer_set_text(s_date_layer, "0/0");                          // set text to 0/0
  text_layer_set_font(s_date_layer, s_date_font);                    // set font to Clarendon, 18pt
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter); // center align text

  // add clock text layers to window layer...
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // load album cover bitmap
  load_background();
}

static void window_unload(Window *window)
{
  // get rid of bitmap layer
  bitmap_layer_destroy(bitmap_layer);

  // destroy clock text layers...
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);

  // unload GFonts...
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
}

static void init(void)
{
  window = window_create(); // create a new window. woohoo!
  window_set_window_handlers(window, (WindowHandlers){
                                         .load = window_load,
                                         .unload = window_unload,
                                     }); // set functions for window handlers

  window_stack_push(window, true); // push window

  update_time(); // register with TickTimerService

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler); // set tick_handler function as time handler (start keeping time, essentially. kinda important on a watch.)

  // register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers){
      .pebble_app_connection_handler = bluetooth_callback});
}

static void deinit(void)
{
  window_destroy(window); // literally obliterate the window
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}