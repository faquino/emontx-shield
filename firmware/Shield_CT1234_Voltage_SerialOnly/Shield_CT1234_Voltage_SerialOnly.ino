/*
  EmonTx CT123 Voltage Serial Only example

  Part of the openenergymonitor.org project
  Licence: GNU GPL V3

  Author: Trystan Lea
*/
#include "Wire.h"
#include "EmonLib.h"


// Calibration factor = CT ratio / burden resistance = (100A / 0.05A) / 33 Ohms = 60.606
// YHDC SCT-013-000 100A - 50uA
#define ICAL 60.606

// Ideal Power 77DE-06-09 (EURO Plug type)
//#define VCAL 260.0
// newVCAL = oldVCAL * ( meter_reading / emonTX_reading)
#define VCAL 251.95

// Create  instances for each CT channel
EnergyMonitor cts[4];
//EnergyMonitor ct1, ct2, ct3, ct4;
bool ctMap[4] = { true, true, true, false };
char *ctMsgs[4] = { "CT1:", ",CT2:", ",CT3:", ",CT4:" };

// On-board emonTx LED
const int LEDpin = 9;

int           ctCount = 0;
int           cycles = 20;
int           s_tout = 2000;

unsigned long loopCount = 0;
unsigned long startTime;
unsigned long elapsTime;

String buff = "Placeholder string";

void setup()
{
  ctMap[0] = true;
  for( int i = 0; i < 4; i++) {
    if (ctMap[i]) {
      ctCount++;
    }
  }
  cycles = ( (800 / ctCount) / 2 ) * 2;
  s_tout = cycles / 2 * 20 + 250;
  unsigned long setupStart = millis();

  // Setup indicator LED
  pinMode(LEDpin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial1.begin(115200);
  // Wait for serial port to connect. Needed for Leonardo only
  while (!Serial) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
    if ((millis() - setupStart) > 10000) {
      break;
    }
  }

  if (Serial) {
    Serial.println("emonTX Shield CT123 Voltage - EmonESP");
    Serial.println("OpenEnergyMonitor.org");
    Serial.print("CT count: ");
    Serial.print(ctCount);
    Serial.print("; cycles: ");
    Serial.println(cycles);
  }

  // Calibration factor = CT ratio / burden resistance = (100A / 0.05A) / 33 Ohms = 60.606
  cts[0].current(1, ICAL);
  if (ctMap[1]) cts[1].current(2, ICAL);
  if (ctMap[2]) cts[2].current(3, ICAL);
  if (ctMap[3]) cts[3].current(4, ICAL);

  // (ADC input, calibration, phase_shift)
  cts[0].voltage(0, VCAL, 1.7);
  if (ctMap[1]) cts[1].voltage(0, VCAL, 1.7);
  if (ctMap[2]) cts[2].voltage(0, VCAL, 1.7);
  if (ctMap[3]) cts[3].voltage(0, VCAL, 1.7);
}

void loop()
{
  // Calculate all. No.of crossings, time-out
  startTime = millis();
  digitalWrite(LEDpin, HIGH);
  cts[0].calcVI(cycles, s_tout); //100
  if (ctMap[1]) cts[1].calcVI(cycles, s_tout);
  if (ctMap[2]) cts[2].calcVI(cycles, s_tout);
  if (ctMap[3]) cts[3].calcVI(cycles, s_tout);
  digitalWrite(LEDpin, LOW);
  elapsTime = millis() - startTime;

  if (loopCount > 0) {
    buildMessage();
    Serial1.println(buff);
    if (Serial) {
      Serial.println(buff);
      Serial.print("Sampling time (ms): ");
      Serial.println(elapsTime);
    }

//    Serial.print("; ");
//    Serial.print(elapsTime);
//    Serial.println();
    // Available properties: ct1.realPower, ct1.apparentPower, ct1.powerFactor, ct1.Irms and ct1.Vrms
  }

  digitalWrite(LED_BUILTIN, HIGH);
  delay(10000 - elapsTime);
  digitalWrite(LED_BUILTIN, LOW);
  loopCount++;
}

void buildMessage() {
  buff = "";
  for (int i = 0; i < 4; i++) {
    if (ctMap[i]) {
      buff += ctMsgs[i];
      buff += cts[i].realPower;
    }
  }
  buff += ",VRMS:";
  buff += cts[0].Vrms;
}
