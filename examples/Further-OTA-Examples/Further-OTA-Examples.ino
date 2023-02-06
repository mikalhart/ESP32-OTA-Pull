/*
Further-OTA-Examples - other techniques for using ESP32-OTA-Pull library for OTA
Copyright (C) 2022-3 Mikal Hart
All rights reserved.

https://github.com/mikalhart/ESP32-OTA-Pull

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files 
(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Arduino.h>
#include "ESP32OTAPull.h"

// First, edit these values appropriately
#if __has_include("settings.h") // optionally override with values in settings.h
#include "settings.h"
#else
#define JSON_URL   "https://example.com/myimages/Further-OTA-Examples.json" // this is where you'll post your JSON filter file
#define SSID 	   "<my WiFi SSID>"
#define PASS       "<my WiFi Password>"
#define VERSION    "1.0.0" // The current version of this program
#endif

void callback_dots(int offset, int totallength);
void callback_percent(int offset, int totallength);

void setup()
{
	Serial.begin(115200);
	delay(2000); // wait for ESP32 Serial to stabilize

	Serial.printf("Connecting to WiFi '%s'...", SSID);
	WiFi.begin(SSID, PASS);
	while (!WiFi.isConnected())
	{
		Serial.print(".");
		delay(250);
	}
	Serial.printf("\n\n");

	ESP32OTAPull ota;

	// Example 1: See if an update is available (but don't do it).  No callback routine
	Serial.printf("Check for update, but don't download it.\n");
	int ret = ota
		.CheckForOTAUpdate(JSON_URL, VERSION, ESP32OTAPull::DONT_DO_UPDATE);
	Serial.printf("CheckForOTAUpdate returned %d (%s)\n\n", ret, errtext(ret));

	// After we've checked, we can obtain the version from the JSON file
    String otaVersion = ota.GetVersion();
	Serial.printf("OTA Version Available: %s\n", otaVersion.c_str());

	delay(3000);

	// Example 2
	Serial.printf("Check for update and download it, but don't reboot.  Display dots.\n");
	ret = ota
		.SetCallback(callback_dots)
		.CheckForOTAUpdate(JSON_URL, VERSION, ESP32OTAPull::UPDATE_BUT_NO_BOOT);
	Serial.printf("CheckForOTAUpdate returned %d (%s)\n\n", ret, errtext(ret));

	delay(3000);

	// Example 3
	Serial.printf("Download and install downgrade, but only if the configuration string matches.  Display percentages.\n");
	ret = ota
		.SetCallback(callback_percent)
		.AllowDowngrades(true)
		.SetConfig("4MB RAM")
		.CheckForOTAUpdate(JSON_URL, "99.99.99", ESP32OTAPull::UPDATE_AND_BOOT);
	Serial.printf("CheckForOTAUpdate returned %d (%s)\n\n", ret, errtext(ret));
}

void loop()
{
}

const char *errtext(int code)
{
	switch(code)
	{
		case ESP32OTAPull::UPDATE_AVAILABLE:
			return "An update is available but wasn't installed";
		case ESP32OTAPull::NO_UPDATE_PROFILE_FOUND:
			return "No profile matches";
		case ESP32OTAPull::NO_UPDATE_AVAILABLE:
			return "Profile matched, but update not applicable";
		case ESP32OTAPull::UPDATE_OK:
			return "An update was done, but no reboot";
		case ESP32OTAPull::HTTP_FAILED:
			return "HTTP GET failure";
		case ESP32OTAPull::WRITE_ERROR:
			return "Write error";
		case ESP32OTAPull::JSON_PROBLEM:
			return "Invalid JSON";
		case ESP32OTAPull::OTA_UPDATE_FAIL:
			return "Update fail (no OTA partition?)";
		default:
			if (code > 0)
				return "Unexpected HTTP response code";
			break;
	}
	return "Unknown error";
}


void callback_percent(int offset, int totallength)
{
	static int prev_percent = -1;
	int percent = 100 * offset / totallength;
	if (percent != prev_percent)
	{
		Serial.printf("Updating %d of %d (%02d%%)...\n", offset, totallength, 100 * offset / totallength);
		prev_percent = percent;
	}
}

void callback_dots(int offset, int totallength)
{
	Serial.print(".");
}
