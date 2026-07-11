#include "wav_writer.h"
#include <cstring>
#include <vector>

// WAV file header structures
#pragma pack(push, 1)
struct WavHeader {
    // RIFF header
    char riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t fileSize;
    char wave[4] = {'W', 'A', 'V', 'E'};
    
    // fmt subchunk
    char fmt[4] = {'f', 'm', 't', ' '};
    uint32_t fmtSize = 16;
    uint16_t audioFormat = 1;  // PCM
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    
    // data subchunk
    char data[4] = {'d', 'a', 't', 'a'};
    uint32_t dataSize;
};
#pragma pack(pop)

WavWriter::WavWriter(const std::string& filename, 
                     uint32_t sampleRate, 
                     uint16_t channels, 
                     uint16_t bitsPerSample)
    : m_filename(filename)
    , m_sampleRate(sampleRate)
    , m_channels(channels)
    , m_bitsPerSample(bitsPerSample)
    , m_dataSize(0)
    , m_finalized(false)
{
    m_file.open(m_filename, std::ios::binary | std::ios::trunc);
    if (m_file.is_open()) {
        // Write placeholder header
        writeHeader();
    }
}

WavWriter::~WavWriter() {
    if (m_file.is_open()) {
        if (!m_finalized) {
            finalize();
        }
        m_file.close();
    }
}

WavWriter::WavWriter(WavWriter&& other) noexcept
    : m_file(std::move(other.m_file))
    , m_filename(std::move(other.m_filename))
    , m_sampleRate(other.m_sampleRate)
    , m_channels(other.m_channels)
    , m_bitsPerSample(other.m_bitsPerSample)
    , m_dataSize(other.m_dataSize)
    , m_finalized(other.m_finalized)
{
    other.m_finalized = true;
}

WavWriter& WavWriter::operator=(WavWriter&& other) noexcept {
    if (this != &other) {
        if (m_file.is_open()) {
            m_file.close();
        }
        m_file = std::move(other.m_file);
        m_filename = std::move(other.m_filename);
        m_sampleRate = other.m_sampleRate;
        m_channels = other.m_channels;
        m_bitsPerSample = other.m_bitsPerSample;
        m_dataSize = other.m_dataSize;
        m_finalized = other.m_finalized;
        other.m_finalized = true;
    }
    return *this;
}

bool WavWriter::write(const uint8_t* data, size_t size) {
    if (!m_file.is_open() || m_finalized) {
        return false;
    }
    
    m_file.write(reinterpret_cast<const char*>(data), size);
    m_dataSize += size;
    return m_file.good();
}

bool WavWriter::writeStereo(const int16_t* leftChannel, const int16_t* rightChannel, size_t frameCount) {
    if (!m_file.is_open() || m_finalized) {
        return false;
    }

    // Interleave stereo samples
    std::vector<int16_t> interleaved(frameCount * 2);
    for (size_t i = 0; i < frameCount; ++i) {
        interleaved[i * 2] = leftChannel[i];
        interleaved[i * 2 + 1] = rightChannel[i];
    }

    size_t bytes = frameCount * 2 * sizeof(int16_t);
    m_file.write(reinterpret_cast<const char*>(interleaved.data()), bytes);
    m_dataSize += bytes;
    return m_file.good();
}

bool WavWriter::finalize() {
    if (!m_file.is_open() || m_finalized) {
        return false;
    }

    // Update header with actual sizes
    m_file.seekp(0, std::ios::beg);
    writeHeader();
    m_file.flush();
    m_finalized = true;
    
    return m_file.good();
}

bool WavWriter::isOpen() const {
    return m_file.is_open() && !m_finalized;
}

uint64_t WavWriter::getBytesWritten() const {
    return m_dataSize;
}

double WavWriter::getDuration() const {
    if (m_sampleRate == 0 || m_channels == 0 || m_bitsPerSample == 0) {
        return 0.0;
    }
    
    uint32_t bytesPerSample = m_bitsPerSample / 8;
    uint32_t samplesPerSecond = m_sampleRate * m_channels * bytesPerSample;
    
    if (samplesPerSecond == 0) {
        return 0.0;
    }
    
    return static_cast<double>(m_dataSize) / samplesPerSecond;
}

void WavWriter::writeHeader() {
    WavHeader header;
    
    header.numChannels = m_channels;
    header.sampleRate = m_sampleRate;
    header.bitsPerSample = m_bitsPerSample;
    
    // Calculate derived values
    uint16_t bytesPerSample = m_bitsPerSample / 8;
    header.blockAlign = m_channels * bytesPerSample;
    header.byteRate = m_sampleRate * header.blockAlign;
    
    // Update sizes (fileSize and dataSize will be filled after writing data)
    header.dataSize = static_cast<uint32_t>(m_dataSize);
    header.fileSize = 36 + m_dataSize;  // Total file size minus 8 bytes for RIFF+fileSize
    
    m_file.write(reinterpret_cast<const char*>(&header), sizeof(header));
}
