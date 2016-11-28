#include <pebble.h>

#include <pebble-generic-weather/pebble-generic-weather.h>
#include <pebble-events/pebble-events.h>

static Window *s_main_window;
static GFont s_time_font, s_date_font, s_weather_icon_font;
static TextLayer *s_date_layer, *s_stardate_layer, *s_batterypercent_layer, *s_lifesupport_text_layer, *s_temperature_layer, *s_city_layer, *icon_weather_layer, *s_time_layer, *s_weekname_layer, *s_weeknum_layer, *s_weatherdescript_layer;
static BitmapLayer *s_enterprise_layer;
static GBitmap *s_enterprise_bitmap;
static char api_key[50];
static char ReplacementWeatherMessage[21], userweatherprovider[7], WeekNumDisp[5], battery_text[] = "Pwr:100%", DateFormatter[]="%m%d%y.";
static int s_step_count = 1, s_step_goal = 1, s_step_average = 1, prev_s_step_count = 1, UserStepGoal = 7500,s_battery_level, s_battery_charging;
static Layer *s_battery_layer, *s_lifesupport_layer, *s_lcars_layer, *s_bt_layer, *s_warp_layer;
static bool F_Tick = S_TRUE, PowerDisp = S_TRUE, UKDateFormat = S_FALSE, WeekdayNameDisp = S_TRUE, WeatherDescriptionDisp = S_FALSE, AnimationEnabled = S_TRUE, DisplaySeconds = S_FALSE, Watchface_Hibernate = S_FALSE;
static int warpsequence = 0, text_color_value =0, UserManualSleepStart=24;
static int loweracrossline = 90, upperacrossline = 50, textleft = 25, watchwidth=168, slept=0;
#define TIMER_INTERVAL_MS 500
#define TIMER_IDLE_INTERVAL_MS 1800000
static PropertyAnimation *image_property_animation; //1800000
static bool WeatherSetupStatusKey = S_FALSE, WeatherSetupStatusProvider = S_FALSE, HibernateEnable = S_TRUE, SleepEnable = S_TRUE;
GColor text_color;
static EventHandle s_health_event_handle, s_tick_timer_event_handle, s_idle_timer_event_handle;

static void read_persist()
{
  if(persist_exists(MESSAGE_KEY_APIKEY)) 	{
		persist_read_string(MESSAGE_KEY_APIKEY, api_key, sizeof(api_key));
	}
	if(persist_exists(MESSAGE_KEY_STEPGOAL)) 	{
		UserStepGoal = persist_read_int(MESSAGE_KEY_STEPGOAL);
	}
	if(persist_exists(MESSAGE_KEY_WEATHERREPLACEMENT)) 	{
		persist_read_string(MESSAGE_KEY_WEATHERREPLACEMENT, ReplacementWeatherMessage, sizeof(ReplacementWeatherMessage));
	}
	if(persist_exists(MESSAGE_KEY_FTICK)) {
		F_Tick = persist_read_bool(MESSAGE_KEY_FTICK);
	}
	if(persist_exists(MESSAGE_KEY_DISPLAYSECONDS)) {
		DisplaySeconds = persist_read_bool(MESSAGE_KEY_DISPLAYSECONDS);
	}
	if(persist_exists(MESSAGE_KEY_UKDATE)) 	{
		UKDateFormat = persist_read_bool(MESSAGE_KEY_UKDATE);
    if (UKDateFormat)
      {snprintf(DateFormatter,sizeof(DateFormatter),"%%d%%m%%y.");}
	}
	if(persist_exists(MESSAGE_KEY_WEATHERDESCRIPTION)) {
		WeatherDescriptionDisp = persist_read_bool(MESSAGE_KEY_WEATHERDESCRIPTION);
	}
	if(persist_exists(MESSAGE_KEY_WEEKDAYNAME))	{
		WeekdayNameDisp = persist_read_bool(MESSAGE_KEY_WEEKDAYNAME);
	}
	if(persist_exists(MESSAGE_KEY_POWERDISPLAY)) {
		PowerDisp = persist_read_bool(MESSAGE_KEY_POWERDISPLAY);
	}
	if(persist_exists(MESSAGE_KEY_ANIMATIONENABLED)) {
		AnimationEnabled = persist_read_bool(MESSAGE_KEY_ANIMATIONENABLED);
	}
	if(persist_exists(MESSAGE_KEY_SLEEPENABLED)) {
		SleepEnable = persist_read_bool(MESSAGE_KEY_SLEEPENABLED);
	}
	if(persist_exists(MESSAGE_KEY_HIBERNATEENABLED)) {
		HibernateEnable = persist_read_bool(MESSAGE_KEY_HIBERNATEENABLED);
	}
	if(persist_exists(MESSAGE_KEY_WeatherProvide)) {
		persist_read_string(MESSAGE_KEY_WeatherProvide, userweatherprovider, sizeof(userweatherprovider));
	}
	if(persist_exists(MESSAGE_KEY_WEEKNUMFORMAT)) {
		persist_read_string(MESSAGE_KEY_WEEKNUMFORMAT, WeekNumDisp, sizeof(WeekNumDisp));
	}
}

