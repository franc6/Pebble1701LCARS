#include <pebble.h>

#include <pebble-generic-weather/pebble-generic-weather.h>
#include <pebble-events/pebble-events.h>

static Window *s_main_window;
static GFont s_time_font, s_date_font, s_weather_icon_font;
static TextLayer *s_date_layer, *s_stardate_layer, *s_parsec_layer, *s_batterypercent_layer, *s_lifesupport_text_layer, *s_temperature_layer, *s_city_layer, *icon_weather_layer, *s_time_layer, *s_weekname_layer, *s_weeknum_layer, *s_weatherdescript_layer;
static BitmapLayer *s_enterprise_layer;
static GBitmap *s_enterprise_bitmap;
static char api_key[50];
static char ReplacementWeatherMessage[21];
static char userweatherprovider[7];
static char healthmeterselector[12];
static char WeekNumDisp[5];
static char StringUserStepGoal[6];
static char battery_text[] = "Pwr:100%";
static int s_step_count = 1, s_step_goal = 1, s_step_average = 1, prev_s_step_count = 1, UserStepGoal = 7500,s_battery_level, s_battery_charging;
static Layer *s_battery_layer, *s_lifesupport_layer, *s_lcars_layer, *s_bt_layer, *s_warp_layer;
static bool F_Tick = S_TRUE;
static bool PowerDisp = S_TRUE;
static bool UKDateFormat = S_FALSE;
static bool WeekdayNameDisp = S_TRUE;
static bool WeatherDescriptionDisp = S_FALSE;
static int warpsequence = 0;
#define TIMER_INTERVAL_MS 500
static PropertyAnimation *image_property_animation;
static bool WeatherSetupStatusKey = S_FALSE, WeatherSetupStatusProvider = S_FALSE;

static void read_persist()
{
	if(persist_exists(MESSAGE_KEY_APIKEY))
	{
		persist_read_string(MESSAGE_KEY_APIKEY, api_key, sizeof(api_key));
	}
	if(persist_exists(MESSAGE_KEY_STEPGOAL))
	{
		persist_read_string(MESSAGE_KEY_STEPGOAL, StringUserStepGoal, sizeof(StringUserStepGoal));
    UserStepGoal = atoi(StringUserStepGoal);
	}
	if(persist_exists(MESSAGE_KEY_WEATHERREPLACEMENT))
	{
		persist_read_string(MESSAGE_KEY_WEATHERREPLACEMENT, ReplacementWeatherMessage, sizeof(ReplacementWeatherMessage));
	}
	if(persist_exists(MESSAGE_KEY_FTICK))
	{
		F_Tick = persist_read_bool(MESSAGE_KEY_FTICK);
	}
	if(persist_exists(MESSAGE_KEY_UKDATE))
	{
		UKDateFormat = persist_read_bool(MESSAGE_KEY_UKDATE);
	}
	if(persist_exists(MESSAGE_KEY_WEATHERDESCRIPTION))
	{
		WeatherDescriptionDisp = persist_read_bool(MESSAGE_KEY_WEATHERDESCRIPTION);
	}
	if(persist_exists(MESSAGE_KEY_WEEKDAYNAME))
	{
		WeekdayNameDisp = persist_read_bool(MESSAGE_KEY_WEEKDAYNAME);
	}
	if(persist_exists(MESSAGE_KEY_POWERDISPLAY))
	{
		PowerDisp = persist_read_bool(MESSAGE_KEY_POWERDISPLAY);
	}
	if(persist_exists(MESSAGE_KEY_WeatherProvide))
	{
		persist_read_string(MESSAGE_KEY_WeatherProvide, userweatherprovider, sizeof(userweatherprovider));
	}
	if(persist_exists(MESSAGE_KEY_WEEKNUMFORMAT))
	{
		persist_read_string(MESSAGE_KEY_WEEKNUMFORMAT, WeekNumDisp, sizeof(WeekNumDisp));
	}
	if(persist_exists(MESSAGE_KEY_HealthMeterSelect))
	{
		persist_read_string(MESSAGE_KEY_HealthMeterSelect, healthmeterselector, sizeof(healthmeterselector));
	}
}

