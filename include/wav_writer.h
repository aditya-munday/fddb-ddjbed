#pragma once

#include <string>
#include <fstream>
#include <cstdint>

/**
 * @brief WAV file writer for PCM audio recording
 * 
 * Writes audio data to a valid WAV file format with the following specs:
 * - Sample Rate: 48,000 Hz
 * - Bit Depth: 16-bit
 * - Channels: Stereo
 * - Encoding: PCM
 */
class WavWriter {
public:
    /**
     * @brief Construct a new WavWriter
     * @param filename Output WAV file path
     * @param sampleRate Sample rate in Hz (default: 48000)
     * @param channels Number of audio channels (default: 2 for stereo)
     * @param bitsPerSample Bits per sample (default: 16)
     */
    WavWriter(const std::string& filename, 
              uint32_t sampleRate = 48000, 
              uint16_t channels = 2, 
              uint16_t bitsPerSample = 16);
    
    ~WavWriter();

    // Prevent copying
    WavWriter(const WavWriter&) = delete;
    WavWriter& operator=(const WavWriter&) = delete;

    // Allow moving
    WavWriter(WavWriter&& other) noexcept;
    WavWriter& operator=(WavWriter&& other) noexcept;

    /**
     * @brief Write PCM audio data to the file
     * @param data Pointer to audio data
     * @param size Number of bytes to write
     * @return true if write succeeded, false otherwise
     */
    bool write(const uint8_t* data, size_t size);

    /**
     * @brief Write interleaved stereo PCM data
     * @param leftChannel Left channel samples
     * @param rightChannel Right channel samples
     * @param frameCount Number of frames (samples per channel)
     * @return true if write succeeded, false otherwise
     */
    bool writeStereo(const int16_t* leftChannel, const int16_t* rightChannel, size_t frameCount);

    /**
     * @brief Finalize the WAV file (write headers)
     * @return true if finalization succeeded, false otherwise
     */
    bool finalize();

    /**
     * @brief Check if the file is open and ready
     * @return true if file is open
     */
    bool isOpen() const;

    /**
     * @brief Get the total bytes written (audio data only)
     * @return Total audio bytes written
     */
    uint64_t getBytesWritten() const;

    /**
     * @brief Get the recording duration in seconds
     * @return Duration in seconds
     */
    double getDuration() const;

private:
    void writeHeader();

    std::ofstream m_file;
    std::string m_filename;
    uint32_t m_sampleRate;
    uint16_t m_channels;
    uint16_t m_bitsPerSample;
    uint64_t m_dataSize;
    bool m_finalized;
};