static void store_persist()
{
  persist_write_int(MESSAGE_KEY_STEPGOAL, UserStepGoal);
	persist_write_string(MESSAGE_KEY_APIKEY, api_key);
	persist_write_string(MESSAGE_KEY_WEATHERREPLACEMENT, ReplacementWeatherMessage);
  persist_write_bool(MESSAGE_KEY_FTICK, F_Tick);
  persist_write_bool(MESSAGE_KEY_DISPLAYSECONDS, DisplaySeconds);
  persist_write_bool(MESSAGE_KEY_UKDATE, UKDateFormat);
  persist_write_bool(MESSAGE_KEY_WEATHERDESCRIPTION, WeatherDescriptionDisp);
  persist_write_bool(MESSAGE_KEY_WEEKDAYNAME, WeekdayNameDisp);
  persist_write_bool(MESSAGE_KEY_POWERDISPLAY, PowerDisp);
  persist_write_bool(MESSAGE_KEY_ANIMATIONENABLED, AnimationEnabled);
  persist_write_bool(MESSAGE_KEY_SLEEPENABLED, SleepEnable);
  persist_write_bool(MESSAGE_KEY_HIBERNATEENABLED, HibernateEnable);
	persist_write_string(MESSAGE_KEY_WeatherProvide, userweatherprovider);
	persist_write_string(MESSAGE_KEY_WEEKNUMFORMAT, WeekNumDisp);
  if (text_color_value >0) { persist_write_int(MESSAGE_KEY_TEXTCOLOR, text_color_value); }
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
			APP_LOG(APP_LOG_LEVEL_ERROR, "Health data unavailable!");
		}
	}
	return output;
}

static void lifesupport_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar which is up to 57 pixels wide
  int goalwidth = ((s_step_count * bounds.size.w) / s_step_goal);
  if (goalwidth > bounds.size.w){goalwidth = bounds.size.w;}
  int avgwidth = ((s_step_count * bounds.size.w) / s_step_average);
  if (avgwidth > bounds.size.w){avgwidth = bounds.size.w;}

  // Draw the background
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_rect(ctx,bounds);
  //  graphics_context_set_fill_color(ctx, GColorWhite);
  //  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_fill_color(ctx, GColorBlueMoon);
  graphics_fill_rect(ctx, GRect(0, 0, goalwidth, bounds.size.h), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorMidnightGreen);
  graphics_fill_rect(ctx, GRect(0, 0, avgwidth, bounds.size.h/2), 0, GCornerNone);
}

static void lcars_block(Layer *layer, GContext *ctx) {
  // LCARS COLORS GColorVividViolet GColorIcterine GColorLavenderIndigo GColorPictonBlue GColorRajah GColorBlue GColorChromeYellow GColorSunsetOrange
  //              dark purple       light yellow    light purple        light blue        tan
  GRect bounds = layer_get_bounds(layer);
  int spacing = (bounds.size.w/60);
  int basewidth = ((bounds.size.w/4)+2);
  int blockwidth = (bounds.size.w/6);
  int thinblockheight = (bounds.size.h/38);
  int blockheight = ((bounds.size.h/10)-1);
  loweracrossline = ((bounds.size.h/2)+6);
  upperacrossline = ((bounds.size.h/3)-6);
  watchwidth = bounds.size.w;
  bool connected = connection_service_peek_pebble_app_connection();
  //lower elbo
  graphics_context_set_fill_color(ctx, GColorLavenderIndigo);
  graphics_fill_rect(ctx, GRect(3,loweracrossline,(bounds.size.w/4)-3,blockheight), 15, GCornerTopLeft);//3,90,33,15,15R -> 3,120,47,21,
  //graphics_fill_rect(ctx, GRect(3,90,33,15), 15, GCornerTopLeft);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(blockwidth,thinblockheight+loweracrossline,17,15), 10, GCornerTopLeft); //24,94,12,11,10R ->33,126,17,15
  //graphics_fill_rect(ctx, GRect(24,94,12,11), 10, GCornerTopLeft);
  //lower elbo +1
  graphics_context_set_fill_color(ctx, GColorIcterine);
  graphics_fill_rect(ctx, GRect(basewidth,loweracrossline,blockwidth,thinblockheight), 0, GCornerNone);
  //graphics_fill_rect(ctx, GRect(38,90,21,4), 0, GCornerNone); //38,90,24,4>52,120,33,6
  //lower elbo +2
  graphics_context_set_fill_color(ctx, GColorVividViolet);
  graphics_fill_rect(ctx, GRect(basewidth+spacing+blockwidth,loweracrossline,blockwidth,thinblockheight), 0, GCornerNone);
  //graphics_fill_rect(ctx, GRect(62,90,21,4), 0, GCornerNone); //64,90,24,4->88,120,33,6
  //lower elbo +3
  graphics_context_set_fill_color(ctx, GColorBlue);
  graphics_fill_rect(ctx, GRect(basewidth+2*(spacing+blockwidth),loweracrossline,blockwidth,thinblockheight), 0, GCornerNone);
  //graphics_fill_rect(ctx, GRect(86,90,21,4), 0, GCornerNone); //90,90,24,4->124,120,33,6
  //lower elbo +4
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, GRect(basewidth+3*(spacing+blockwidth),loweracrossline,blockwidth+7,thinblockheight), 0, GCornerNone);
  //graphics_fill_rect(ctx, GRect(110,90,34,4), 0, GCornerNone);  //116,90,28,4 ->160,120,40,6  //accounts for the display having a slightly longer last block
  //lower elbo -1
  graphics_context_set_fill_color(ctx, GColorChromeYellow);
  graphics_fill_rect(ctx, GRect(3,loweracrossline+blockheight+spacing,blockwidth-3,blockheight), 0, GCornerNone); 
