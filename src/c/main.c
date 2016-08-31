#include <pebble.h>

#include <pebble-generic-weather/pebble-generic-weather.h>
#include <pebble-events/pebble-events.h>

static Window *s_main_window;
static GFont s_time_font, s_date_font, s_weather_icon_font;
static TextLayer *s_date_layer, *s_stardate_layer, *s_parsec_layer, *s_batterypercent_layer, *s_lifesupport_text_layer, *s_temperature_layer, *s_city_layer, *icon_weather_layer, *s_time_layer;
static BitmapLayer *s_enterprise_layer;
static GBitmap *s_enterprise_bitmap;
static char api_key[50];
static char userweatherprovider[6];
static int s_step_count = 0, s_step_goal = 0, s_step_average = 0, s_battery_level, s_battery_charging;
static Layer *s_battery_layer, *s_lifesupport_layer, *s_lcars_layer, *s_bt_layer;
static bool F_Tick = S_TRUE;

static void read_persist()
{
	if(persist_exists(MESSAGE_KEY_APIKEY))
	{
		persist_read_string(MESSAGE_KEY_APIKEY, api_key, sizeof(api_key));
	}
	if(persist_exists(MESSAGE_KEY_FTICK))
	{
		F_Tick = persist_read_bool(MESSAGE_KEY_FTICK);
	}
	if(persist_exists(MESSAGE_KEY_WeatherProvide))
	{
		persist_read_string(MESSAGE_KEY_WeatherProvide, userweatherprovider, sizeof(userweatherprovider));
	}
}

static void store_persist()
{
	persist_write_string(MESSAGE_KEY_APIKEY, api_key);
  persist_write_bool(MESSAGE_KEY_FTICK, F_Tick);
	persist_write_string(MESSAGE_KEY_WeatherProvide, userweatherprovider);
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
//  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected==S_FALSE);
//  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected==S_TRUE);
//  layer_set_hidden(s_bt_icon_layer, connected==S_FALSE);
//  layer_set_hidden(s_nobt_layer, connected==S_TRUE);
  layer_mark_dirty(s_lcars_layer);
  layer_mark_dirty(s_bt_layer);
  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
    layer_mark_dirty(s_lcars_layer);
    layer_mark_dirty(s_bt_layer);
  }
}

static int get_health(HealthMetric metric, bool average)
{
  //HealthMetricStepCount
  //HealthMetricSleepSeconds
	time_t start = time_start_of_today();
	time_t end = time(NULL);
  const time_t endday = start + SECONDS_PER_DAY;

	int output;
	
	if(average)
	{
		output = (int)health_service_sum_averaged(metric, start, endday, HealthServiceTimeScopeDaily);
	}
	else
	{
		HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
	
		if(mask & HealthServiceAccessibilityMaskAvailable)
		{
			//APP_LOG(APP_LOG_LEVEL_INFO, "Metric: %d", (int)health_service_sum_today(metric));
			output = health_service_sum_today(metric);
		}
		else
		{
			//APP_LOG(APP_LOG_LEVEL_ERROR, "Data unavailable!");
			output = 0;
		}
	}
	
	return output;
}