static void store_persist()
{
  persist_write_string(MESSAGE_KEY_STEPGOAL, StringUserStepGoal);
	persist_write_string(MESSAGE_KEY_APIKEY, api_key);
	persist_write_string(MESSAGE_KEY_WEATHERREPLACEMENT, ReplacementWeatherMessage);
  persist_write_bool(MESSAGE_KEY_FTICK, F_Tick);
  persist_write_bool(MESSAGE_KEY_UKDATE, UKDateFormat);
  persist_write_bool(MESSAGE_KEY_WEATHERDESCRIPTION, WeatherDescriptionDisp);
  persist_write_bool(MESSAGE_KEY_WEEKDAYNAME, WeekdayNameDisp);
  persist_write_bool(MESSAGE_KEY_POWERDISPLAY, PowerDisp);
	persist_write_string(MESSAGE_KEY_WeatherProvide, userweatherprovider);
	persist_write_string(MESSAGE_KEY_WEEKNUMFORMAT, WeekNumDisp);
	persist_write_string(MESSAGE_KEY_HealthMeterSelect, healthmeterselector);
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

static int get_health(HealthMetric metric, int average)
{
  //HealthMetricStepCount
  //HealthMetricSleepSeconds
	time_t start = time_start_of_today();
	time_t end = time(NULL);

	int output =0;
  switch(average) {
	case 0: {
		HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
			if(mask & HealthServiceAccessibilityMaskAvailable)
		{
			//APP_LOG(APP_LOG_LEVEL_INFO, "Metric: %d", (int)health_service_sum_today(metric));
			output = health_service_sum_today(metric);
		}
    else {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Health metric unavaiilable");
      output = 0;
    }
    break;    
  }
  case 1: {
    // Check that an averaged value is accessible
    HealthServiceAccessibilityMask mask = 
          health_service_metric_averaged_accessible(metric, start, end, HealthServiceTimeScopeWeekly);
    if(mask & HealthServiceAccessibilityMaskAvailable) {
      // Average is available, read it
      output = (int)health_service_sum_averaged(metric, start, end, HealthServiceTimeScopeWeekly);
      }
//    output = (int)health_service_sum_averaged(metric, start, end, HealthServiceTimeScopeDaily);
    break;
  }
  case 2: {
    HealthServiceAccessibilityMask mask = 
          health_service_metric_averaged_accessible(metric, start, start+SECONDS_PER_DAY, HealthServiceTimeScopeWeekly);
    if(mask & HealthServiceAccessibilityMaskAvailable) {
      // Average is available, read it
      output = (int)health_service_sum_averaged(metric, start, start+SECONDS_PER_DAY, HealthServiceTimeScopeWeekly);
      }
    break;
  }
	default:
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Haalth data unavailable!");
		}
	}
	return output;
}

static void lifesupport_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar which is up to 57 pixels wide
  int goalwidth = (int)(float)(((float)s_step_count / s_step_goal) * bounds.size.w);
  if (goalwidth > bounds.size.w){goalwidth = bounds.size.w;}
  int avgwidth = (int)(float)(((float)s_step_count / s_step_average) * bounds.size.w);
  if (avgwidth > bounds.size.w){avgwidth = bounds.size.w;}

  // Draw the background
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_rect(ctx,bounds);
  //  graphics_context_set_fill_color(ctx, GColorWhite);
  //  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
/*  if (strcmp(healthmeterselector,"currenttime")==0)
    {graphics_context_set_fill_color(ctx, GColorMidnightGreen);}
  else
    {graphics_context_set_fill_color(ctx, GColorBlueMoon);}
*/
  graphics_context_set_fill_color(ctx, GColorBlueMoon);
  graphics_fill_rect(ctx, GRect(0, 0, goalwidth, bounds.size.h), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorMidnightGreen);
  graphics_fill_rect(ctx, GRect(0, 0, avgwidth, bounds.size.h-7), 0, GCornerNone);
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
  //upper elbo -1
  graphics_fill_rect(ctx, GRect(3,18,21,15), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  //upper elbo -2
  graphics_fill_rect(ctx, GRect(3,0,21,16), 0, GCornerNone);
}