//  graphics_fill_rect(ctx, GRect(3,107,21,15), 0, GCornerNone); //->3,144,30,21
  //lower elbo -2
  graphics_fill_rect(ctx, GRect(3,loweracrossline+2*(blockheight+spacing),blockwidth-3,blockheight), 0, GCornerNone);
//  graphics_fill_rect(ctx, GRect(3,124,21,15), 0, GCornerNone); //->3,168,30,21
  //lower elbo -3
  if (connected) {
    graphics_context_set_fill_color(ctx, GColorRajah);}
  else {
    graphics_context_set_fill_color(ctx, GColorRed);}
  graphics_fill_rect(ctx, GRect(3,loweracrossline+3*(blockheight+spacing),blockwidth-3,36), 0, GCornerNone);
//  graphics_fill_rect(ctx, GRect(3,141,21,27), 0, GCornerNone); //->3,192,30,36 36 because the last block is higher
  //upper elbo
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, GRect(3,upperacrossline-blockheight,blockwidth*3,blockheight+blockheight/2), 15, GCornerBottomLeft);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(blockwidth,upperacrossline - blockheight,blockwidth*2+3,blockheight+blockheight/2-thinblockheight), 10, GCornerBottomLeft);
  //upper elbo +1
  graphics_context_set_fill_color(ctx, GColorChromeYellow);
  graphics_fill_rect(ctx, GRect(blockwidth*3+3+spacing,upperacrossline+blockheight/2-thinblockheight,blockwidth,thinblockheight), 0, GCornerNone);
//  graphics_fill_rect(ctx, GRect(77,53,21,4), 0, GCornerNone);
  //upper elbo +2
  graphics_context_set_fill_color(ctx, GColorSunsetOrange);
  graphics_fill_rect(ctx, GRect(blockwidth*4+3+spacing*2,upperacrossline+blockheight/2-thinblockheight,blockwidth*2,thinblockheight), 0, GCornerNone);
//  graphics_fill_rect(ctx, GRect(110,53,44,4), 0, GCornerNone);
  //upper elbo -1
  graphics_context_set_fill_color(ctx, GColorLavenderIndigo);
  graphics_fill_rect(ctx, GRect(3,upperacrossline-2*blockheight-spacing,blockwidth-3,blockheight), 0, GCornerNone);
//  graphics_fill_rect(ctx, GRect(3,18,21,15), 0, GCornerNone);
  //upper elbo -2
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_rect(ctx, GRect(3,0,blockwidth-3,upperacrossline-2*(blockheight+spacing)), 0, GCornerNone);
//  graphics_fill_rect(ctx, GRect(3,0,21,16), 0, GCornerNone);
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
  if (warpsequence==0) {
    // Record the new battery level
    s_battery_level = state.charge_percent;
    s_battery_charging = state.is_charging;
    // Update meter
    layer_mark_dirty(s_battery_layer);
  }
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = ((s_battery_level * bounds.size.w) / 100);

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
  GRect to_frame = GRect(watchwidth, upperacrossline+8, watchwidth, 32);

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
    GRect from_frame = GRect(-watchwidth, upperacrossline+8, 0, 32);
    GRect to_frame = GRect(0, upperacrossline+8, watchwidth, 32);

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
  loweracrossline = ((bounds.size.h/2)+6);
  upperacrossline = ((bounds.size.h/3)-6);
  textleft = ((bounds.size.w*5)/28);

  // Create lcars background Layer
  s_lcars_layer = layer_create(bounds);
  layer_set_update_proc(s_lcars_layer, lcars_block);
  // Add to Window
  layer_add_child(window_layer, s_lcars_layer);

  s_bt_layer = layer_create(GRect((bounds.size.w/14)-7,bounds.size.h-(bounds.size.h/12+10),21,23));
