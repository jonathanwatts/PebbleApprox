#include <pebble.h>
#include "num2words.h"

#define BUFFER_SIZE 86

static Window *s_main_window;
static TextLayer *s_text_layer;
static char s_buffer[BUFFER_SIZE];

static TextLayer *s_battery_layer;
static TextLayer *s_connection_layer;

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(s_connection_layer, connected ? "connected" : "disconnected");
}


static void update_time(struct tm *t) {
  fuzzy_time_to_words(t->tm_hour, t->tm_min, s_buffer, BUFFER_SIZE);
  text_layer_set_text(s_text_layer, s_buffer);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_text_layer = text_layer_create(GRect(0, 1, bounds.size.w, bounds.size.h - 20));
  text_layer_set_background_color(s_text_layer, GColorBlack);
  text_layer_set_text_color(s_text_layer, GColorWhite);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  
  s_connection_layer = text_layer_create(GRect(0, 135, bounds.size.w, 34));
  text_layer_set_text_color(s_connection_layer, GColorWhite);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentRight);
  handle_bluetooth(bluetooth_connection_service_peek());

  s_battery_layer = text_layer_create(GRect(0, 150, bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "100% charged");
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);

  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  text_layer_destroy(s_connection_layer);
  text_layer_destroy(s_battery_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}