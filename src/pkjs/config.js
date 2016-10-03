module.exports = [
  {
    "type": "heading",
    "defaultValue": "NCC-1701 LCARS Settings"
  },
  {
    "type": "text",
    "defaultValue": "I will be adding more settings, for now enjoy weather on your watchface."
  },
    {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Watchface Settings"
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
        "type": "input",
        "messageKey": "STEPGOAL",
        "defaultValue": "7500",
        "label": "Set your daily step goal and your watch will buzz and display an animation when you achieve that goal.  Set to 0 to disable both functions."
      }
    ]
    },
  {
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
        "label": "If you would like to disable weather, enter a short 20 character message to display instead.  Note: depending on the characters used, you may not have room to display all 20 characters.  When this field is blank, we will poll weather."
      },
      {
        "type": "radiogroup",
        "messageKey": "WeatherProvide",
        "label": "Please select which weather provider you would like to use",
        "defaultValue": "OpenWe",
        "options": [
          { 
          "label": "OpenWeatherMap", 
          "value": "OpenWe" 
          },
          { 
          "label": "WeatherUnderGround", 
          "value": "WUnder" 
          },
          { 
          "label": "Forecast.io", 
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