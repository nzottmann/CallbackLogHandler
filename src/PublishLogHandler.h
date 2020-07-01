#ifndef __PUBLISHLOGHANDLER_H
#define __PUBLISHLOGHANDLER_H

#include "Particle.h"
#include "RingBuffer.h"

#include <set>


/**
 * @brief Class for writing a data stream to SD card
 *
 * You normally instantiate one of these as a global variable, passing in the SdFat object and the parameters
 * you'd normally pass to SdFat::begin().
 *
 * ~~~~{.c}
 * const int SD_CHIP_SELECT = A2;
 * SdFat sd;
 *
 * PublishLogHandler logHandler(sd, SD_CHIP_SELECT, SPI_FULL_SPEED);
 * ~~~~
 *
 * You can pass additional options using the fluent-style methods beginning with "with" like withLogsDirName().
 *
 * This class is a subclass of Print, so you can use all of the overloads of print, println, and printf that are supported
 * by Print. Data is buffered until the \n, then written to the card, for performance reasons and to avoid splitting
 * a line between multiple files.
 */
class PublishPrintHandler : public Print {
public:
	PublishPrintHandler();
	virtual ~PublishPrintHandler();

	/**
	 * @brief Set whether to sync the file system after every log entry. Default: true
	 *
	 * Setting this to false dramatically improves the performance, but it also makes it much more likely that
	 * in the case of a reboot, the last log messages will be lost. The SdFat library normally only flushes the
	 * file in 512 byte increments so if you log infrequently, you could lose a number of log messages.
	 *
	 * @param value The value to set (size_t)
	 */
	inline PublishPrintHandler &withSyncEveryEntry(size_t value) { syncEveryEntry = value; return *this; };

	/**
	 * @brief The default is to log to Serial as well as SD card; to only log to SD card call this method.
	 *
	 * If you want to log to a different stream (like Serial1), use withWriteToStream() instead.
	 */
	inline PublishPrintHandler &withNoSerialLogging() { writeToStream = NULL; return *this; };

	/**
	 * @brief Write to a different Stream, such as Serial1. Default: Serial
	 *
	 * @param value The stream to write log output to (such as &Serial1) or NULL to only write to the SD card.
	 *
	 * Only one stream is supported. Setting it again replaces the last setting.
	 */
	inline PublishPrintHandler &withWriteToStream(Stream *value) { writeToStream = value; return *this; };


	/**
	 * @brief Virtual override for the StreamLogHandler to write data to the log
	 */
    virtual size_t write(uint8_t);

private:
    static const size_t BUF_SIZE = 622;  //!< size of buf[], the buffer to hold log messages. This improves write performance. Logs messages can be bigger than this.

    /**
     * Writes the current buffer in buf of length bufOffset to the SD card then resets the bufOffset to 0
     *
     * If writeToStream is non-null, its write method is called to write out the buffer as well. This
     * default to &Serial, but you can set it to &Serial1, or other streams.
     */
    void writeBuf();

    const char *logsDirName = "logs"; //!< Name of the logs directory, override using withLogsDirName()
    bool syncEveryEntry = true; //!< Whether to sync the filesystem after each log entry. Override using withSyncEveryEntry().
    Stream *writeToStream = NULL; //!< Write to another Stream in addition to SD, override using withWriteToStream().

    size_t bufOffset = 0; //!< Offset we're currently writing to in buf
    uint8_t buf[BUF_SIZE];  //!< Buffer to hold partial log message.
};


/**
 * @brief Class for logging to SD card
 *
 * You normally instantiate one of these as a global variable, passing in the SdFat object and the parameters
 * you'd normally pass to SdFat::begin(). You can optionally pass the LogLevel and LogCategoryFilters parameters
 * you'd pass to the LogHandler constructor.
 *
 * ~~~~{.c}
 * const int SD_CHIP_SELECT = A2;
 * SdFat sd;
 *
 * PublishLogHandler logHandler(sd, SD_CHIP_SELECT, SPI_FULL_SPEED);
 * ~~~~
 *
 * You can pass additional options using the fluent-style methods beginning with "with" like withLogsDirName().
 */
class PublishLogHandlerBuffer : public StreamLogHandler, public PublishPrintHandler, public RingBuffer<uint8_t> {
public:
	/**
	 * @brief Constructor. The object is normally instantiated as a global object.
	 *
	 * @param buf Ring buffer pointer
	 * @param bufSize Ring buffer size
	 * @param sd The SdFat object, normally allocated a global object.
	 * @param csPin The pin used for the SPI chip select for the SD card reader
	 * @param spiSettings Usually either SPI_FULL_SPEED or SPI_HALF_SPEED. You can also use a SPISettings object.
	 * @param level  (optional, default is LOG_LEVEL_INFO)
	 * @param filters (optional, default is none)
	 */
	PublishLogHandlerBuffer(uint8_t *buf, size_t bufSize, LogLevel level = LOG_LEVEL_INFO, LogCategoryFilters filters = {});
	virtual ~PublishLogHandlerBuffer();

	/**
	 * @brief Must be called from setup (added in 0.0.6)
	 *
	 * On mesh devices, it's not safe to set up the log handler at global object construction time and you will likely
	 * fault.
	 */
	void setup();

    /**
     * @brief Must be called from loop (added in 0.1.0)
     *
     * This method must be called from loop(), ideally on every call to loop. The reason is
     * that it's not really safe to call SPI from the log handler, and it's best to buffer the data and
     * write it out from loop to avoid SPI conflicts.
     */
    void loop();

	/**
	 * @brief Virtual override for the StreamLogHandler to write data to the log
	 */
    virtual size_t write(uint8_t);

protected:

};

template<size_t BUFFER_SIZE>
class PublishLogHandler : public PublishLogHandlerBuffer {
public:
	/**
	 * @brief Constructor. The object is normally instantiated as a global object.
	 *
	 * @param sd The SdFat object, normally allocated a global object.
	 * @param csPin The pin used for the SPI chip select for the SD card reader
	 * @param spiSettings Usually either SPI_FULL_SPEED or SPI_HALF_SPEED. You can also use a SPISettings object.
	 * @param level  (optional, default is LOG_LEVEL_INFO)
	 * @param filters (optional, default is none)
	 */
	explicit PublishLogHandler(LogLevel level = LOG_LEVEL_INFO, LogCategoryFilters filters = {}) :
		PublishLogHandlerBuffer(ringBuffer, sizeof(ringBuffer), level, filters) {};

protected:
	uint8_t ringBuffer[BUFFER_SIZE];
};


#endif /* __PUBLISHLOGHANDLER_H */