static void lifesupport_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar which is up to 57 pixels wide
  int width = (int)(float)(((float)s_step_count / s_step_goal) * bounds.size.w);
  if (width > bounds.size.w){width = bounds.size.w;}

  //this is here for debugging
  //s_width_display = bounds.size.w;

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorBlueMoon);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void lcars_block(Layer *layer, GContext *ctx) {
  // LCARS COLORS GColorVividViolet GColorIcterine GColorLavenderIndigo GColorPictonBlue GColorRajah GColorBlue GColorChromeYellow GColorSunsetOrange
  //              dark purple       light yellow    light purple        light blue        tan
  bool connected = connection_service_peek_pebble_app_connection();
  graphics_context_set_fill_color(ctx, GColorLavenderIndigo);
  //lower elbo
  graphics_fill_rect(ctx, GRect(3,90,33,15), 15, GCornerTopLeft);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(24,94,12,11), 10, GCornerTopLeft);
  graphics_context_set_fill_color(ctx, GColorIcterine);
  //lower elbo +1
  graphics_fill_rect(ctx, GRect(38,90,21,4), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorVividViolet);
  //lower elbo +2
  graphics_fill_rect(ctx, GRect(62,90,21,4), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorBlue);
  //lower elbo +3
  graphics_fill_rect(ctx, GRect(86,90,21,4), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  //lower elbo +4
  graphics_fill_rect(ctx, GRect(110,90,34,4), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorChromeYellow);
  //lower elbo -1
  graphics_fill_rect(ctx, GRect(3,108,21,15), 0, GCornerNone);
  //lower elbo -2
  graphics_fill_rect(ctx, GRect(3,126,21,15), 0, GCornerNone);
  if (connected) {
    graphics_context_set_fill_color(ctx, GColorRajah);}
  else {
    graphics_context_set_fill_color(ctx, GColorRed);}
  //lower elbo -3
  graphics_fill_rect(ctx, GRect(3,144,21,24), 0, GCornerNone);
  //upper elbo
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, GRect(3,35,70,22), 15, GCornerBottomLeft);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(24,35,49,18), 10, GCornerBottomLeft);
  graphics_context_set_fill_color(ctx, GColorChromeYellow);
  //upper elbo +1
  graphics_fill_rect(ctx, GRect(76,53,21,4), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorSunsetOrange);
  //upper elbo +2
  graphics_fill_rect(ctx, GRect(100,53,44,4), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorLavenderIndigo);
  //upper elbo 11
  graphics_fill_rect(ctx, GRect(3,18,21,15), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  //upper elbo -2
  graphics_fill_rect(ctx, GRect(3,0,21,16), 0, GCornerNone);
}

