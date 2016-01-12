#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_am_pm_layer;
static TextLayer *s_date_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static GFont s_time_font;
static GFont s_am_pm_font;
static GFont s_date_font;

static int s_battery_level;

struct tm *tick_time; 

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  tick_time = localtime(&temp);

  // Write the current time 
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  // Write the current AM/PM 
  static char s_buffer2[8];
  strftime(s_buffer2, sizeof(s_buffer2), "%p", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(s_am_pm_layer, s_buffer2);
}

static void update_date(){
  // Update the current Date
  static char s_buffer[16];
  strftime(s_buffer, sizeof(s_buffer), "%a %d %b", tick_time);
  
  // Display this date on the TextLayer
  text_layer_set_text(s_date_layer, s_buffer); 
}

static void update_am_pm(){
  // Write the current AM/PM 
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), "%p", tick_time);
  
  // Display this date on the TextLayer
  text_layer_set_text(s_am_pm_layer, s_buffer); 
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  if(s_battery_level > 50){
    window_set_background_color(s_main_window, GColorMediumAquamarine); 
  } else if (s_battery_level > 25){
    window_set_background_color(s_main_window, GColorRajah);
  } else {
    window_set_background_color(s_main_window, GColorSunsetOrange);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);

  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);

  // Maybe this will help
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  
  // Time box
  s_time_layer = text_layer_create(
    GRect(0, 73, bounds.size.w, 50)  
  );
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // AM/PM box
  s_am_pm_layer = text_layer_create(
    GRect(65, 83, bounds.size.w, 50)
  );
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_am_pm_layer, GColorClear);
  text_layer_set_text_color(s_am_pm_layer, GColorWhite);
  text_layer_set_text(s_am_pm_layer, "PM");
  text_layer_set_text_alignment(s_am_pm_layer, GTextAlignmentCenter);
  
  // Date Layer
  s_date_layer = text_layer_create(
    GRect(0,120,bounds.size.w, 50)
  );
  
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "PM");
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CABIN_40));
  s_am_pm_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CABIN_SMALL_15));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CABIN_BOLD_15));
  
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_font(s_am_pm_layer, s_am_pm_font);
  text_layer_set_font(s_date_layer, s_date_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_am_pm_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_am_pm_layer);
  text_layer_destroy(s_date_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_am_pm_font);
  fonts_unload_custom_font(s_date_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  // Unsubscribe
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorClear);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();
  update_date();
  update_am_pm();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}