//  s_bt_layer = layer_create(GRect(3,144,21,23));
  layer_set_update_proc(s_bt_layer, bt_update_proc);
  layer_add_child(window_layer, s_bt_layer);
  
  // Create GFonts
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LCARS_32));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_LCARS_58));
  s_weather_icon_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WEATHERICON_38));
  
    // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(GRect(0, loweracrossline-7, bounds.size.w, 80));
  //s_time_layer = text_layer_create(GRect(33, 83, bounds.size.w, 80));
  // Create GFont
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, text_color);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create BitmapLayer to display the GBitmap 
  s_enterprise_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ENTERPRISE);
  s_enterprise_layer = bitmap_layer_create(GRect(0, (upperacrossline+loweracrossline)/2-12, bounds.size.w, 32));
  //s_enterprise_layer = bitmap_layer_create(GRect(0, 58, 144, 32));
  bitmap_layer_set_compositing_mode(s_enterprise_layer, GCompOpSet);
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_enterprise_layer, s_enterprise_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_enterprise_layer));

  // Create the TextLayer with specific bounds
  s_date_layer = text_layer_create(GRect(textleft, 15, bounds.size.w-textleft, 40));
  //s_date_layer = text_layer_create(GRect(26, 15, bounds.size.w, 40));
  // Apply to TextLayer
  text_layer_set_font(s_date_layer, s_date_font);
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, text_color);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Create the TextLayer with specific bounds
  icon_weather_layer = text_layer_create(GRect(textleft, loweracrossline-5, 40, 40));
  //icon_weather_layer = text_layer_create(GRect(25, 85, 40, 40));
  // Apply to TextLayer
  text_layer_set_font(icon_weather_layer, s_weather_icon_font);
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(icon_weather_layer, GColorClear);
  text_layer_set_text_color(icon_weather_layer, text_color);
  text_layer_set_text_alignment(icon_weather_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(icon_weather_layer));
  
  s_temperature_layer = text_layer_create(GRect(textleft, (bounds.size.h/6+2)+loweracrossline, 40, 32));
  //s_temperature_layer = text_layer_create(GRect(25, 120, 40, 32));
  text_layer_set_text_color(s_temperature_layer, text_color);
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_font(s_temperature_layer, s_date_font);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  text_layer_set_text(s_temperature_layer,"…"); //this is the voyager com badge icon for the custom font
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));
  
  s_city_layer = text_layer_create(GRect(textleft, 2*(bounds.size.h/6+2)+loweracrossline, bounds.size.w-textleft, 32));
  //s_city_layer = text_layer_create(GRect(27, 150, bounds.size.w, 32));
  text_layer_set_text_color(s_city_layer, text_color);
  text_layer_set_background_color(s_city_layer, GColorClear);
  if (bounds.size.w==144) {
    text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  } else {
    text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  }
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentLeft);
  text_layer_set_text(s_city_layer,"LCARS1701D");
  layer_add_child(window_layer, text_layer_get_layer(s_city_layer));

  s_weatherdescript_layer = text_layer_create(GRect(bounds.size.w/2+2, (upperacrossline+loweracrossline)/2+3, bounds.size.w/2-2, 32));
  //s_weatherdescript_layer = text_layer_create(GRect(74, 74, bounds.size.w, 32));
  text_layer_set_text_color(s_weatherdescript_layer, text_color);
  text_layer_set_background_color(s_weatherdescript_layer, GColorClear);
  if (bounds.size.w==144) {
    text_layer_set_font(s_weatherdescript_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  } else {
    text_layer_set_font(s_weatherdescript_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  }
  text_layer_set_text_alignment(s_weatherdescript_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_weatherdescript_layer));
  
  // Create the TextLayer with specific bounds
  s_batterypercent_layer = text_layer_create(GRect(((bounds.size.w*13)/28)-64, upperacrossline+(4*bounds.size.h/100), bounds.size.w/2, 15));
  //s_batterypercent_layer = text_layer_create(GRect(2, 54, bounds.size.w, 15));
  // Apply to TextLayer
  text_layer_set_font(s_batterypercent_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_batterypercent_layer, GColorClear);
  text_layer_set_text_color(s_batterypercent_layer, text_color);
  text_layer_set_text_alignment(s_batterypercent_layer, GTextAlignmentLeft);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_batterypercent_layer));

  // Create battery meter Layer
  s_battery_layer = layer_create(GRect((bounds.size.w/2)-68, ((bounds.size.h*25)/60)+2, 34, 5));
  //s_battery_layer = layer_create(GRect(4, 72, 34, 5));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  // Create life support meter Layer
  s_lifesupport_layer = layer_create(GRect(bounds.size.w-76, bounds.size.h/2+59, 75, ((bounds.size.h*7)/30)-25));
  //s_lifesupport_layer = layer_create(GRect(68, 143, 73, 14));
  layer_set_update_proc(s_lifesupport_layer, lifesupport_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_lifesupport_layer);

  // Create the TextLayer with specific bounds
  s_lifesupport_text_layer = text_layer_create(GRect(bounds.size.w-76, ((bounds.size.h*7)/12)+42, 75, 19));
  //s_lifesupport_text_layer = text_layer_create(GRect(75, 140, 92, 15));
  // Apply to TextLayer
  if (bounds.size.w==144) {
    text_layer_set_font(s_lifesupport_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  } else {
    text_layer_set_font(s_lifesupport_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  }
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_lifesupport_text_layer, GColorClear);
  text_layer_set_text_color(s_lifesupport_text_layer, text_color);
  text_layer_set_text_alignment(s_lifesupport_text_layer, GTextAlignmentCenter);
  text_layer_set_text(s_lifesupport_text_layer, "Life Support");
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_lifesupport_text_layer));

  // Create the TextLayer with specific bounds
  s_weekname_layer = text_layer_create(GRect(3,0,bounds.size.w/6-3,19));
  //s_weekname_layer = text_layer_create(GRect(3,0,21,16));
  // Apply to TextLayer
  if (bounds.size.w==144) {
    text_layer_set_font(s_weekname_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  } else {
    text_layer_set_font(s_weekname_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  }
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_weekname_layer, GColorClear);
  text_layer_set_text_color(s_weekname_layer, GColorBlack);
  text_layer_set_text_alignment(s_weekname_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weekname_layer));

  // Create the TextLayer with specific bounds
  s_weeknum_layer = text_layer_create(GRect(3,((bounds.size.h*7)/60)-3,bounds.size.w/6-3,19));
  //s_weeknum_layer = text_layer_create(GRect(3,16,21,15));
  // Apply to TextLayer
  if (bounds.size.w==144) {
    text_layer_set_font(s_weeknum_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  } else {
    text_layer_set_font(s_weeknum_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  }
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_weeknum_layer, GColorClear);
  text_layer_set_text_color(s_weeknum_layer, GColorBlack);
  text_layer_set_text_alignment(s_weeknum_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weeknum_layer));

  // Create the TextLayer with specific bounds
  s_stardate_layer = text_layer_create(GRect(textleft, 00, bounds.size.w-textleft, 19));
  //s_stardate_layer = text_layer_create(GRect(textleft, 00, bounds.size.w, 15));
  // Apply to TextLayer
  if (bounds.size.w==144) {
    text_layer_set_font(s_stardate_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  } else {
    text_layer_set_font(s_stardate_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  }
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_stardate_layer, GColorClear);
  text_layer_set_text_color(s_stardate_layer, text_color);
  text_layer_set_text_alignment(s_stardate_layer, GTextAlignmentLeft);
  // Add it as a child layer to the Window's root layer
  text_layer_set_text(s_stardate_layer, "Stardate     Parsecs");
  layer_add_child(window_layer, text_layer_get_layer(s_stardate_layer));
  
  // Create warp engine Layer
  s_warp_layer = layer_create(GRect((bounds.size.w/2)-68, ((bounds.size.h*25)/60)-2, 33, 15));
  //s_warp_layer = layer_create(GRect(4, 68, 33, 15));
  layer_set_update_proc(s_warp_layer, warp_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_warp_layer);
  
//  text_layer_set_text(icon_weather_layer, "D");
//  text_layer_set_text(s_temperature_layer,"68");
//  text_layer_set_text(s_weatherdescript_layer,"Partly Cloudy");
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H%M" : "%I%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
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

  //char string to hold the date and step for the stardate value
  static char date_buffer[16];
  static char date_step_buffer[16];
  // Copy date into buffer from tm structure
  strftime(date_buffer, sizeof(date_buffer), DateFormatter, tick_time);
  if (DisplaySeconds) {
    static char s_seconds_buffer[3];
    strftime(s_seconds_buffer,sizeof(s_seconds_buffer),"%S",tick_time);
    snprintf(date_step_buffer,sizeof(date_step_buffer),"%s%s",date_buffer,s_seconds_buffer);
  } else {
    //pull the current step count appending to the date buffer
    snprintf(date_step_buffer,sizeof(date_step_buffer),"%s%d",date_buffer,s_step_count);
  }
  // Show the stardate
  text_layer_set_text(s_date_layer, date_step_buffer);
}

