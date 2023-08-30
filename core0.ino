/*********************************************************
* Core 0
* 
* This is the code that executes on the first core in the 
* ESP32. The code handles UDP broadcast messages
**********************************************************/

#include <WiFiUdp.h>

// Commands:
// color:# -> disable random colors and set the LED matrix to the selected color
// flash:#x:#y -> disable random colors, flash the lights using the selected color (#x) #y times, enable random colors
// lightning -> disable random colors, flash the lights white (like lightning), enable random colors
// off -> turn off all LEDs
// random -> Enable random colors

void Task0code(void* pvParameters) {

  const String BROADCAST_PREFIX = "pumpkin";

  WiFiUDP Udp;
  String request, searchStr;
  int color, colorPos, count;
  unsigned int localPort = 65001;       // local port to listen on
  char packetBuffer[255];               //buffer to hold incoming packet
  char ReplyBuffer[] = "acknowledged";  // a string to send back

  Serial.print("UDP Listener running on core ");
  Serial.println(xPortGetCoreID());

  Udp.begin(localPort);

  // listen for UDP broadcasts
  for (;;) {
    // if there's data available, read a packet
    int packetSize = Udp.parsePacket();
    if (packetSize) {
      IPAddress remoteIp = Udp.remoteIP();
#ifdef DEBUG
      Serial.print("Received packet of size ");
      Serial.println(packetSize);
      Serial.print("From ");
      Serial.print(remoteIp);
      Serial.print(", port ");
      Serial.println(Udp.remotePort());
#endif
      // read the packet into packetBufffer
      int len = Udp.read(packetBuffer, 255);
      if (len > 0) {
        packetBuffer[len] = 0;
      }

      // // send a reply, to the IP address and port that sent us the packet we received
      // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      // // https://forum.arduino.cc/t/solved-invalid-conversion-from-char-to-uint8_t/563582/2
      // int i = 0;
      // while (ReplyBuffer[i] != 0)
      //   Udp.write((uint8_t)ReplyBuffer[i++]);
      // Udp.endPacket();

      request = packetBuffer;
      Serial.print("Request: ");
      S_erial.println(request);

      searchStr = BROADCAST_PREFIX + "::color:";
      colorPos = searchStr.length();
      if (request.indexOf(searchStr) >= 0) {
        color = request.charAt(colorPos) - '0';
        Serial.print("Set Color #");
        Serial.println(color);
        disableRandom();
        fadeColor(colors[color]);
        continue;
      }

      searchStr = BROADCAST_PREFIX + "::flash:";
      colorPos = searchStr.length();
      if (request.indexOf(searchStr) >= 0) {
        color = request.charAt(colorPos) - '0';
        count = request.charAt(colorPos + 2) - '0';
        Serial.print("Flash color #");
        Serial.print(color);
        Serial.print(", ");
        Serial.print(count);
        Serial.println(" times");
        flashLEDs(colors[color], count);
        continue;
      }

      searchStr = BROADCAST_PREFIX + "::lightning";
      if (request.indexOf(searchStr) >= 0) {
        flicker();
        continue;
      }

      searchStr = BROADCAST_PREFIX + "::off";
      if (request.indexOf(searchStr) >= 0) {
        allOff();
        continue;
      }

      searchStr = BROADCAST_PREFIX + "::random";
      if (request.indexOf(searchStr) >= 0) {
        enableRandom();
        continue;
      }
    }
    delay(25);
  }
}
