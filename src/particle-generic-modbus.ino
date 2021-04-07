	/*
 * Project Generic Modbus Driver for Particle Gen3 Hardware (Argon wifi or Boron cellular)
 * Description: modbus driver - get and set 16bit integer values (signed or unsigned)
 * Author: tim@hudsonsonoma.com
 * Date: 25 March 2021
 */

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

#define NODE_UNIT_ID 1   // The address of the modbus unit to connect to
#define NODE_BAUD 9600   // The baud rate of the modbus unit
#define EVENT_NAME String("GenericModbus") // expect EVENT_NAME/status and EVENT_NAME/cmd_resp events to be published
#define STATUS_FREQ 1 // minutes per status update (default every 1 minute)

/*  
1. Every STATUS_FREQ minutes, this status JSON will be published:
EVENT_NAME/status:
{
  "0": 301,
  "1": 403,
  "2": 1024,
  "3": 0
}

2. When a command is executed, this JSON will be published:

GetOrSet_Register_Unsigned_regORregEQvalue_0_65535 OR
GetOrSet_Register_Signed_regORregEQvalue_-32767_32767
Enter modbus register number (0-indexed) to retrieve its value, as either a signed or unsigned 16-bit integer
0
(retrieves value of the first register)
0=5
(sets the value of the first register to 5)

EVENT_NAME/cmd_resp:
[
  {
    "function": "set_signed",
    "command": "-625",
    "result": "-625",
    "error": 0
  }
]
(typically a 1 element JSON array)

3. You may send events that call functions to this device 
Thanks to ParticleFunctionCaller(), you may also call Particle.functions from subscribed events:
Subscribed by default to EVENT_NAME/function/{{{PARTICLE_DEVICE_ID}}} 

for example:
$ particle publish Novus322/function/e00fce68ee01234a8b1ffc0c '{"function":"set_setpoint_1","command":"29.9"}'

You may direct webhook responses to this device (so that webhook response JSON may trigger particle functons)
Add this configuration to the particle console: 
Integrations / Your Integration / Edit / Advanced / Webhook Responses / Response Topic
EVENT_NAME/function/{{{PARTICLE_DEVICE_ID}}}

Functions may be called as such:  (set the first register (0) to the signed value -1)
{"function":"set_signed","command":"0=-1"}

*/



#define CTRL_PIN D6
#define TEMP_REGISTERS 0 // starting offset of modbus holding registers

#include "ModbusMaster.h"
#include "JsonParserGeneratorRK.h"

SerialLogHandler logHandler(9600,LOG_LEVEL_WARN, {
    {"app", LOG_LEVEL_TRACE},
    {"system", LOG_LEVEL_INFO}
});

// connect to modbus node NODE_UNIT_ID
ModbusMaster node(NODE_UNIT_ID);
JsonWriterStatic<512> jw;

void setup() {
	pinMode(CTRL_PIN,OUTPUT);

	//selectExternalMeshAntenna();  UNCOMMENT to select external antenna - ONLY IF IT IS CONNECTED.

	node.begin(NODE_BAUD);
	node.enableTXpin(CTRL_PIN);
	node.enableDebug();

    delay(5000);

	// Edit or comment out to control what options are presented 
	// in the Particle.io mobile app interface
	// Also remember to add function calls to ParticleFunctionCaller()
    Particle.function("GetOrSet_Register_Unsigned_regORregEQvalue_0_65535",set_unsigned);
    Particle.function("GetOrSet_Register_Signed_regORregEQvalue_-32767_32767",set_signed);

    // Handle incoming function calls via subscribe
    // Uncomment the following line to allow webhooks to respond with particle function calls
    // Particle.subscribe(EVENT_NAME + "/function/" + System.deviceID(), ParticleFunctionCaller, MY_DEVICES);

    Particle.publishVitals(60*60); // publish vitals every hour 

}

void ParticleFunctionCaller(const char *eventName, const char *data) {

	JSONValue outerObj = JSONValue::parseCopy(data);
    JSONObjectIterator iter(outerObj);
    String functionName;
    String command;
    bool hasFunction = false;
    bool hasCommand = false;

    while(iter.next()) {
       	if ( String(iter.name()) == "function") { functionName = String(iter.value().toString()); hasFunction = true;}
       	if ( String(iter.name()) == "command") { command = String(iter.value().toString()); hasCommand = true; }
    }

    // PUBLISHED FUNCTIONS: ADD ALL Particle Functions here that you wish to allow webhooks to call.
    if (hasFunction && hasCommand) {
    	if (functionName == "set_unsigned") { set_unsigned(command); }
    	else if (functionName == "set_signed") { set_signed(command); }
    	// .. add more as needed
    } 

}

bool cmd_resp(String function, String command, String result, int error) {
	jw.clear();
	jw.startArray();
	jw.startObject();
	jw.insertKeyValue("function", function);
	jw.insertKeyValue("command", command);
	jw.insertKeyValue("result", result);
	jw.insertKeyValue("error", error);
	jw.finishObjectOrArray();
	jw.finishObjectOrArray();
	String json = jw.getBuffer();
	jw.clear();
	if (json.charAt(0) == ',') {
		json.setCharAt(0,' ');
	} // remove leading comma
	return (Particle.publish(EVENT_NAME + "/cmd_resp", json.c_str(), PRIVATE));

}

/* ************************ */