static void weather_callback(GenericWeatherInfo *info, GenericWeatherStatus status) {
  switch(status) {
    case GenericWeatherStatusAvailable:
    {
      static char s_temp_buffer[16];
      static char s_city_buffer[6];
		static char s_city_buffer_2[7];
/*      snprintf(s_buffer, sizeof(s_buffer),"Temperature (K/C/F): %d/%d/%d\n\nName:\n%s\n\nDescription:\n%s",info->temp_k, info->temp_c, info->temp_f, info->name, info->description);
      text_layer_set_text(s_text_layer, s_buffer);
      snprintf(s_buffer, sizeof(s_buffer),"Temperature (K/C/F): %d/%d/%d\n\nName:\n%s\n\nDescription:\n%s",info->temp_k, info->temp_c, info->temp_f, info->name, info->description);
      text_layer_set_text(s_text_layer, s_buffer);*/
      if (F_Tick) {
        snprintf(s_temp_buffer, sizeof(s_temp_buffer),"%d", info->temp_f);}
      else {
        snprintf(s_temp_buffer, sizeof(s_temp_buffer),"%d", info->temp_c);}
      text_layer_set_text(s_temperature_layer, s_temp_buffer);
      snprintf(s_city_buffer, sizeof(s_city_buffer),"%s",info->name);
		snprintf(s_city_buffer_2, sizeof(s_city_buffer_2),"%s.", s_city_buffer);
      text_layer_set_text(s_city_layer, s_city_buffer_2);
		
		switch(info->condition)
			{
				case GenericWeatherConditionClearSky:
					if(info->day)
					{
						text_layer_set_text(icon_weather_layer, "J");
					}
					else
					{
						text_layer_set_text(icon_weather_layer, "D");
					}
					break;
				case GenericWeatherConditionFewClouds:
					if(info->day)
					{
						text_layer_set_text(icon_weather_layer, "F");
					}
					else
					{
						text_layer_set_text(icon_weather_layer, "E");
					}
					break;
				case GenericWeatherConditionScatteredClouds:
					text_layer_set_text(icon_weather_layer, "A");
					break;
				case GenericWeatherConditionBrokenClouds:
					text_layer_set_text(icon_weather_layer, "A");
					break;
				case GenericWeatherConditionShowerRain:
					text_layer_set_text(icon_weather_layer, "G");
					break;
				case GenericWeatherConditionRain:
					text_layer_set_text(icon_weather_layer, "G");
					break;
				case GenericWeatherConditionThunderstorm:
					text_layer_set_text(icon_weather_layer, "I");
					break;
				case GenericWeatherConditionSnow:
					text_layer_set_text(icon_weather_layer, "H");
					break;
				case GenericWeatherConditionMist:
					text_layer_set_text(icon_weather_layer, "C");
					break;
				case GenericWeatherConditionUnknown:
					text_layer_set_text(icon_weather_layer, "");
					break;
			}
    }
      break;
    case GenericWeatherStatusNotYetFetched:
      text_layer_set_text(s_city_layer, "Load...");
      break;
    case GenericWeatherStatusBluetoothDisconnected:
      text_layer_set_text(s_city_layer, "NoBT");
      break;
    case GenericWeatherStatusPending:
      text_layer_set_text(s_city_layer, "Load...");
      break;
    case GenericWeatherStatusFailed:
      text_layer_set_text(s_city_layer, "Failed");
      break;
    case GenericWeatherStatusBadKey:
      text_layer_set_text(s_city_layer, "BadKey");
      break;
    case GenericWeatherStatusLocationUnavailable:
      text_layer_set_text(s_city_layer, "NoLoc");
      break;
  }
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  s_battery_charging = state.is_charging;
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * bounds.size.w);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorBlueMoon);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void bt_update_proc(Layer *layer, GContext *ctx) {
 
  bool connected = connection_service_peek_pebble_app_connection();
  //GRect bounds = layer_get_bounds(layer);
  if(connected){
    graphics_context_set_stroke_color(ctx, GColorBlack);
  }
  else {
    graphics_context_set_stroke_color(ctx, GColorDarkGray);
  }
    
  graphics_draw_line(ctx, GPoint(10, 1), GPoint(10, 22));
  graphics_draw_line(ctx, GPoint(11, 1), GPoint(11, 22));
  graphics_draw_line(ctx, GPoint(11, 1), GPoint(16, 6));
  graphics_draw_line(ctx, GPoint(11, 2), GPoint(16, 7));
  graphics_draw_line(ctx, GPoint(16, 6), GPoint(5, 16));
  graphics_draw_line(ctx, GPoint(16, 7), GPoint(5, 17));
  graphics_draw_line(ctx, GPoint(10, 22), GPoint(16, 17));
  graphics_draw_line(ctx, GPoint(11, 22), GPoint(16, 16));
  graphics_draw_line(ctx, GPoint(16, 16), GPoint(5, 6));
  graphics_draw_line(ctx, GPoint(16, 17), GPoint(5, 7));  
  
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  // Create lcars background Layer
  s_lcars_layer = layer_create(GRect(0,0,144,177));
  layer_set_update_proc(s_lcars_layer, lcars_block);
  // Add to Window
  layer_add_child(window_layer, s_lcars_layer);

  s_bt_layer = layer_create(GRect(3,144,21,23));
  layer_set_update_proc(s_bt_layer, bt_update_proc);
  layer_add_child(window_layer, s_bt_layer);
  
  // Create GFonts
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LCARS_32));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LCARS_58));
  s_weather_icon_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WEATHERICON_38));
  
    // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(GRect(30, 83, bounds.size.w, 80));
  // Create GFont
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorChromeYellow);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create BitmapLayer to display the GBitmap 
  s_enterprise_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ENTERPRISE);
  s_enterprise_layer = bitmap_layer_create(GRect(0, 58, 144, 32));
  bitmap_layer_set_compositing_mode(s_enterprise_layer, GCompOpSet);
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_enterprise_layer, s_enterprise_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_enterprise_layer));

  // Create the TextLayer with specific bounds
  s_date_layer = text_layer_create(GRect(26, 15, bounds.size.w, 40));
  // Apply to TextLayer
  text_layer_set_font(s_date_layer, s_date_font);
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorChromeYellow);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Create the TextLayer with specific bounds
  icon_weather_layer = text_layer_create(GRect(29, 85, bounds.size.w, 40));
  // Apply to TextLayer
  text_layer_set_font(icon_weather_layer, s_weather_icon_font);
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(icon_weather_layer, GColorClear);
  text_layer_set_text_color(icon_weather_layer, GColorChromeYellow);
  text_layer_set_text_alignment(icon_weather_layer, GTextAlignmentLeft);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(icon_weather_layer));
  
  s_temperature_layer = text_layer_create(GRect(30, 120, bounds.size.w, 32));
  text_layer_set_text_color(s_temperature_layer, GColorChromeYellow);
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_font(s_temperature_layer, s_date_font);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));
  
  s_city_layer = text_layer_create(GRect(27, 150, bounds.size.w, 32));
  text_layer_set_text_color(s_city_layer, GColorChromeYellow);
  text_layer_set_background_color(s_city_layer, GColorClear);
  text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentLeft);
  text_layer_set_text(s_city_layer,"Load...");
  layer_add_child(window_layer, text_layer_get_layer(s_city_layer));
  
   // Create the TextLayer with specific bounds
  s_batterypercent_layer = text_layer_create(GRect(2, 54, bounds.size.w, 15));
  // Apply to TextLayer
  text_layer_set_font(s_batterypercent_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_batterypercent_layer, GColorClear);
  text_layer_set_text_color(s_batterypercent_layer, GColorChromeYellow);
  text_layer_set_text_alignment(s_batterypercent_layer, GTextAlignmentLeft);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_batterypercent_layer));

  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(4, 72, 33, 5));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  // Create life support meter Layer
  s_lifesupport_layer = layer_create(GRect(75, 147, 57, 5));
  layer_set_update_proc(s_lifesupport_layer, lifesupport_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_lifesupport_layer);

  // Create the TextLayer with specific bounds
  s_lifesupport_text_layer = text_layer_create(GRect(70, 150, bounds.size.w, 15));
  // Apply to TextLayer
  text_layer_set_font(s_lifesupport_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_lifesupport_text_layer, GColorClear);
  text_layer_set_text_color(s_lifesupport_text_layer, GColorChromeYellow);
  text_layer_set_text_alignment(s_lifesupport_text_layer, GTextAlignmentLeft);
  text_layer_set_text(s_lifesupport_text_layer, "Life Support");
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_lifesupport_text_layer));
  
      // Create the TextLayer with specific bounds
  s_parsec_layer = text_layer_create(GRect(95, 00, bounds.size.w, 15));
  // Apply to TextLayer
  text_layer_set_font(s_parsec_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_parsec_layer, GColorClear);
  text_layer_set_text_color(s_parsec_layer, GColorChromeYellow);
  text_layer_set_text_alignment(s_parsec_layer, GTextAlignmentLeft);
  // Add it as a child layer to the Window's root layer
  text_layer_set_text(s_parsec_layer, "Parsecs");
  layer_add_child(window_layer, text_layer_get_layer(s_parsec_layer));

  // Create the TextLayer with specific bounds
  s_stardate_layer = text_layer_create(GRect(26, 00, bounds.size.w, 15));
  // Apply to TextLayer
  text_layer_set_font(s_stardate_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_stardate_layer, GColorClear);
  text_layer_set_text_color(s_stardate_layer, GColorChromeYellow);
  text_layer_set_text_alignment(s_stardate_layer, GTextAlignmentLeft);
  // Add it as a child layer to the Window's root layer
  text_layer_set_text(s_stardate_layer, "Stardate");
  layer_add_child(window_layer, text_layer_get_layer(s_stardate_layer));
  

}

