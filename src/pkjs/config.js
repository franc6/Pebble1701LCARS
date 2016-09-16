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
        "messageKey": "HealthMeterSelect",
        "label": "The health meter bar can display your progress toward your...",
        "defaultValue": "daily",
        "options": [
          { 
          "label": "Average daily steps you have walked up to right now (health meter will be colored green)", 
          "value": "currenttime"
          },
          { 
          "label": "Average dayily steps for the entire day (health meter will be colored blue)", 
          "value": "daily"
          }]
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