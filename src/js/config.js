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
        "defaultValue": "SETTINGS"
      },
      {
        "type": "input",
        "messageKey": "APIKEY",
        "defaultValue": "",
        "label": "OpenWeatherMap API Key"
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