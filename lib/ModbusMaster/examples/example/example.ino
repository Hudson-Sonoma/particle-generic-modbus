#include "ModbusMaster.h"

// instantiate ModbusMaster object as slave ID 1
ModbusMaster node(1);


void setup() {
	// initialize Modbus communication baud rate
	node.begin(9600);
	node.enableTXpin(D7); //D7 is the pin used to control the TX enable pin of RS485 driver
	//node.enableDebug();  //Print TX and RX frames out on Serial. Beware, enabling this messes up the timings for RS485 Transactions, causing them to fail.
	
	Serial.begin(9600);
	while(!Serial.available()) Particle.process();
	Serial.println("Starting Modbus Transaction:");
}


void loop() {
	static uint32_t i;
	uint8_t j, result;
	uint16_t data[10];

	i++;

	result = node.readHoldingRegisters(0x2f,2);

	Serial.println("");
	
	// do something with data if read is successful
	if (result == node.ku8MBSuccess) {
		Serial.print("Success, Received data: ");
		for (j = 0; j < 2; j++) {
			data[j] = node.getResponseBuffer(j);
			Serial.print(data[j], HEX);
			Serial.print(" ");
		}
		Serial.println("");
	} else {
		Serial.print("Failed, Response Code: ");
		Serial.print(result, HEX); 
		Serial.println("");
		delay(5000); //if failed, wait for bit longer, before retrying!
	}
	delay(1000);
}