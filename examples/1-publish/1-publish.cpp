#include "Particle.h"

#include "CallbackLogHandler.h"

SYSTEM_THREAD(ENABLED);

void callback(uint8_t* buf, size_t length) {
	if(!Particle.connected()) return;
	
	Particle.publish("log", (char*)buf, PRIVATE);
}

// A single publish message can be up to 622 bytes large
CallbackLogHandler<2048, 622> logHandler(callback);

size_t counter = 0;
unsigned long lastCounterUpdate = 0;

void setup() {
	Serial.begin(115200);

	// You must call logHandler.setup() from setup()!
	logHandler.setup();
}

void loop() {
	// You must call logHandler.loop() from loop()!
	logHandler.loop();

	if (millis() - lastCounterUpdate >= 10000) {
		lastCounterUpdate = millis();
		Log.info("testing counter=%d", counter++);
	}
}
