
#include "wifi.h"


void connect_to_wifi(DebugSerial debug, const char *ssid, const char *password) {
#ifdef ESP32
      WiFi.setSleep(false);
#endif

    int n = WiFi.scanNetworks();
    debug.printf("%d networks found\n", n);
    for(int i = 0; i < n; i++)
    {
        debug.printf("\t%s\n", WiFi.SSID(i).c_str());
    }

    debug.printf("Establishing connection to '%s'", ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        debug.print(".");
        delay(500);
    }
    debug.println();

    debug.printf("Connected as '%s'\n\n", WiFi.localIP().toString().c_str());
}

