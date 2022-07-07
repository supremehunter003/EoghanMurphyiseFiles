
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 8); // CE, CSN
//const byte address[6] = "00001";
const uint64_t slaveID[2] = {0xE8E8F0F0E1LL, 0xE8E8F0F0E2LL} ;
char xData[32] = "";
char yData[32] = "";
String xAxis, yAxis;
void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setDataRate( RF24_2MBPS );
  radio.openWritingPipe(slaveID[0]);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}
void loop() {
  
  xAxis = analogRead(A0); // Read Joysticks X-axis
  yAxis = analogRead(A1); // Read Joysticks Y-axis
  // X value
  xAxis.toCharArray(xData, 5); // Put the String (X Value) into a character array
  radio.write(&xData, sizeof(xData)); // Send the array data (X value) to the other NRF24L01 modile
  Serial.println(xData);
  // Y value
  yAxis.toCharArray(yData, 5);
  radio.write(&yData, sizeof(yData));
  Serial.println(yData);
  
  
}