// Functions to set various modbus registers.
// Code is identical, except for the macros which are custom to each register.
int set_signed(String command) {
#define REGISTER 0
#define FUNCTION_NAME "set_signed"
#define CONSTRAIN(floatval) (constrain(floatval, -32768, 32768))
#define STRING_TO_MODBUS_VALUE(command,modbus_value) \
	float floatval = command.toFloat();  \
	floatval = CONSTRAIN(floatval);      \
	modbus_value = (int16_t)(floatval)
#define MODBUS_VALUE_TO_STRING(modbus_value) (String::format("%i", ((int16_t) modbus_value )) )

	uint8_t modbus_code;
	uint16_t modbus_value;
	uint16_t value;

	command = command.trim();
	int i = command.indexOf('=');
	if (i == -1 ) {
		int offset = command.toInt();
		modbus_code = node.readHoldingRegisters(REGISTER+offset,1);
    	if (modbus_code == node.ku8MBSuccess) {
            modbus_value = node.getResponseBuffer(0);
            cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
        	return (int16_t) modbus_value;
		} else { cmd_resp(FUNCTION_NAME,command,"",-2); return -2; }
	} else {	
		int offset = command.substring(0,i-1).toInt();
		String string_val = command.substring(i+1);
		STRING_TO_MODBUS_VALUE(string_val,modbus_value);
		modbus_code = node.writeSingleRegister(REGISTER,(uint16_t)modbus_value);
		if (modbus_code != node.ku8MBSuccess) { cmd_resp(FUNCTION_NAME,command,"",-3); return -3; }
		delay(250);
		modbus_code = node.readHoldingRegisters(REGISTER,1);
	    if (modbus_code == node.ku8MBSuccess) {
	       	modbus_value = node.getResponseBuffer(0);
	       	cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
	       	return (int16_t) modbus_value;
		} else { cmd_resp(FUNCTION_NAME,command,"",-4); return -4; }
	}
}

int set_unsigned(String command) {
#define REGISTER 0
#define FUNCTION_NAME "set_unsigned"
#define CONSTRAIN(floatval) (constrain(floatval, 0, 65535))
#define STRING_TO_MODBUS_VALUE(command,modbus_value) \
	float floatval = command.toFloat();  \
	floatval = CONSTRAIN(floatval);      \
	modbus_value = (uint16_t)(floatval)
#define MODBUS_VALUE_TO_STRING(modbus_value) (String::format("%u", ((uint16_t) modbus_value )) )

	uint8_t modbus_code;
	uint16_t modbus_value;
	uint16_t value;

	command = command.trim();
	int i = command.indexOf('=');
	if (i == -1 ) {
		int offset = command.toInt();
		modbus_code = node.readHoldingRegisters(REGISTER+offset,1);
    	if (modbus_code == node.ku8MBSuccess) {
            modbus_value = node.getResponseBuffer(0);
            cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
        	return (uint16_t) modbus_value;
		} else { cmd_resp(FUNCTION_NAME,command,"",-2); return -2; }
	} else {	
		int offset = command.substring(0,i-1).toInt();
		String string_val = command.substring(i+1);
		STRING_TO_MODBUS_VALUE(string_val,modbus_value);
		modbus_code = node.writeSingleRegister(REGISTER,(uint16_t)modbus_value);
		if (modbus_code != node.ku8MBSuccess) { cmd_resp(FUNCTION_NAME,command,"",-3); return -3; }
		delay(250);
		modbus_code = node.readHoldingRegisters(REGISTER,1);
	    if (modbus_code == node.ku8MBSuccess) {
	       	modbus_value = node.getResponseBuffer(0);
	       	cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
	       	return (uint16_t) modbus_value;
		} else { cmd_resp(FUNCTION_NAME,command,"",-4); return -4; }
	}
}


/* ************************* */

unsigned long previousMillis1 = 0;
void loop() {
    // do nothing :P

	unsigned long currentMillis1 = millis();
    if(currentMillis1 - previousMillis1 > (STATUS_FREQ * 60 * 1000) ) {
        previousMillis1 = currentMillis1;

	    uint8_t result = node.readHoldingRegisters(TEMP_REGISTERS,4);

	    if (result == node.ku8MBSuccess) {
	        uint16_t value = node.getResponseBuffer(0);
	        //Log.info("Received: %0x",value);

				jw.startObject();

				// Add various types of data
				// Edit or comment out lines to control what
				// data is reported in Particle.io mobile app Events
				jw.insertKeyValue("0",  node.getResponseBuffer(0) );
				jw.insertKeyValue("1", node.getResponseBuffer(1)  );
				jw.insertKeyValue("2", node.getResponseBuffer(2));
				jw.insertKeyValue("3", node.getResponseBuffer(3));
				// Add more registers you wish to see in the status report.

				jw.finishObjectOrArray();

			Serial.printlnf("%s", jw.getBuffer());
			String json = jw.getBuffer();
			if (json.charAt(0) == ',') {
				json.setCharAt(0,' ');
			} // remove leading comma if neccesary
			Particle.publish(EVENT_NAME + "/status", json.c_str(), PRIVATE); 
			jw.clear();

	    } else {
	        Serial.print("Failed, Response Code: ");
			Serial.print(result, HEX); 
			Serial.println("");
	    }

	}

	delay(250);
	
}


void selectExternalMeshAntenna()
{
#if   (PLATFORM_ID == PLATFORM_ARGON)
	digitalWrite(ANTSW1, 1);
	digitalWrite(ANTSW2, 0);
#elif (PLATFORM_ID == PLATFORM_BORON)
	digitalWrite(ANTSW1, 0);
#elif (PLATFORM_ID == PLATFORM_XENON)
	digitalWrite(ANTSW1, 0);
	digitalWrite(ANTSW2, 1);
#endif
}