static void weather_callback(GenericWeatherInfo *info, GenericWeatherStatus status) {
  if (strlen(ReplacementWeatherMessage)==0){
  switch(status) {
    case GenericWeatherStatusAvailable:
    {
      static char s_temp_buffer[16];
      static char s_city_buffer[32];
      static char s_weather_description[14];
//      snprintf(s_buffer, sizeof(s_buffer),"Temperature (K/C/F): %d/%d/%d\n\nName:\n%s\n\nDescription:\n%s",info->temp_k, info->temp_c, info->temp_f, info->name, info->description);
      if (F_Tick) {
        snprintf(s_temp_buffer, sizeof(s_temp_buffer),"%d", info->temp_f);}
      else {
        snprintf(s_temp_buffer, sizeof(s_temp_buffer),"%d", info->temp_c);}
      text_layer_set_text(s_temperature_layer, s_temp_buffer);
      snprintf(s_city_buffer, sizeof(s_city_buffer),"%s",info->name);
      text_layer_set_text(s_city_layer, s_city_buffer);
      if (WeatherDescriptionDisp) {
        snprintf(s_weather_description, sizeof(s_weather_description),"%s",info->description);
        text_layer_set_text(s_weatherdescript_layer, s_weather_description);
      }		
      else {text_layer_set_text(s_weatherdescript_layer, " ");}
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
  }}
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

  if (s_battery_level>0) {
    // Draw the background
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    // Draw the bar
    graphics_context_set_fill_color(ctx, GColorBlueMoon);
    graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
  }
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

static void warp_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Draw the bar
  if (warpsequence>0) {
    graphics_context_set_fill_color(ctx, GColorPictonBlue);
    graphics_fill_rect(ctx, GRect(0, 6-warpsequence, bounds.size.w,2*warpsequence+1), 2, GCornersAll);
  }
}

static void update_battery(){
  battery_callback(battery_state_service_peek());
}

static void trigger_animation() {

  if ((warpsequence>0) && (warpsequence<8)) {
    warpsequence++;
    layer_mark_dirty(s_warp_layer);
    app_timer_register(TIMER_INTERVAL_MS, trigger_animation, NULL);
  }
  else if (warpsequence ==8){
    warpsequence = 0;
    layer_mark_dirty(s_warp_layer);
  
  // Set start and end
  GRect from_frame = layer_get_frame(bitmap_layer_get_layer(s_enterprise_layer));
  GRect to_frame = GRect(144, 58, 144, 32);

  // Create the animation
  image_property_animation = property_animation_create_layer_frame(bitmap_layer_get_layer(s_enterprise_layer), &from_frame, &to_frame);

  // Configure the animation, these are optional
  animation_set_duration((Animation*) image_property_animation, TIMER_INTERVAL_MS); // milliseconds
  animation_set_delay((Animation*) image_property_animation, 10); // milliseconds
  animation_set_curve((Animation*) image_property_animation, AnimationCurveEaseInOut);
  // Schedule to occur ASAP with default settings
  animation_schedule((Animation*) image_property_animation);
  app_timer_register(2*TIMER_INTERVAL_MS, trigger_animation, NULL);
  }
  else {
    warpsequence =0;
    //animate back onto the screen
    GRect from_frame = GRect(-144, 58, 0, 32);
    GRect to_frame = GRect(0, 58, 144, 32);

    // Create the animation
    image_property_animation = property_animation_create_layer_frame(bitmap_layer_get_layer(s_enterprise_layer), &from_frame, &to_frame);

    // Configure the animation, these are optional
    animation_set_duration((Animation*) image_property_animation, TIMER_INTERVAL_MS); // milliseconds
    animation_set_delay((Animation*) image_property_animation, 10); // milliseconds
    animation_set_curve((Animation*) image_property_animation, AnimationCurveEaseInOut);
    // Schedule to occur ASAP with default settings
    animation_schedule((Animation*) image_property_animation);
    app_timer_register(TIMER_INTERVAL_MS, update_battery, NULL);
  }
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
  s_time_layer = text_layer_create(GRect(33, 83, bounds.size.w, 80));
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
  icon_weather_layer = text_layer_create(GRect(25, 85, 40, 40));
  // Apply to TextLayer
  text_layer_set_font(icon_weather_layer, s_weather_icon_font);
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(icon_weather_layer, GColorClear);
  text_layer_set_text_color(icon_weather_layer, GColorChromeYellow);
  text_layer_set_text_alignment(icon_weather_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(icon_weather_layer));
  
  s_temperature_layer = text_layer_create(GRect(25, 120, 40, 32));
  text_layer_set_text_color(s_temperature_layer, GColorChromeYellow);
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_font(s_temperature_layer, s_date_font);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));
  
  s_city_layer = text_layer_create(GRect(27, 150, bounds.size.w, 32));
  text_layer_set_text_color(s_city_layer, GColorChromeYellow);
  text_layer_set_background_color(s_city_layer, GColorClear);
  text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_city_layer));

  s_weatherdescript_layer = text_layer_create(GRect(74, 74, bounds.size.w, 32));
  text_layer_set_text_color(s_weatherdescript_layer, GColorChromeYellow);
  text_layer_set_background_color(s_weatherdescript_layer, GColorClear);
  text_layer_set_font(s_weatherdescript_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_weatherdescript_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_weatherdescript_layer));
  
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
  s_battery_layer = layer_create(GRect(4, 72, 34, 5));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  // Create life support meter Layer
  s_lifesupport_layer = layer_create(GRect(68, 143, 73, 14));
  layer_set_update_proc(s_lifesupport_layer, lifesupport_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_lifesupport_layer);

  // Create the TextLayer with specific bounds
  s_lifesupport_text_layer = text_layer_create(GRect(75, 140, 92, 15));
  // Apply to TextLayer
  text_layer_set_font(s_lifesupport_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_lifesupport_text_layer, GColorClear);
  text_layer_set_text_color(s_lifesupport_text_layer, GColorChromeYellow);
  text_layer_set_text_alignment(s_lifesupport_text_layer, GTextAlignmentLeft);
  text_layer_set_text(s_lifesupport_text_layer, "Life Support");
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_lifesupport_text_layer));

  // Create the TextLayer with specific bounds
  s_weekname_layer = text_layer_create(GRect(3,0,21,16));
  // Apply to TextLayer
  text_layer_set_font(s_weekname_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_weekname_layer, GColorClear);
  text_layer_set_text_color(s_weekname_layer, GColorBlack);
  text_layer_set_text_alignment(s_weekname_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weekname_layer));

  // Create the TextLayer with specific bounds
  s_weeknum_layer = text_layer_create(GRect(3,16,21,15));
  // Apply to TextLayer
  text_layer_set_font(s_weeknum_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_weeknum_layer, GColorClear);
  text_layer_set_text_color(s_weeknum_layer, GColorBlack);
  text_layer_set_text_alignment(s_weeknum_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weeknum_layer));

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
  
  // Create warp engine Layer
  s_warp_layer = layer_create(GRect(4, 68, 33, 15));
  layer_set_update_proc(s_warp_layer, warp_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_warp_layer);
}