static void update_steps(){
  prev_s_step_count = s_step_count;
  s_step_count = get_health(HealthMetricStepCount, 0);
  if (prev_s_step_count <2) {prev_s_step_count = s_step_count;}
  s_step_average = get_health(HealthMetricStepCount, 1);
  s_step_goal = get_health(HealthMetricStepCount, 2);
  if ((s_step_goal==0) ||(s_step_average==0)) {//this comes up when the watch has less than a week of health data
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    s_step_goal = UserStepGoal;  //just use the user's step goal
    s_step_average=(((tick_time->tm_hour+1)*s_step_goal)/24);  //calculate average by the time of day
  }

//  UserStepGoal = 1120;
  if ((prev_s_step_count<UserStepGoal)&&(s_step_count>=UserStepGoal)) {
    static const uint32_t const segments[] = { 200, 200, 50, 50, 50, 50, 100, 200, 400 };
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pat);
    if (AnimationEnabled) {
      warpsequence = 1;
      s_battery_level = 0;
      layer_mark_dirty(s_battery_layer);
      app_timer_register(4*TIMER_INTERVAL_MS, trigger_animation, NULL);
    }
  }
}

static void watchface_refresh(){
  update_steps();
  update_time();
  if (strlen(ReplacementWeatherMessage)==0) {
    generic_weather_fetch(weather_callback);
  } else {
    text_layer_set_text(s_weatherdescript_layer, " ");
    text_layer_set_text(s_city_layer,ReplacementWeatherMessage);
    text_layer_set_text(s_temperature_layer,"…");  //place the star trek comm badge
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (tick_time->tm_sec==0) { update_steps(); }
  update_time();
  
  //every 30 minutes do the following
  if ((tick_time->tm_min % 30 == 0)&&(tick_time->tm_sec==0)) {  
    if (strlen(ReplacementWeatherMessage)==0) {generic_weather_fetch(weather_callback);}
    //battery numerical display
    if (PowerDisp) {
      snprintf(battery_text, sizeof(battery_text), "Pwr:%d%%", s_battery_level);
      text_layer_set_text(s_batterypercent_layer, battery_text); 
    }
  }
  
/*  if ((tick_time->tm_hour>21)||(tick_time->tm_hour<7)) {slept++;}
  static char s_buffer[8];
  snprintf(s_buffer,sizeof(s_buffer),"%d",slept);
  text_layer_set_text(s_weatherdescript_layer, s_buffer);*/
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // A tap event occured
  Watchface_Hibernate=S_FALSE;
  // Unsubscribe from tap events
  accel_tap_service_unsubscribe();
  watchface_refresh();
  if (s_tick_timer_event_handle==NULL) {
    if (DisplaySeconds) {
      s_tick_timer_event_handle = events_tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    } else {
      s_tick_timer_event_handle = events_tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    }
  }
  if (HibernateEnable) {app_timer_reschedule(s_idle_timer_event_handle,TIMER_IDLE_INTERVAL_MS);}
}

bool is_user_sleeping() {
  static bool is_sleeping=S_FALSE;
  if (SleepEnable) {
    HealthActivityMask activities = health_service_peek_current_activities();
    is_sleeping = activities & HealthActivitySleep || activities & HealthActivityRestfulSleep;
  }
  return is_sleeping;
/*  if (s_tick_timer_event_handle!=NULL) {
    return S_TRUE;
  } else {
    return S_FALSE;
  }*/
}

static void prv_health_event_handler(HealthEventType event, void *context) {
    switch(event) {
    case HealthEventSignificantUpdate: 
      if (!Watchface_Hibernate) {
        if (HibernateEnable) {app_timer_reschedule(s_idle_timer_event_handle,TIMER_IDLE_INTERVAL_MS);}
        prv_health_event_handler(HealthEventSleepUpdate, context);
      }
      break;
    case HealthEventSleepUpdate: {
        bool sleeping = is_user_sleeping();
        if (sleeping && ((s_tick_timer_event_handle!=NULL)||Watchface_Hibernate)) {
          if (HibernateEnable) {app_timer_reschedule(s_idle_timer_event_handle,TIMER_IDLE_INTERVAL_MS*16);}
          text_layer_set_text(s_city_layer,"Watch is Sleeping");
          text_layer_set_text(s_time_layer, "8888");
          if (s_tick_timer_event_handle!=NULL) {
            events_tick_timer_service_unsubscribe(s_tick_timer_event_handle);
            s_tick_timer_event_handle=NULL;
          }
          Watchface_Hibernate = S_FALSE;  //if the user falls asleep while in hibernation then reset to wake when the user wakes
/*          static char s_buffer[8];
          time_t temp = time(NULL);
          struct tm *tick_time = localtime(&temp);
          strftime(s_buffer, sizeof(s_buffer), "%H%M", tick_time);
          text_layer_set_text(s_weatherdescript_layer, s_buffer);
*/          accel_tap_service_subscribe(accel_tap_handler);
        } else if (!sleeping && (s_tick_timer_event_handle==NULL) && !Watchface_Hibernate) {
          if (s_tick_timer_event_handle==NULL) {
            if (DisplaySeconds) {
              s_tick_timer_event_handle = events_tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
            } else {
              s_tick_timer_event_handle = events_tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
            }
            accel_tap_service_unsubscribe();  //do this here because having a Null tick_timer coorelates with subscribing to tap service
          }
          Watchface_Hibernate = S_FALSE;
          watchface_refresh();
          if (HibernateEnable) {app_timer_reschedule(s_idle_timer_event_handle,TIMER_IDLE_INTERVAL_MS);}
        }
      }
      break;
    case HealthEventMovementUpdate:
      if (HibernateEnable) {app_timer_reschedule(s_idle_timer_event_handle, TIMER_IDLE_INTERVAL_MS);}
      break;
    case HealthEventHeartRateUpdate:
//      app_timer_reschedule(s_idle_timer_event_handle, TIMER_IDLE_INTERVAL_MS);
      break;
    case HealthEventMetricAlert:
      break;
    }
}

static void idle_scheduler(){
  if (HibernateEnable) {
    text_layer_set_text(s_city_layer,"Watch is Idle");
    text_layer_set_text(s_time_layer, "8888");
    if (s_tick_timer_event_handle!=NULL){
      events_tick_timer_service_unsubscribe(s_tick_timer_event_handle);
      s_tick_timer_event_handle=NULL;
    }
    Watchface_Hibernate = S_TRUE;  //signifies that the watchface went idle, used so that a healthevent does not wake up the watch
    accel_tap_service_subscribe(accel_tap_handler);
    //prv_health_event_handler(HealthEventSleepUpdate, NULL);
    s_idle_timer_event_handle = app_timer_register(TIMER_IDLE_INTERVAL_MS*16,idle_scheduler,NULL);  //the only time the idle handeler is used is when this function is called, so create a new one that can be rescheduled by tap handeler or something else.
  } 
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  Tuple *data = dict_find(iterator, MESSAGE_KEY_READY);
	if(data)
	{
  	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Ready Recieved");
    //If we just recieved a ready then we are launching the watchface.

    if (strlen(ReplacementWeatherMessage)==0) {
      if (WeatherSetupStatusKey&&WeatherSetupStatusProvider) {
        generic_weather_fetch(weather_callback);
      } else {
        text_layer_set_text(s_city_layer,"Please save settings");
      }
    } else {
      text_layer_set_text(s_city_layer,ReplacementWeatherMessage);
      text_layer_set_text(s_temperature_layer,"…");  //place the star trek comm badge
    }
	}
	
	data = dict_find(iterator, MESSAGE_KEY_APIKEY);
	if(data)
	{
		strcpy(api_key, data->value->cstring);
    if (strlen(api_key)>0) {
      generic_weather_set_api_key(api_key);
      WeatherSetupStatusKey=S_TRUE;
    } else {
      WeatherSetupStatusKey=S_FALSE;
    }
	}

	data = dict_find(iterator, MESSAGE_KEY_STEPGOAL);
	if(data)
	{
		UserStepGoal = data->value->int32;
	}

	data = dict_find(iterator, MESSAGE_KEY_WEATHERREPLACEMENT);
	if(data)
	{
		strcpy(ReplacementWeatherMessage, data->value->cstring);
  }

	data = dict_find(iterator, MESSAGE_KEY_FTICK);
	if(data)
	{
		F_Tick = data->value->int32 == 1;
	}

	data = dict_find(iterator, MESSAGE_KEY_DISPLAYSECONDS);
	if(data)
	{
		DisplaySeconds = data->value->int32 == 1;
    if (s_tick_timer_event_handle!=NULL) {events_tick_timer_service_unsubscribe(s_tick_timer_event_handle);}
    if (DisplaySeconds) {
      s_tick_timer_event_handle = events_tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    } else {
      s_tick_timer_event_handle = events_tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    }
  }

	data = dict_find(iterator, MESSAGE_KEY_UKDATE);
	if(data)
	{
		UKDateFormat = data->value->int32 == 1;
    if (UKDateFormat) {
      snprintf(DateFormatter,sizeof(DateFormatter),"%%d%%m%%y.");
    } else {
      snprintf(DateFormatter,sizeof(DateFormatter),"%%m%%d%%y.");
    }
  }

	data = dict_find(iterator, MESSAGE_KEY_ANIMATIONENABLED);
	if(data)
	{
		AnimationEnabled = data->value->int32 == 1;
	}

	data = dict_find(iterator, MESSAGE_KEY_SLEEPENABLED);
	if(data)
	{
		SleepEnable = data->value->int32 == 1;
	}

	data = dict_find(iterator, MESSAGE_KEY_HIBERNATEENABLED);
	if(data)
	{
		HibernateEnable = data->value->int32 == 1;

    events_health_service_events_unsubscribe(s_health_event_handle);
    //register to recieve significant updates of which we are looking for sleep updates
    if (SleepEnable||HibernateEnable) {s_health_event_handle = events_health_service_events_subscribe(prv_health_event_handler, NULL);}
  
    //register to detect when the watch goes idle
    if (HibernateEnable) {
      s_idle_timer_event_handle = app_timer_register(TIMER_IDLE_INTERVAL_MS,idle_scheduler,NULL);
    } else {
      app_timer_cancel(s_idle_timer_event_handle);
    }
	}

	data = dict_find(iterator, MESSAGE_KEY_WEATHERDESCRIPTION);
	if(data)
	{
		WeatherDescriptionDisp = data->value->int32 == 1;
	}

	data = dict_find(iterator, MESSAGE_KEY_WEEKDAYNAME);
	if(data)
	{
		WeekdayNameDisp = data->value->int32 == 1;
	}
  
	data = dict_find(iterator, MESSAGE_KEY_TEXTCOLOR);
  if(data) {
    if (data->value->int32!=text_color_value) {
      text_color_value = data->value->int32;
      text_color = GColorFromHEX(text_color_value);
      text_layer_set_text_color(s_time_layer, text_color);
      text_layer_set_text_color(s_date_layer, text_color);
      text_layer_set_text_color(icon_weather_layer, text_color);
      text_layer_set_text_color(s_temperature_layer, text_color);
      text_layer_set_text_color(s_city_layer, text_color);
      text_layer_set_text_color(s_weatherdescript_layer, text_color);
      text_layer_set_text_color(s_batterypercent_layer, text_color);
      text_layer_set_text_color(s_lifesupport_text_layer, text_color);
      text_layer_set_text_color(s_stardate_layer, text_color);
    }
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
    else
      {WeatherSetupStatusProvider = S_FALSE;}
  }

	data = dict_find(iterator, MESSAGE_KEY_WEEKNUMFORMAT);
	if(data)
	{  //all setting are set each time the user saves settings.  Since this is the last one refresh all displays so their new settings.
    strcpy(WeekNumDisp, data->value->cstring);
    update_steps();
//    APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbox Updating time");
    update_time();
    if (strlen(ReplacementWeatherMessage)==0) {
      if (WeatherSetupStatusKey&&WeatherSetupStatusProvider) {
//        APP_LOG(APP_LOG_LEVEL_DEBUG, "Calling Weather");
        generic_weather_fetch(weather_callback);
      }
      else {
        text_layer_set_text(s_city_layer,"Please save settings");
      }
    }
    else {
      text_layer_set_text(s_city_layer,ReplacementWeatherMessage);
      text_layer_set_text(s_temperature_layer,"…"); //this is the voyager com badge icon for the custom font
      text_layer_set_text(s_weatherdescript_layer," ");
      text_layer_set_text(icon_weather_layer,"M");  //blank character
    }
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

static void window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_city_layer);
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_stardate_layer);
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

  //unsubscribe from services
  events_battery_state_service_unsubscribe(battery_callback);
  events_connection_service_unsubscribe(bluetooth_callback);
  events_tick_timer_service_unsubscribe(tick_handler);
  events_health_service_events_unsubscribe(s_health_event_handle);
  events_app_message_unsubscribe(inbox_received_callback);
  events_app_message_unsubscribe(inbox_dropped_callback);
  events_app_message_unsubscribe(outbox_failed_callback);
  events_app_message_unsubscribe(outbox_sent_callback);
  app_timer_cancel(s_idle_timer_event_handle);

  //Destroy Window
  window_destroy(window);
}

