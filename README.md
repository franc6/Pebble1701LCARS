# Pebble1701LCARS
This is a pebble time watchface I wrote for myself.  The code is a combination of what I remember from long ago at college, libraries, and others example watchfaces.  Feel free to use the code, if you are going to open source please attribute me as a source and if closed source please drop me a line so I can enjoy your creation

I wrote the watchface in CloudPebble because there were no exsisting watchfaces that combined my wishes and some existing devs I contacted had no interest in adding my wants to their code.  

This watchface displays:
-A "stardate" like element at the top which is the the numerical date mmddyyyy. followed by the current number of steps reported by pebble health.
-The engines will power up and down correspoinding to your watch's battery life
-The watchface will also display the battery life as a numerical value, you can configure to hide this if you wish
-The watchface uses the user created pebble-generic-weather library to get weather from Wunderground, Forcast.io, and OpenWeatherMap and you will need to sign up at https://www.wunderground.com/weather/api/d/docs, https://developer.forecast.io/, or http://openweathermap.org/appid to obtain a free API key. The API key is to be entered on the watchface settings page
-The current time
-A life support bar that indicates your progress to meeting either your instanteous average steps or your daily steps

Thank you to
https://github.com/gregoiresage for the pebble-generic-weather library.  Always a good idea to use good code than to rewrite!
@C-D-Lewis @cat-haines and @Katharine for  owm-weather on which generic-weather is based
https://github.com/orviwan for the pebble-clay library
Jeroen the devoloper of https://apps.getpebble.com/applications/577527d3ba2fe5a0c1000599 for office hours to knock out some of the bugs in my code