static void window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_city_layer);
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_stardate_layer);
  text_layer_destroy(s_parsec_layer);
  text_layer_destroy(s_batterypercent_layer);
  text_layer_destroy(s_lifesupport_text_layer);
  text_layer_destroy(icon_weather_layer);
  text_layer_destroy(s_weeknum_layer);
  text_layer_destroy(s_weekname_layer);
  text_layer_destroy(s_weatherdescript_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_weather_icon_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_enterprise_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_enterprise_layer);
  
  //Destroy Layers
  layer_destroy(s_battery_layer);
  layer_destroy(s_lifesupport_layer);
  layer_destroy(s_lcars_layer);
  layer_destroy(s_bt_layer);
  layer_destroy(s_warp_layer);

  //Destroy Window
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

  if (WeekdayNameDisp) {
    static char s_weekdayname[4];
    static char s_weekdayname1[3];
    strftime(s_weekdayname, sizeof(s_weekdayname), "%a", tick_time);
    snprintf(s_weekdayname1,sizeof(s_weekdayname1),"%s",s_weekdayname);
    text_layer_set_text(s_weekname_layer, s_weekdayname1);}
  else {
    text_layer_set_text(s_weekname_layer, " ");}

  if ((strcmp(WeekNumDisp,"none")==0) || (strcmp(WeekNumDisp,"")==0))
      {text_layer_set_text(s_weeknum_layer, " ");}
  else {
    static char s_weeknum[3];
    static char s_weeknum1[4];
    strftime(s_weeknum, sizeof(s_weeknum), WeekNumDisp, tick_time);
    snprintf(s_weeknum1,sizeof(s_weeknum1),"#%s",s_weeknum);
    text_layer_set_text(s_weeknum_layer, s_weeknum1);}

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  prev_s_step_count = s_step_count;
  s_step_count = get_health(HealthMetricStepCount, 0);
  if (prev_s_step_count <=1) {prev_s_step_count = s_step_count;}
  s_step_average = get_health(HealthMetricStepCount, 1);
  s_step_goal = get_health(HealthMetricStepCount, 2);

  if ((prev_s_step_count<UserStepGoal)&&(s_step_count>=UserStepGoal)) {
    warpsequence = 1;
    static const uint32_t const segments[] = { 200, 200, 50, 50, 50, 50, 100, 200, 400 };
    s_battery_level = 0;
    layer_mark_dirty(s_battery_layer);
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pat);
    app_timer_register(4*TIMER_INTERVAL_MS, trigger_animation, NULL);
  }

  //char string to hold the date and step for the stardate value
  static char date_buffer[16];
  static char date_step_buffer[16];
  // Copy date into buffer from tm structure
  if (UKDateFormat)
    {strftime(date_buffer, sizeof(date_buffer), "%d%m%y.", tick_time);}
  else
    {strftime(date_buffer, sizeof(date_buffer), "%m%d%y.", tick_time);}
  //pull the current step count appending to the date buffer
  snprintf(date_step_buffer,sizeof(date_step_buffer),"%s%d",date_buffer,s_step_count);
  // Show the stardate
  text_layer_set_text(s_date_layer, date_step_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  //every 30 minutes do the following
  if(tick_time->tm_min % 30 == 0) {  
    if (strlen(ReplacementWeatherMessage)==0) {generic_weather_fetch(weather_callback);}
    //battery numerical display
    if (PowerDisp) {
      snprintf(battery_text, sizeof(battery_text), "Pwr:%d%%", s_battery_level);}
    else {
      snprintf(battery_text, sizeof(battery_text), " ");}
    text_layer_set_text(s_batterypercent_layer, battery_text); 
}
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
	Tuple *data = dict_find(iterator, MESSAGE_KEY_READY);
	if(data)
	{
    if ((strlen(ReplacementWeatherMessage)==0)&&WeatherSetupStatusKey&&WeatherSetupStatusProvider) 
      {generic_weather_fetch(weather_callback);}
    else
      {text_layer_set_text(s_city_layer,"Please save settings");}
	}
	
	data = dict_find(iterator, MESSAGE_KEY_APIKEY);
	if(data)
	{
		strcpy(api_key, data->value->cstring);
		generic_weather_set_api_key(api_key);
    WeatherSetupStatusKey=S_TRUE;
    //psleep(3000);
    if ((strlen(ReplacementWeatherMessage)==0)&&WeatherSetupStatusProvider) {generic_weather_fetch(weather_callback);}
	}

	data = dict_find(iterator, MESSAGE_KEY_STEPGOAL);
	if(data)
	{
		strcpy(StringUserStepGoal, data->value->cstring);
    UserStepGoal = atoi(StringUserStepGoal);
	}

	data = dict_find(iterator, MESSAGE_KEY_WEATHERREPLACEMENT);
	if(data)
	{
		strcpy(ReplacementWeatherMessage, data->value->cstring);
    text_layer_set_text(s_city_layer,ReplacementWeatherMessage);
    text_layer_set_text(s_temperature_layer,"…");
    text_layer_set_text(s_weatherdescript_layer," ");
    text_layer_set_text(icon_weather_layer,"M");
	}

	data = dict_find(iterator, MESSAGE_KEY_FTICK);
	if(data)
	{
		F_Tick = data->value->int32 == 1;
	}

	data = dict_find(iterator, MESSAGE_KEY_UKDATE);
	if(data)
	{
		UKDateFormat = data->value->int32 == 1;
   	update_time();
	}

	data = dict_find(iterator, MESSAGE_KEY_WEATHERDESCRIPTION);
	if(data)
	{
		WeatherDescriptionDisp = data->value->int32 == 1;
    if ((strlen(ReplacementWeatherMessage)==0)&&WeatherSetupStatusKey&&WeatherSetupStatusProvider) {generic_weather_fetch(weather_callback);}
	}

	data = dict_find(iterator, MESSAGE_KEY_WEEKDAYNAME);
	if(data)
	{
		WeekdayNameDisp = data->value->int32 == 1;
   	update_time();
	}

	data = dict_find(iterator, MESSAGE_KEY_POWERDISPLAY);
	if(data)
	{
		PowerDisp = data->value->int32 == 1;
    //battery numerical display
    if (PowerDisp) {
      snprintf(battery_text, sizeof(battery_text), "Pwr:%d%%", s_battery_level);}
    else {
      snprintf(battery_text, sizeof(battery_text), " ");}
    text_layer_set_text(s_batterypercent_layer, battery_text); 
	}

	data = dict_find(iterator, MESSAGE_KEY_WeatherProvide);
	if(data)
	{
    strcpy(userweatherprovider, data->value->cstring);
    if (strcmp(userweatherprovider,"OpenWe")==0)
      {	generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);
      WeatherSetupStatusProvider=S_TRUE;}
    else if(strcmp(userweatherprovider,"WUnder")==0)
      {generic_weather_set_provider(GenericWeatherProviderWeatherUnderground);
      WeatherSetupStatusProvider=S_TRUE;}
		else if(strcmp(userweatherprovider,"For.io")==0)
      {generic_weather_set_provider(GenericWeatherProviderForecastIo);
      WeatherSetupStatusProvider=S_TRUE;}
    //psleep(3000);
    if ((strlen(ReplacementWeatherMessage)==0)&&WeatherSetupStatusKey) {generic_weather_fetch(weather_callback);}
  }

	data = dict_find(iterator, MESSAGE_KEY_WEEKNUMFORMAT);
	if(data)
	{
    strcpy(WeekNumDisp, data->value->cstring);
  	update_time();
  }

	data = dict_find(iterator, MESSAGE_KEY_HealthMeterSelect);
	if(data)
	{
    strcpy(healthmeterselector, data->value->cstring);
  	update_time();
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

  //initialize battery numerical display
  if (PowerDisp) {
    snprintf(battery_text, sizeof(battery_text), "Pwr:%d%%", s_battery_level);
      text_layer_set_text(s_batterypercent_layer, battery_text);}
  else {
    snprintf(battery_text, sizeof(battery_text), " ");}
  
  if (strlen(ReplacementWeatherMessage)>0) {
    text_layer_set_text(s_city_layer,ReplacementWeatherMessage);
    text_layer_set_text(s_temperature_layer,"…");
  }
  
  // Generic Weather setup and display
  generic_weather_init();
  generic_weather_set_api_key(api_key);
  if (strlen(api_key)>0) {WeatherSetupStatusKey=S_TRUE;}
  if (strcmp(userweatherprovider,"OpenWe")==0)
    {generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);
    WeatherSetupStatusProvider=S_TRUE;}
  else if(strcmp(userweatherprovider,"WUnder")==0)
    {generic_weather_set_provider(GenericWeatherProviderWeatherUnderground);
    WeatherSetupStatusProvider=S_TRUE;}
	else if(strcmp(userweatherprovider,"For.io")==0)
    {generic_weather_set_provider(GenericWeatherProviderForecastIo);
    WeatherSetupStatusProvider=S_TRUE;}
  else
    {APP_LOG(APP_LOG_LEVEL_DEBUG, "UNKNOWN PROVIDER: -%s-", userweatherprovider);
    }
	events_app_message_request_inbox_size(1024);
	events_app_message_register_inbox_received(inbox_received_callback, NULL);
	events_app_message_register_inbox_dropped(inbox_dropped_callback, NULL);
	events_app_message_register_outbox_failed(outbox_failed_callback, NULL);
	events_app_message_register_outbox_sent(outbox_sent_callback, NULL);
  events_app_message_open();
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