static void window_unload(Window *window) {
   text_layer_destroy(s_city_layer);
  text_layer_destroy(s_temperature_layer);
  //bitmap_layer_destroy(s_icon_layer);
    // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_stardate_layer);
  text_layer_destroy(s_parsec_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  //fonts_unload_custom_font(s_stardate_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_enterprise_bitmap);
  //gbitmap_destroy(s_bt_icon_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_enterprise_layer);
  //bitmap_layer_destroy(s_bt_icon_layer);
  
  //Destroy Layers
  layer_destroy(s_battery_layer);
  layer_destroy(s_lifesupport_layer);
  //layer_destroy(s_bt_icon_layer);
  window_destroy(window);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H%M" : "%I%M", tick_time);

  s_step_count = get_health(HealthMetricStepCount, S_FALSE);
  s_step_goal = get_health(HealthMetricStepCount, S_TRUE);
  //HealthMetricSleepSeconds

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  //char string to hold the date and step for the stardate value
  static char date_buffer[16];
  static char date_step_buffer[16];
  // Copy date into buffer from tm structure
  strftime(date_buffer, sizeof(date_buffer), "%m%d%y.", tick_time);
  //pull the current step count appending to the date buffer
  snprintf(date_step_buffer,sizeof(date_step_buffer),"%s%d",date_buffer,s_step_count);
  // Show the stardate
  text_layer_set_text(s_date_layer, date_step_buffer);
 
  //battery numerical display
  static char battery_text[] = "Pwr:100%";

// if (s_battery_charging) {
//    snprintf(battery_text, sizeof(battery_text), "CHARGING");
//  } else {
    snprintf(battery_text, sizeof(battery_text), "Pwr:%d%%", s_battery_level);
//  }
  text_layer_set_text(s_batterypercent_layer, battery_text); 

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  if(tick_time->tm_min % 30 == 0) {  generic_weather_fetch(weather_callback);}
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
	Tuple *data = dict_find(iterator, MESSAGE_KEY_READY);
	if(data)
	{
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Ready Received! Requesting weather.");
		generic_weather_fetch(weather_callback);
	}
	
	data = dict_find(iterator, MESSAGE_KEY_APIKEY);
	if(data)
	{
		strcpy(api_key, data->value->cstring);
		generic_weather_set_api_key(api_key);
//		generic_weather_fetch(weather_callback);
	}

	data = dict_find(iterator, MESSAGE_KEY_FTICK);
	if(data)
	{
		F_Tick = data->value->int32 == 1;
	}
  
	data = dict_find(iterator, MESSAGE_KEY_WeatherProvide);
	if(data)
	{
    strcpy(userweatherprovider, data->value->cstring);
    if (strcmp(userweatherprovider,"OpenWe")==0)
      {	generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);}
    else if(strcmp(userweatherprovider,"WUnder")==0)
    {generic_weather_set_provider(GenericWeatherProviderWeatherUnderground);}
		else if(strcmp(userweatherprovider,"For.io")==0)
    {generic_weather_set_provider(GenericWeatherProviderForecastIo);}
	generic_weather_fetch(weather_callback);
	}
}

