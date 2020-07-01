#include "Particle.h"

#include "PublishLogHandler.h"

// Define the debug logging level here
// 0 = Off
// 1 = Normal
// 2 = High
#define PUBLISH_LOGHANDLER_DEBUG_LEVEL 1

// Don't change these, just change the debugging level above
// Note: must use Serial.printlnf here, not Log.info, as these are called from the log handler itself!
#if PUBLISH_LOGHANDLER_DEBUG_LEVEL >= 1
#define DEBUG_NORMAL(x) Serial.printlnf x
#else
#define DEBUG_NORMAL(x)
#endif

#if PUBLISH_LOGHANDLER_DEBUG_LEVEL >= 2
#define DEBUG_HIGH(x) Serial.printlnf x
#else
#define DEBUG_HIGH(x)
#endif




//
//
//

PublishLogHandlerBuffer::PublishLogHandlerBuffer(uint8_t *buf, size_t bufSize, LogLevel level, LogCategoryFilters filters) :
	StreamLogHandler(*this, level, filters), RingBuffer(buf, bufSize) {

	// This was the old default for PublishLogHandler. The subclass PublishPrintHandler now defaults to NULL.
	withWriteToStream(&Serial);
}

PublishLogHandlerBuffer::~PublishLogHandlerBuffer() {
}

void PublishLogHandlerBuffer::setup() {
	// Add this handler into the system log manager
	LogManager::instance()->addHandler(this);
}

void PublishLogHandlerBuffer::loop() {

	while(true) {
		uint8_t c;

		bool bResult = RingBuffer::read(&c);
		if (!bResult) {
			break;
		}
		PublishPrintHandler::write(c);
	}
}


size_t PublishLogHandlerBuffer::write(uint8_t c) {

	return RingBuffer::write(&c) ? 1 : 0;
}

//
//
//
PublishPrintHandler::PublishPrintHandler(){
}

PublishPrintHandler::~PublishPrintHandler() {

}


size_t PublishPrintHandler::write(uint8_t c) {

	buf[bufOffset++] = c;
	if (bufOffset >= BUF_SIZE || c == '\n') {
		// Buffer is full or have the LF in CRLF, write it out
		writeBuf();
	}

	return 1;
}



void PublishPrintHandler::writeBuf() {

	if (writeToStream) {
		writeToStream->write(buf, bufOffset);
	}

	if (Particle.connected()) {
		// Terminate log message
		buf[min(bufOffset, BUF_SIZE-1)] = '\0';

		// Publish log message
		Particle.publish("log", (char*)buf, PRIVATE);
	}

	// Start over at beginning of buffer
	bufOffset = 0;
}

