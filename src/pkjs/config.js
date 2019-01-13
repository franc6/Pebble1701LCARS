module.exports = [
  {
    "type": "heading",
    "defaultValue": "NCC-1701 LCARS TOM Settings"
  },
    {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Watchface Settings"
      },
      {
      "type": "color",
      "messageKey": "TEXTCOLOR",
      "defaultValue": "FFAA00",
      "label": "Press the color dot to the right to select a color used to display text and the weather icon",
      "sunlight": true,
      },
      {
        "type": "toggle",
        "messageKey": "POWERDISPLAY",
        "label": "Show percentage of battery remaining above the engines?",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "UKDATE",
        "label": "Display the stardate as ddmmyy for UK instead of mmddyy?",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "DISPLAYSECONDS",
        "label": "Display seconds instead of the number of steps?",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "WEEKDAYNAME",
        "label": "Display the abbreviated weekday name at the top left?",
        "defaultValue": true
      },
      {
        "type": "radiogroup",
        "messageKey": "WEEKNUMFORMAT",
        "label": "You can also display the week number in one of the following formats.  The week number will display at the top left prepended by a #",
        "defaultValue": "none",
        "options": [
          { 
          "label": "Do not display the week number", 
          "value": "none"
          },
          { 
          "label": "Week number using the first Sunday as the first week", 
          "value": "%U"
          },
          { 
          "label": "ISO-8601 week number where the first week of the year has at least 4 weekdays.  Monday is the start of the week", 
          "value": "%V"
          },
          { 
          "label": "Week number using the first Monday as the first week", 
          "value": "%W"
          }]
      },
      {
        "type": "toggle",
        "messageKey": "DISPLAYDOY",
        "label": "Display day of year?",
        "defaultValue": false
      },
      {
        "type": "slider",
        "messageKey": "STEPGOAL",
        "defaultValue": 7500,
        "label": "Daily Step Goal",
        "description": "Set your daily step goal and your watch will buzz when you achieve that goal.  You will recieve a long-short-short-long vibration pattern when you meet this goal",
        "min": 5000,
        "max": 20000,
        "step": 500
      },
      {
        "type": "toggle",
        "messageKey": "ANIMATIONENABLED",
        "label": "Enable a custom animation when achieve your step goal?  If enabled the vibration pattern be the notification to glance down the the animation, disabling will save some power.",
        "defaultValue": true
      }
    ]
    },{
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Power Saving Options"
      },
      {
      "type": "text",
      "defaultValue": "These options will save your Pebble's battery life by stopping the display and prevent the weather from refreshing in these two circumstances."
      },
      {
        "type": "toggle",
        "messageKey": "SLEEPENABLED",
        "label": "If enabled, the watch will show 'Watch is sleeping' and set the time to ... when you are sleeping.  Flick your wrist to wake it up early",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "HIBERNATEENABLED",
        "label": "If enabled, the watch will show 'Watch is idle' and set time time to ... when you don't move for 45 minutes.  Flick your wrist to wake it  up or it will wake up automatically when you start moving",
        "defaultValue": true
      }
    ]
      },{
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Watchface Weather Settings"
      },
      {
        "type": "input",
        "messageKey": "WEATHERREPLACEMENT",
        "defaultValue": "",
        "limit":20,
        "label": "If you would like to disable weather, enter a short 20 character message to display instead.  Note: depending on the characters used, you may not have room to display all 20 characters.  When this field is blank, we will poll weather."
      },
      {
        "type": "radiogroup",
        "messageKey": "WeatherProvide",
        "label": "Please select which weather provider you would like to use",
        "defaultValue": "YahooW",
        "options": [
          { 
          "label": "Yahoo Weather", 
          "value": "YahooW" 
          },
          { 
          "label": "OpenWeatherMap", 
          "value": "OpenWe" 
          },
          { 
          "label": "WeatherUnderGround", 
          "value": "WUnder" 
          },
          { 
          "label": "Dark Sky/Forecast.io", 
          "value": "For.io" 
          }]
      },
      {
        "type": "input",
        "messageKey": "APIKEY",
        "defaultValue": "",
        "label": "Please sign up for an API Key from your selected weather provider and enter it here"
      },
      {
        "type": "toggle",
        "messageKey": "FTICK",
        "label": "On for F, off for C",
        "defaultValue": true
      },
      {
        "type": "toggle",
        "messageKey": "WEATHERDESCRIPTION",
        "label": "Display the description of the weather in addition to the weather icon",
        "defaultValue": false
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
