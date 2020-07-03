#include "Particle.h"

#include "CallbackLogHandler.h"

SYSTEM_THREAD(ENABLED);

void callback(uint8_t* buf, size_t length) {
    const String m_host = "logs7.papertrailapp.com";
    const uint16_t m_port = 49665;
    const String m_app = "example";
    const String m_system = System.deviceID();
    const uint16_t kLocalPort = 8888;
    static UDP m_udp;
    static IPAddress m_address;
    static bool m_inited = false;

    if (!m_inited) {
        m_inited = m_udp.begin(kLocalPort) != 0;
        if (!m_inited) return;
    }
    if (!m_address) {
        #if Wiring_WiFi
            m_address = WiFi.resolve(m_host);
        #elif Wiring_Cellular
            m_address = Cellular.resolve(m_host);
        #else
            #error Unsupported plaform
        #endif
        if (!m_address) return;
    }

    String time = Time.format(Time.now(), TIME_FORMAT_ISO8601_FULL);
    String packet = String::format("<22>1 %s %s %s - - - %s", time.c_str(), m_system.c_str(), m_app.c_str(), buf);
    int ret = m_udp.sendPacket(packet, packet.length(), m_address, m_port);
    if (ret < 1) m_inited = false;
}

CallbackLogHandler<2048, 128> logHandler(callback);

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