static void init() {
	//I shouldn't be doing this here and it should be in read_persist, but the function takes a while to complete and I need this setting for window_load
  text_color = persist_exists(MESSAGE_KEY_TEXTCOLOR)? GColorFromHEX(persist_read_int(MESSAGE_KEY_TEXTCOLOR)) : GColorChromeYellow;

  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);
	
//read in stored settings from the pebble VM memory
  read_persist();

  // Register with TickTimerService
  if (DisplaySeconds) {
    s_tick_timer_event_handle = events_tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  } else {
    s_tick_timer_event_handle = events_tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }
  
  //register to recieve significant updates of which we are looking for sleep updates
  if (SleepEnable||HibernateEnable) {s_health_event_handle = events_health_service_events_subscribe(prv_health_event_handler, NULL);}
  
  //register to detect when the watch goes idle
  if (HibernateEnable) {s_idle_timer_event_handle = app_timer_register(TIMER_IDLE_INTERVAL_MS,idle_scheduler,NULL);}
  
  // Make sure the time and steps are displayed from the start
  update_steps();
  update_time();

  // Register for battery level updates
  events_battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
    
  //initialize battery numerical display
  if (PowerDisp) {
    snprintf(battery_text, sizeof(battery_text), "Pwr:%d%%", s_battery_level);
    text_layer_set_text(s_batterypercent_layer, battery_text);}
  	
  // Register for Bluetooth connection updates
  events_connection_service_subscribe((ConnectionHandlers) {.pebble_app_connection_handler = bluetooth_callback});

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