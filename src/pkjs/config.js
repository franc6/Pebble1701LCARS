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
        "type": "radiogroup",
        "messageKey": "HealthMeterSelect",
        "label": "The health meter bar can display your progress toward your...",
        "defaultValue": "daily",
        "options": [
          { 
          "label": "average daily steps you have walked up to right now", 
          "value": "currenttime"
          },
          { 
          "label": "average dayily steps for the entire day", 
          "value": "daily"
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
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];