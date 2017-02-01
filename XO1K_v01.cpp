/******************************************************************************************
  **** includes
  *****************************************************************************************/
//#include <arduino.h>// compatibility with PlatformIO
#include <FastLED.h>// rgb addressable leds
#include <ThingSpeak.h>// api for reporting to thingspeak
#include <ESP8266WiFi.h> // wifi connections
#include <DebouncedInput.h>
#include <NtpClientLib.h>

#include <TimeLib.h>

/******************************************************************************************
  **** wifi config
  *****************************************************************************************/
    char ssid[] = "your SSID";    //  your network SSID (name)
    char pass[] = "your network pwd";   // your network password
    int status = WL_IDLE_STATUS;
    
    WiFiClient  client;
    time_t getNtpTime();

    

    /******************************************************************************************
      **** fastLED setup
      *****************************************************************************************/
    // How many leds in your strip?
    #define NUM_LEDS 1

    // For led chips like Neopixels, which have a data line, ground, and power, you just
    // need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
    // ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
    #define DATA_PIN D8
    //#define brightness
    #define BRIGHTNESS  2

    /******************************************************************************************
      **** switch and vib pins setup
      *****************************************************************************************/
    #define VIB D7
    #define LIFT D3
    #define BEND D4
    #define FEATURE_ADC_VCC// for voltmeter

    #define VOLTAGE_MAX 1.0
    #define VOLTAGE_MAXCOUNTS 65535
    // Define the array of leds
    CRGB leds[NUM_LEDS];


    ADC_MODE(ADC_VCC);



unsigned long myChannelNumber = channel number goes here;
const char * myWriteAPIKey = "your write API key";


/********************************Globals***************************************************/
    int delayval = 500; // delay for half a second

    unsigned long fadeTimer;
    unsigned long lastTime;

    unsigned long checkTimer;
    int checkInterval = 50;// check for movement every 1/10 sec
    int sec = 1000; // to adjust for error in the onboard clock

struct t_time{
  int h;
  int m;
  int s;
  bool mil;
} nowish;

struct t_counts{
  int lifts;
  int bends;
  int steps;
  int reports;
} count;



/******************************************************************************************
  **** debounce setup
  *****************************************************************************************/

// We want to keep 10 entries.  That will allocate 80 extra bytes in the object -
// 2 arrays * 10 entries * 4 bytes per entry (unsigned long)
const uint8_t numToKeep = 10;

// 50ms debounce time, with the internal pull-up enabled
DebouncedInput lifting(LIFT, 250, true, numToKeep);
// 50ms debounce time, with the internal pull-up enabled
DebouncedInput bending(BEND, 250, true, numToKeep);

/*******************************Functions**************************************************/

void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {
	Serial.printf("Got IP: %s\r\n", ipInfo.ip.toString().c_str());
	NTP.begin("pool.ntp.org", 1, true);
	NTP.setInterval(63);
	digitalWrite(2, LOW);
}

void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {
	Serial.printf("Disconnected from SSID: %s\n", event_info.ssid.c_str());
	Serial.printf("Reason: %d\n", event_info.reason);
	digitalWrite(2, HIGH);
}

/******************************************************************************************
  **** addzeros function to pad numbers for printing
  *****************************************************************************************/
String addzeros(int num){
     if (num < 10) {
        String newnum = ("0");
        newnum = newnum + String(num);
    return newnum;
    }else return String(num);
}//end addzeros

/******************************************************************************************
  **** thingspeak report
  *****************************************************************************************/

void thingSend(){//////////////////////////////////////////////////////////////
//  int sensorValue = analogRead(A0);
//  Serial.print("Raw Volts reading: ");Serial.println(sensorValue);

//  float voltage = sensorValue * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);
//  Serial.print("voltage reading: ");Serial.println(voltage);
  String timeStamp;
    timeStamp = timeStamp + String(nowish.h);
    timeStamp += (":");
    timeStamp = timeStamp + String(addzeros(nowish.m));
    timeStamp += (":");
    timeStamp = timeStamp + String(addzeros(nowish.s));
  Serial.print("time stamp: "); Serial.println(timeStamp);

//startup the wifi again...
/*  WiFi.forceSleepWake();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print(" wifi login...");

//if (client) {//wait for wifi to connect
  *timeClient.update();//update NTP time
Serial.print(timeClient.getFormattedTime());
  */
  Serial.println("loading unsafe lift to Thingspeak");
  ThingSpeak.setField(1,count.bends);
  Serial.println("loading lift to thingspeak");
  ThingSpeak.setField(2,count.lifts);
  Serial.println("loading timestamp to thingspeak");
  ThingSpeak.setField(3,timeStamp);
// Serial.println("loading voltage to thingspeak");
// ThingSpeak.setField(4,voltage);
// update and Write the fields that you've set all at once.
Serial.println("sending to thingspeak channel");
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
count.reports++;
Serial.println("sent to thingspeak");
  //Serial.print("I just wrote "); Serial.print(unsafeCount); Serial.print(" unsafe lifts and "); Serial.print(liftCount);Serial.print(" good lifts");Serial.println(" to ThingSpeak.");
  //Serial.print("at "); Serial.print(nowish.h);Serial.print(":");Serial.print(nowish.m);Serial.print(":");Serial.print(nowish.s);Serial.println(".");//report updated time
    /*  WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      WiFi.forceSleepBegin();
      delay(1);*/
}//end thingsend

/******************************************************************************************
  **** heartbeat
  *****************************************************************************************/