static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
{
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
	
	read_persist();
  
  // Register with TickTimerService
  events_tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Make sure the time is displayed from the start
	
  // Register for battery level updates
  events_battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register for Bluetooth connection updates
  events_connection_service_subscribe((ConnectionHandlers) {.pebble_app_connection_handler = bluetooth_callback});
	update_time();
  
  // Generic Weather setup and display
  generic_weather_init();
  generic_weather_set_api_key(api_key);
  if (strcmp(userweatherprovider,"OpenWe")==0)
    {generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);}
  else if(strcmp(userweatherprovider,"WUnder")==0)
    {generic_weather_set_provider(GenericWeatherProviderWeatherUnderground);}
	else if(strcmp(userweatherprovider,"For.io")==0)
    {generic_weather_set_provider(GenericWeatherProviderForecastIo);}
//  generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);
	events_app_message_request_inbox_size(1024);
	events_app_message_register_inbox_received(inbox_received_callback, NULL);
	events_app_message_register_inbox_dropped(inbox_dropped_callback, NULL);
	events_app_message_register_outbox_failed(outbox_failed_callback, NULL);
	events_app_message_register_outbox_sent(outbox_sent_callback, NULL);
  events_app_message_open();
  //app_timer_register(3000, js_ready_handler, NULL);
}

static void deinit() {
  generic_weather_deinit();
	store_persist();
}

int main() {
  init();
  app_event_loop();
  deinit();
}