var GenericWeather = require('pebble-generic-weather');

var genericWeather = new GenericWeather();

Pebble.addEventListener('ready', function(e) {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('appmessage: ' + JSON.stringify(e.payload));
  genericWeather.appMessageHandler(e);
});

// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);
