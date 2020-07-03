#ifndef __CALLBACKLOGHANDLER_H
#define __CALLBACKLOGHANDLER_H

#include "Particle.h"
#include "RingBuffer.h"

#include <set>


/**
 * @brief Class for writing a data stream
 *
 * You can pass additional options using the fluent-style methods beginning with "with" like withSplitEntries().
 *
 * This class is a subclass of Print, so you can use all of the overloads of print, println, and printf that are supported
 * by Print. Data is buffered until the \n, then written
 */
class CallbackPrintHandler : public Print {
public:
	CallbackPrintHandler(void (* logCallback)(uint8_t* buf, size_t length), uint8_t* callbackBuffer, size_t callbackBufferSize) : logCallback(logCallback), callbackBuffer(callbackBuffer), callbackBufferSize(callbackBufferSize) {};
	virtual ~CallbackPrintHandler();

	/**
	 * @brief Set whether to split entries exceeding the callbackBufferSize. Default: false
	 *
	 * Log messages are written at a line break \n. If a log message exceeds
	 * callbackBufferSize, the leftover is discarded. If withSplitEntries is true, the callback is called
	 * for each part of the log message fitting in callbackBuffer
	 *
	 * @param value The value to set (bool)
	 */
	inline CallbackPrintHandler &withSplitEntries(bool value) { splitEntries = value; return *this; };

	/**
	 * @brief The default is to log to Serial as well as callback; to only log to callback call this method.
	 *
	 * If you want to log to a different stream (like Serial1), use withWriteToStream() instead.
	 */
	inline CallbackPrintHandler &withNoSerialLogging() { writeToStream = NULL; return *this; };

	/**
	 * @brief Write to a different Stream, such as Serial1. Default: Serial
	 *
	 * @param value The stream to write log output to (such as &Serial1) or NULL to only write to the callback.
	 *
	 * Only one stream is supported. Setting it again replaces the last setting.
	 */
	inline CallbackPrintHandler &withWriteToStream(Stream *value) { writeToStream = value; return *this; };


	/**
	 * @brief Virtual override for the StreamLogHandler to write data to the log
	 */
    virtual size_t write(uint8_t);

private:
    /**
     * Writes the current buffer in buf of length bufOffset to the callback, adds zero termination \0, then resets the bufOffset to 0
     *
     * If writeToStream is non-null, its write method is called to write out the buffer as well. This
     * default to &Serial, but you can set it to &Serial1, or other streams.
     */
    void writeBuf();

    bool splitEntries = false; //!< Whether to split entries not fitting in callbackBuffer over multiple callbacks. Override using withSplitEntries().
    Stream *writeToStream = NULL; //!< Write to another Stream in addition to callback, override using withWriteToStream().

    size_t bufOffset = 0; //!< Offset we're currently writing to in buf
	void (* logCallback)(uint8_t* buf, size_t length);
	uint8_t* callbackBuffer; //!< Buffer to hold partial log message.
	size_t callbackBufferSize; //!< size of callbackBuffer, the buffer to hold log messages. Logs messages can be bigger than this.
};


/**
 * @brief Class for logging
 *
 */
class CallbackLogHandlerBuffer : public StreamLogHandler, public CallbackPrintHandler, public RingBuffer<uint8_t> {
public:
	/**
	 * @brief Constructor. The object is normally instantiated as a global object.
	 *
	 * @param buf Ring buffer pointer
	 * @param bufSize Ring buffer size
	 * @param logCallback The user defined callback for logging
	 * @param callbackBuffer The buffer passed to logCallback, containing a single log message of part of it
	 * @param callbackBufferSize Size of callbackBuffer
	 * @param level  (optional, default is LOG_LEVEL_INFO)
	 * @param filters (optional, default is none)
	 */
	CallbackLogHandlerBuffer(uint8_t *buf, size_t bufSize, void (* logCallback)(uint8_t* buf, size_t length), uint8_t* callbackBuffer, size_t callbackBufferSize, LogLevel level = LOG_LEVEL_INFO, LogCategoryFilters filters = {});
	virtual ~CallbackLogHandlerBuffer();

	/**
	 * @brief Must be called from setup
	 *
	 * On mesh devices, it's not safe to set up the log handler at global object construction time and you will likely
	 * fault.
	 */
	void setup();

    /**
     * @brief Must be called from loop
     *
     * This method must be called from loop(), ideally on every call to loop. The reason is
     * that it's not really safe to call shared resources from the log handler, and it's best to buffer the data and
     * call the callback from loop to avoid conflicts.
     */
    void loop();

	/**
	 * @brief Virtual override for the StreamLogHandler to write data to the log
	 */
    virtual size_t write(uint8_t);

protected:

};

template<size_t BUFFER_SIZE, size_t CB_BUFFER_SIZE>
class CallbackLogHandler : public CallbackLogHandlerBuffer {
public:
	/**
	 * @brief Constructor. The object is normally instantiated as a global object.
	 *
	 * @param logCallback The user defined callback for logging
	 * @param level  (optional, default is LOG_LEVEL_INFO)
	 * @param filters (optional, default is none)
	 */
	explicit CallbackLogHandler(void (* logCallback)(uint8_t* buf, size_t length), LogLevel level = LOG_LEVEL_INFO, LogCategoryFilters filters = {}) :
		CallbackLogHandlerBuffer(ringBuffer, sizeof(ringBuffer), logCallback, callbackBuffer, CB_BUFFER_SIZE, level, filters) {};

protected:
	uint8_t ringBuffer[BUFFER_SIZE];
	uint8_t callbackBuffer[CB_BUFFER_SIZE]; 
};


#endif /* __CALLBACKLOGHANDLER_H */