void heartbeat(){
//int combinedState = (liftReading + bendReading);
int maxBrite = 64;//change this to change the brightness of the heartbeat
  // fade in from min to max in increments of 5 points:
  if (millis() >(fadeTimer)){

    for (int fadeValue = 0 ; fadeValue <= maxBrite; fadeValue += 1) {
      // sets the value (range from 0 to maxbrite):

      leds[0] = CHSV(96,255,fadeValue);//green
      FastLED.show();
      // wait for 30 milliseconds to see the dimming effect
      fadeTimer = millis();
      }

    for (int fadeValue = maxBrite ; fadeValue >= 0; fadeValue -= 1) {
    // sets the value (range from maxbrite to 0):

    leds[0] = CHSV(96,255,fadeValue);//green
    FastLED.show();
      // wait for 30 milliseconds to see the dimming effect
      fadeTimer = millis();

      }
      Serial.print("heartbeat...");
      fadeTimer +=30000;
      thingSend();
    }//end if time
}//end heartbeat

/******************************************************************************************
  **** vibrate
  *****************************************************************************************/
void vibrateOn(int vibtime){
 unsigned long vibTimer = millis();
  while (millis() < (vibTimer + vibtime)){
   digitalWrite(VIB,HIGH);
 }
 digitalWrite(VIB,LOW);
}

/******************************************************************************************
  **** keeptime
  *****************************************************************************************/
void keepTime(){
  if ((millis()+sec) >> lastTime) {
    nowish.h = hour();
    nowish.m = minute();
    nowish.s = second();
    }//endif
    lastTime = millis();
  }//end keeptime



/******************************************************************************
  **** general setup commands
  ****************************************************************************/
void setup() {/////////////////////////////////////////////////////////////////
    static WiFiEventHandler e1, e2;
  pinMode(BEND, INPUT); // unsafe lift angle
  pinMode(LIFT, INPUT); // safe lift angle
  //pinMode(D2, INPUT); // level angle (pedometer?)
  pinMode(VIB, OUTPUT); // Vib motor
  // Set the pinMode and get the initial button state.
	lifting.begin();
  bending.begin();

  Serial.begin(115200);
  Serial.println("starting Wifi...");
  WiFi.begin(ssid, pass);


FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);


NTP.begin();
NTP.setTimeZone(-6);
	NTP.onNTPSyncEvent([](NTPSyncEvent_t ntpEvent) {
		if (ntpEvent) {
			Serial.print("Time Sync error: ");
			if (ntpEvent == noResponse)
				Serial.println("NTP server not reachable");
			else if (ntpEvent == invalidAddress)
				Serial.println("Invalid NTP server address");
		}
		else {
			Serial.print("Got NTP time: ");
			Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
		}
	});
	WiFi.onEvent([](WiFiEvent_t e) {
		Serial.printf("Event wifi -----> %d\n", e);
	});
	e1 = WiFi.onStationModeGotIP(onSTAGotIP);// As soon WiFi is connected, start NTP Client
	e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
    
    ThingSpeak.begin(client);// thingspeak client
        Serial.println("starting thingspeak...");

  //  Serial.println(timeClient.getFormattedTime());
/*
WiFi.disconnect();
    Serial.println("wifi disconnected.");
WiFi.mode(WIFI_OFF);
WiFi.forceSleepBegin();
    Serial.println("wifi sleeping");
delay(1);
*/}//end setup

void loop() {//////////////////////////////////////////////////////////////////
	static int i = 0;
	static int last = 0;

	if ((millis() - last) > 5100) {
		//Serial.println(millis() - last);
		last = millis();
		Serial.print(i); Serial.print(" ");
		Serial.print(NTP.getTimeDateString()); Serial.print(" ");
		Serial.print(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
		Serial.print("WiFi is ");
		Serial.print(WiFi.isConnected() ? "connected" : "not connected"); Serial.print(". ");
		Serial.print("Uptime: ");
		Serial.print(NTP.getUptimeString()); Serial.print(" since ");
		Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

		i++;
	}

	delay(0);
  // If the state has changed to low then, for each of the previous 10 entries,
	// print the difference between them and the current time.
if (millis() > (checkTimer+checkInterval)) {// every 100ms check to see the switches' states


  if (bending.changedTo(LOW)) {
		unsigned long now = millis();
    leds[0] = CHSV(0,255,64);//red
    FastLED.show();
    count.bends++;
    vibrateOn(500);
    Serial.println("Bending detected, vibrate on");
		Serial.print(F("Time since last "));
		Serial.print(numToKeep);
		Serial.println(F(" button presses:"));
		for (int i = 0; i < 10; i++) {
			Serial.print(F("  "));
			Serial.print(now - bending.getLowTime(i));
			Serial.println(F("ms"));
		}
	}else
  if (lifting.changedTo(LOW)) {
		unsigned long now = millis();
    leds[0] = CHSV(64,255,128); //yellow
    FastLED.show();
    digitalWrite(VIB,LOW);
    count.lifts++;
    Serial.println("Lifting detected, vibrate off");
		Serial.print(F("Time since last "));
		Serial.print(numToKeep);
		Serial.println(F(" button presses:"));
		for (int i = 0; i < 10; i++) {
			Serial.print(F("  "));
			Serial.print(now - lifting.getLowTime(i));
			Serial.println(F("ms"));
		}
	}else
  leds[0] = CHSV(96,255,0);//lights off
  FastLED.show();

  heartbeat();//if we're not bending, just keep the heartbeat going
/*  leds[0] = CHSV(0,255,0);//black
  FastLED.show();
  digitalWrite(VIB,LOW);*/
  //Serial.println("nothing to see here.");
  }
keepTime();//update the time

}//end loop
