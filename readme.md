# Glowing Pumpkin Server (UDP)

This is the [Glowing Pumpkin Xiao 5x5 BFF](https://github.com/johnwargo/glowing-pumpkin-xiao-bff) project with a UDP broadcast listener added to you can control the LEDs from a smartphone, tablet, of desktop PC. The original project changed colors randomly and periodically flashed the LEDs white to simulate sparks or lightning. 

This is slightly modified from [Glowing Pumpkin Server (HTTP)](https://github.com/johnwargo/glowing-pumpkin-server-http); this version uses UDP Broadcast messages to send commands to turn the LEDs on and off, etc, but it commands multiple devices simultaneously. My goal with this version is to setup multiple pumpkins in dormers in my house and have them all doing the same thing at the same time.

The project runs on a [Seeed Studio Xiao ESP32 device](https://www.seeedstudio.com/xiao-series-page) with a [5x5 NeoPixel array](https://www.adafruit.com/product/5646). You can use any other device and LED configuration if you want using this project as a starting point, but the web server is specific to the ESP32 device family, so that limits your microcontroller options. 

## The API

The API exposed through the web server looks like this:

| Method       | Description |
| ------------ | ----------- |
| `/color:#`   | Lights all of the NeoPixels with a specific color. The command accepts a single parameter, the array index for the selected color. So, for example, to illuminate all of the LEDs Purple, the command is `color:3` as Purple is the 4th color in the array (with an index of 3 since C arrays start at 0). To illuminate the LEDs Blue, the command is `color:0`. |
| `/flash:#:#` | Flash the LED matrix a specific color a specific number of times. For example, to flash the LED matrix Blue three times, the command is `flash:0:3`. |
| `/lightning` | Flash the LED matrix White a random number of times for a random amount of time for each flash in an attempt to simulate lightning or electrical flashes inside the pumpkin. |
| `/off`       | Turns off the LED Matrix (sets all NeoPixels to `CRGB::Black`). |
| `/random`    | Enables random mode where the ESP32 device randomly picks a color from the array and displays it for a random amount of time (seconds) before picking another color and starting all over again. |

## The App

The sample app I published for the HTTP version ([Glowing Pumpkin Controller](https://github.com/johnwargo/glowing-pumpkin-controller-html/tree/main)) doesn't work with this project; I didn't recode it to support UDP.  To control this device, you must send your own UDP broadcast messages in the right format, or you can use the [Pumpkin Controller (Flutter)](https://github.com/fumblystuff/pumpkin-controller-app-flutter) on a phone or tablet.

Here's a screenshot of the mobile app, you basically tap buttons and it makes corresponding stuff happen on compatible devices nearby. 

![](images/home.png)

The app defaults to prepending `pumpkin::` to every API request so the sketch expects that. 

![](images/settings-udp.png)



## The Code

I wrote extensively about the project code in [Arduino ESP32 Web Server on a Processor Core](https://johnwargo.com/posts/2023/arduino-esp32-web-server-on-a-processor-core/), so you have that as a reference. It explains how I coded the web server, here I'll explain how to configure it and explain a bit about what the code does.

### Wi-Fi and Wi-Fi Settings

The web server connects to a Wi-Fi network, so the first thing you must do is provide the project sketch with the local network credentials. In the repository, you'll see a file called `constants.h.rename`; rename that file to `constants.h` then open the file in the Arduino IDE. 

The file currently looks like this:

```c
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
```

Edit the file and add the local Wi-Fi network [SSID](https://en.wikipedia.org/wiki/Service_set_(802.11_network)#SSID) (network name) between the quotes in the `WIFI_SSID` define. Add the network access password between the quotes in the `WIFI_PASSWORD` define.  The file should look something like this but with your network name and password:

```c
#define WIFI_SSID "my_network_name"
#define WIFI_PASSWORD "my_password"
```

### Making the Server Findable

The sketch uses the [ESPmDNS](https://github.com/espressif/arduino-esp32/blob/master/libraries/ESPmDNS/src/ESPmDNS.h) library to register a human friendly name for the web server on the local network. This happens with the following code taken slightly out of context from the sketch:

```c
#include <ESPmDNS.h>
#define HOSTNAME "pumpkin"

if (MDNS.begin(HOSTNAME)) {
    displayMessage("MDNS responder started");
    MDNS.addService("http", "tcp", 80);
} else {
  displayMessage("Error setting up MDNS responder!");
  fadeColor(CRGB::Red);
  while (1) {
    delay(1000);
  }
}
```

If you run the sketch on the device and the LEDs turn solid red and stay that way, that means that the code failed to configure the device's name on the network and you have some troubleshooting to do. 

Basically what this code does is register the name `pumpkin.local` on the network, with this in place, you can open the browser and access the server by typing `pumpkin.local` in the browser's address field. Essentially, the browser launches a simple page from the web server, redirects to the hosted web app and even self-configures the controller app with the IP Address of the server. Refer to [Arduino ESP32 Web Server on a Processor Core](https://johnwargo.com/posts/2023/arduino-esp32-web-server-on-a-processor-core/) for additional information.

If you want to use a different name for the device, simply change the value between the quotes in the `HOSTNAME` define statement. Compile the sketch and deploy it to the device and the web server will respond to the new name (with `.local` appended to the end of it).

### The Web Server

The primary web server implementation is in the following code:

```c
server.enableCORS();
server.on("/", handleRoot);
server.on(UriBraces("/color:{}"), handleColor);
server.on(UriBraces("/flash:{}"), handleFlash);
server.on("/lightning", handleFlicker);
server.on("/off", handleOff);
server.on("/random", handleRandom);
server.onNotFound(handleNotFound);
server.begin();
displayMessage("HTTP server started\n");

for (;;) {
  server.handleClient();
  // Add a small delay to let the watchdog process
  //https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time
  delay(25);
}
```

This code:

1. Enables CORS so the web server automatically adds the needed headers to every response.
2. Adds an event handler for root requests for the web server (/').
3. Adds event handlers for the different API commands (described above).
4. Adds an event handler for requests that don't match any of the defined requests (404 - not found).
5. Starts the server.
6. Writes to the Serial Monitor letting you know that the server started.

Refer to [Arduino ESP32 Web Server on a Processor Core](https://johnwargo.com/posts/2023/arduino-esp32-web-server-on-a-processor-core/) for additional information about what happens from there.

***

You can find information on many different topics on my [personal blog](http://www.johnwargo.com). Learn about all of my publications at [John Wargo Books](http://www.johnwargobooks.com).

If you find this code useful and feel like thanking me for providing it, please consider <a href="https://www.buymeacoffee.com/johnwargo" target="_blank">Buying Me a Coffee</a>, or making a purchase from [my Amazon Wish List](https://amzn.com/w/1WI6AAUKPT5P9).
