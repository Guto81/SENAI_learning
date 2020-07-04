
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "wc5WtLZLYByP8Cbj4xYc6xLWVobP8gmT";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Hermione2";
char pass[] = "10M16G01A";

//byte statusLed    = 13;

byte inFlowSensor = D5;  


float calibrationFactor = 4.5;
BlynkTimer timer;

volatile byte pulseCount;  

float inFlowRate; // V2 - inflowrate 

boolean sensorInput = 0;
unsigned int inFlowMilliLitres; 
unsigned long inTotalMilliLitres; // V1 - inTotalLitres
unsigned long oldTime;

//unsigned long totalLitres;


BLYNK_CONNECTED() { // runs once at device startup, once connected to server.

  Blynk.syncVirtual(V1); //gets last known value of V1 virtual pin
  Blynk.syncVirtual(V2);  
}

BLYNK_WRITE(V1)
{
  inTotalMilliLitres = param.asFloat();

}

BLYNK_WRITE(V2)
{
  inFlowRate = param.asFloat();

}


BLYNK_WRITE(V5) {  // reset all data with button in PUSH mode on virtual pin V4
  int resetdata = param.asInt();
  if (resetdata == 0) {
    Serial.println("Clearing Data");
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 0);
    inFlowRate = 0;
    inFlowMilliLitres = 0;
    inTotalMilliLitres = 0;
    //totalLitres = 0;
    //totalLitresold = 0;
  }
}

ICACHE_RAM_ATTR void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

void inflow()
{

   if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    detachInterrupt(inFlowSensor);

    inFlowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;

    oldTime = millis();

    inFlowMilliLitres = (inFlowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    inTotalMilliLitres += inFlowMilliLitres;

    unsigned int frac;

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(inFlowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (inFlowRate - int(inFlowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Fuel Flowing: ");             // Output separator
    Serial.print(inFlowMilliLitres);
    Serial.print("mL/Sec");

    // Print the cumulative total of litres flowed since starting
    Serial.print("  Input Fuel Quantity: ");             // Input separator
    Serial.print(inTotalMilliLitres);
    Serial.println("mL"); 

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;

    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(inFlowSensor, pulseCounter, FALLING);
  }
}


void sendtoBlynk()  // In this function we are sending values to blynk server
{
  Blynk.virtualWrite(V2, inFlowRate);
  Blynk.virtualWrite(V1, inTotalMilliLitres);

}

void setup()
{

  Serial.begin(9600); //38400
  Blynk.begin(auth,ssid,pass);
  Serial.println("Setup Started");

  pulseCount = 0;
  inFlowRate = 0.0;
  inFlowMilliLitres = 0;
  inTotalMilliLitres = 0;
  oldTime = 0;

  attachInterrupt(inFlowSensor, pulseCounter, FALLING);
  
  timer.setInterval(10000L, sendtoBlynk);
}

void maincode(){
  inflow();

}

/**
 *  program loop
 */
void loop(){
  Blynk.run();
  timer.run();
  Serial.println("Timer and Blynk Started");
  Serial.println(inFlowSensor);
  maincode();
}
