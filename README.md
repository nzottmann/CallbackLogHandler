# CallbackLogHandler

*Library for writing logs to custom callback on Particle Boards*

This library is based on the ring buffer implemented in SdCardLogHandlerRK [https://github.com/rickkas7/SdCardLogHandlerRK](https://github.com/rickkas7/SdCardLogHandlerRK).

It's in the Particle community libraries as: CallbackLogHandler.

It's also possible (as of version 0.0.5) to use this to write an arbitrary stream of data, not hooked into the logging API. See CallbackPrintHandler, below.


## Using the library

This library uses the [Logging API](https://docs.particle.io/reference/firmware/#logging) that was added in system firmware 0.6.0.

For example:

```
Log.trace("Low level debugging message");
Log.info("This is info message");
Log.warn("This is warning message");
Log.error("This is error message");

Log.info("System version: %s", System.version().c_str());
```

It does not remap Serial, so if you're using Serial.print for logging, you should switch to using Log.info instead.

CallbackLogHandler passes the log messages to a user defined callback which handles the message. The log messages are buffered and passed to the user defined callback from the main `loop()`. This allows using network or hardware functions in the log callback which is not recommended to do in the log handler itself.

Sample applications are (see examples):
- Logging to `Particle.publish()`
- Logging to papertrail

By default, CallbackLogHandler writes to Serial as well, like SerialLogHandler. This can be reconfigured.

This is the example program:

```
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

```

### Initialize the CallbackLogHandler

Normally you initialize the CallbackLogHandler by creating a global object like this:

```
CallbackLogHandler<2048, 622> logHandler(callback);
```

- `callback` is the callback: `void callback(uint8_t* buf, size_t length)`
- `2048` is the internal ring buffer size for all log messages
- `622` is the buffer size for a single log message passed to the callback.

There are also two optional parameters:

```
CallbackLogHandler<2048, 622> logHandler(callback, LOG_LEVEL_TRACE);
```

The next optional parameter allows you to set the logging level.

- `LOG_LEVEL_ALL` : special value that can be used to enable logging of all messages
- `LOG_LEVEL_TRACE` : verbose output for debugging purposes
- `LOG_LEVEL_INFO` : regular information messages
- `LOG_LEVEL_WARN` : warnings and non-critical errors
- `LOG_LEVEL_ERROR` : error messages
- `LOG_LEVEL_NONE` : special value that can be used to disable logging of any messages

And finally, you can use logging categories as well to set the level for certain categories:

```
CallbackLogHandler<2048, 622> logHandler(callback, LOG_LEVEL_INFO, {
	{ "app.network", LOG_LEVEL_TRACE } 
});
```

That example defaults to INFO but app.network events would be at TRACE level.

### Using CallbackPrintHandler

Instead of using CallbackLogHandler, you can use CallbackPrintHandler. This works like CallbackLogHandler, except it does not hook into the system log handler. This makes it useful for logging arbitrary data, and not have system logs mixed in.

```
uint8_t callbackBuffer[622];
size_t callbackBufferSize = sizeof(callbackBuffer);
CallbackPrintHandler logToCallback(logCallback, callbackBuffer, callbackBufferSize);
```

You can use any of the print, println, or printf methods to print to the log file. The same circular log file structure is used, and the card is only written to after a \n or println. A line is never broken up across multiple files.

```
logToCallback.println("testing!");
```

### Options

The options below work with both CallbackLogHandler and CallbackPrintHandler.

There are also options available to control how logging is done. For example, if you want the callback to be called for each split part of a log message longer than the callback buffer (per default, the first callback is called and all following are discarded):

```
CallbackLogHandler logHandler<2048, 622>(callback, LOG_LEVEL_TRACE);
STARTUP(logHandler.withSplitEntries(true));
```

You can chain these together fluent-style to change multiple settings:

```
CallbackLogHandler logHandler<2048, 622>(callback, LOG_LEVEL_TRACE);
STARTUP(logHandler.withSplitEntries(true).withNoSerialLogging());
```

By default, CallbackLogHandler writes the log entries to Serial like SerialLogHandler. You can turn this off by using:

```
CallbackLogHandler logHandler<2048, 622>(callback, LOG_LEVEL_TRACE);
STARTUP(logHandler.withNoSerialLogging());
```

Or if you want to log to Serial1 (or another port) instead:

```
CallbackLogHandler logHandler<2048, 622>(callback, LOG_LEVEL_TRACE);
STARTUP(logHandler.withWriteToStream(&Serial1);
```
