#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <iostream>
#include <istream>
#include <set>


constexpr size_t WAV_HEADER_DEFAULT_SIZE = 44u;
//! Wave header
/*!
 Reads and stores the wave header in the same way it appears in the WAV file.
*/
struct WAV_HEADER
{
	/*!
	* Read the first 44 bytes of the input stream into the header.
	*/
	bool read(std::istream& s)
	{
		if (s.good())
		{
			s.seekg(0u);
			s.read(&m_0_headerChunkID[0], 4);
			s.read((char*)&m_4_chunkSize, 4);
			s.read(&m_8_format[0], 4);
			s.read(&m_12_subchunk1ID[0], 4);
			s.read((char*)&m_16_subchunk1Size, 4);
			s.read((char*)&m_20_audioFormat, 2);
			s.read((char*)&m_22_numChannels, 2);
			s.read((char*)&m_24_sampleRate, 4);
			s.read((char*)&m_28_byteRate, 4);
			s.read((char*)&m_32_bytesPerBlock, 2);
			s.read((char*)&m_34_bitsPerSample, 2);
			s.read(&m_36_dataSubchunkID[0], 4);
			s.read((char*)&m_40_dataSubchunkSize, 4);
		}
		return s.good();
	}
	/*!
	* Checks whether the header is in a format that can be read by waveread
	*/
	bool valid() const
	{
		return (std::string{ &m_0_headerChunkID[0],4u } == std::string{ "RIFF" }) && // RIFF 
			std::string{ &m_8_format[0],4u } == std::string{ "WAVE" } && // WAVE format
			m_16_subchunk1Size == 16 &&// PCM, with no extra parameters in file
			m_20_audioFormat == 1 && // uncompressed
			m_32_bytesPerBlock == (m_22_numChannels * (m_34_bitsPerSample / 8)) && // block align matches # channels and bit depth
			(
				(m_34_bitsPerSample == 8) ||
				(m_34_bitsPerSample == 16) ||
				(m_34_bitsPerSample == 24) ||
				(m_34_bitsPerSample == 32) // available bit depths
				);
	}
	/*!
	* Clear all data in the header setting values to 0 or "nil\0"
	*/
	void clear()
	{
		auto cpy = [](char from[4], char to[4]) // since strcpy is deprecated on windows and strcpy_s absent on *nix.
		{
			for (size_t i{ 0u }; i < 4u; ++i)
				to[i] = from[i];
		};
		char none[4]{ "nil" };
		cpy(none, m_0_headerChunkID);
		m_4_chunkSize = 0;
		cpy(none, m_8_format);
		cpy(none, m_12_subchunk1ID);
		m_16_subchunk1Size = 0;
		m_20_audioFormat = 0;
		m_22_numChannels = 0;
		m_24_sampleRate = 0;
		m_28_byteRate = 0;
		m_32_bytesPerBlock = 0;
		m_34_bitsPerSample = 0;
		cpy(none, m_36_dataSubchunkID);
		m_40_dataSubchunkSize = 0;
	}
	/*!
	* Samples per channel
	*/
	int samples() const { return (m_40_dataSubchunkSize / ((m_22_numChannels * m_34_bitsPerSample) / 8)); }

	char m_0_headerChunkID[4]; /*!< Header chunk ID */
	int32_t m_4_chunkSize; /*!< Chunk size*/
	char m_8_format[4]; /*!< Format */

	char m_12_subchunk1ID[4]; /*!< Subchunk ID */
	int16_t m_16_subchunk1Size; /*!< Subchunk size*/

	int16_t m_20_audioFormat; /*!< Audio format */
	int16_t m_22_numChannels; /*!< Number of channels*/
	int32_t m_24_sampleRate; /*!< Sample rate of a single channel */
	int32_t m_28_byteRate; /*!< Number of bytes per sample*/
	int16_t m_32_bytesPerBlock; /*!< Number of bytes per block (where a block is a single sample from each channel)*/
	int16_t m_34_bitsPerSample; /*!< Bits per sample */

	char m_36_dataSubchunkID[4]; /*!< Detailed description after the member */
	int32_t m_40_dataSubchunkSize; /*!< Detailed description after the member */
};
static_assert(sizeof(WAV_HEADER) == WAV_HEADER_DEFAULT_SIZE, "WAV File header is not the expected size.");

//! Wave reader
/*!
  Reads audio from an input stream.
*/
class Waveread
{
public:
	//! Constructor
	/*!
	* \param stream the input stream
	* \param cacheSize the size of the cache. This should usually be a reasonable multiple of the size of the set of samples you expect to read each time you call audio().
	* \param cacheExtensionThreshold Within interval [0,1]. When a caller gets audio, how far into the cache should the caller go before the cache is triggered to be extended?
	*/
	Waveread(
		std::unique_ptr<std::istream>&& stream,
		size_t cacheSize = 1048576u,
		double cacheExtensionThreshold = 0.5
	)
		:
		m_stream{ stream.release() },
		m_header{},
		m_data{},
		m_dataMutex{},
		m_cachePos{ 0u },
		m_opened{ false },
		m_cacheSize{ cacheSize }, // 1MB == 1048576u
		m_cacheExtensionThreshold{ cacheExtensionThreshold }
	{
		if (m_cacheExtensionThreshold < 0.0)
			m_cacheExtensionThreshold = 0.0;
		else if (m_cacheExtensionThreshold > 1.0)
			m_cacheExtensionThreshold = 1.0;

		m_header.clear();
	}

	Waveread(const Waveread&) = delete;
	Waveread& operator=(const Waveread&) = delete;

	//! Move Constructor
	/*!
	* \param other	Another waveread object. The move constructor enables the placement of waveread objects in containers using std::move().
	*				For instance you can do:
	*				Waveread a{stream};
	*				std::vector<Waveread> readers;
	*				readers.push_back(std::move(a))
	*				a is now unusable, but the vector now contains the wavereader.
	*/
	Waveread(Waveread&& other) noexcept
		:
		m_stream{ },
		m_header{ },
		m_data{ },
		m_dataMutex{},
		m_cachePos{ other.m_cachePos },
		m_opened{ other.m_opened },
		m_cacheSize{ other.m_cacheSize },
		m_cacheExtensionThreshold{ other.m_cacheExtensionThreshold }
	{
		std::lock_guard<std::mutex> l{ other.m_dataMutex };
		m_data = other.m_data;
		m_header = other.m_header;
		m_stream.reset(other.m_stream.release());
	}
	//! Reset
	/*!
	* Resets the wavereader, clearing all data.
	* \param stream a new std::istream to read a wave file from.
	*/
	void reset(std::unique_ptr<std::istream>&& stream)
	{
		std::lock_guard<std::mutex> lock{ m_dataMutex };
		m_stream = std::move(stream);
		m_data.clear();
		m_header.clear();
		m_cachePos = 0u;
		m_opened = false;
	}
	//! Open
	/*!
	* Loads the wave header from file, and fills the cache from the start.
	*/
	bool open()
	{
		if (!m_opened)
		{
			m_header.read(*m_stream.operator->());
			if (m_header.valid())
			{
				m_opened = true;
				load(0u, m_cacheSize);
				return true;
			}
			else
				return false; // we couldn't open it
		}
		return true; // we didn't open it, but it was already opened.
	}
	//! Close
	/*!
	* Closes the wavereader.
	*/
	void close()
	{
		std::lock_guard<std::mutex> lock{ m_dataMutex };
		m_stream->seekg(0u);
		m_data.clear();
		m_header.clear();
		m_cachePos = 0u;
		m_opened = false;
	}
	//! Audio
	/*!
	* Get interleaved floating point audio samples in the interval (-1.f,1.f).
	* \param startSample index of first sample desired
	* \param sampleCount number of samples needed including first sample
	* \param channels Which channels would you like to retrieve. Zero-indexed. If channels are out of bounds, then their modulus with the channel count will be taken. This means if you ask for channels {0,1} from a mono file, you will retrieve two copies of the mono channel, interleaved.
	* \param stride for each channel, when getting samples, skip every n samples where n == stride.
	* \param interleaved determines how samples are ordered: true provides {C1S1, C2S1, ..., CMS1, C1S2, C2S2, ..., CMS2} false provides {C1S1, C1S2, ..., C1SN, C2S1, C2S2, ..., C2S2, ...}
	*/
	std::vector<float> audio(
		size_t startSample, 
		size_t sampleCount, 
		std::set<int> channels = std::set<int>{ 0,1 }, 
		size_t stride = 0u, 
		bool interleaved = true
	)
	{
		if (!open())
			return std::vector<float>{};

		size_t startSample_ch_bit{ startSample * m_header.m_32_bytesPerBlock };
		size_t sampleCount_ch_bit{ sampleCount * m_header.m_32_bytesPerBlock };

		if ((startSample_ch_bit + sampleCount_ch_bit) >= (size_t)m_header.m_40_dataSubchunkSize)	// case1: out of bounds of file
		{
			if (startSample_ch_bit >= (size_t)m_header.m_40_dataSubchunkSize)						// case1A: read starts out of bounds
				return std::vector<float>{};
			else																					// case1B: read starts within bounds, ends out of bounds
			{
				load(startSample_ch_bit, m_header.m_40_dataSubchunkSize - startSample_ch_bit);
				return samples(0u, m_header.m_40_dataSubchunkSize - startSample_ch_bit, channels, stride, interleaved);
			}
		}
		else if (startSample_ch_bit >= m_cachePos &&
			(startSample_ch_bit + sampleCount_ch_bit) <= (m_cachePos + m_data.size()))				// case2: within cache
		{
			std::vector<float> result{ samples(startSample_ch_bit - m_cachePos, sampleCount_ch_bit, channels, stride,interleaved) };
			if (startSample_ch_bit > (m_cachePos + (size_t)(m_data.size() * 0.5)))					// case2A: approaching end of cache
			{
				std::thread extendBuffer{ &Waveread::load,this,m_cachePos + (size_t)(m_cacheSize * 0.5), m_cacheSize };
				extendBuffer.detach();
			}
			return result;
		}
		else																						// case3: within file, outside of cache
		{
			if (load(startSample_ch_bit, m_cacheSize > sampleCount_ch_bit ? m_cacheSize : sampleCount_ch_bit)) // load samplecount or cachesize, whichever is greater.
				return samples(0u, sampleCount_ch_bit, channels, stride, interleaved);
			else
				return std::vector<float>{};
		}
	}

	//! Get header file
	const WAV_HEADER& header() const { return m_header; }
	//! Get size of caches
	const size_t& cacheSize() const { return m_cacheSize; }
	//! Get start position of cache
	const size_t& cachePos() const { return m_cachePos; }
	//! Has the file been opened
	const bool& opened() const { return m_opened; }
	//! Get cache extension threshold: this is the fraction of the cache that is read before it is extended.
	const double& cacheExtensionThreshold() const { return m_cacheExtensionThreshold; }


	//! Set cache extension threshold. Does not extend the cache until audio() has been called. Function will halt until the last load operation has finished.
	void setCacheExtensionThreshold(const double& cacheExtensionThreshold)
	{
		std::lock_guard<std::mutex> l{ m_dataMutex };
		m_cacheExtensionThreshold = cacheExtensionThreshold;
	}
	//! Set cache size. Does not extend the cache until audio() has been called. Function will halt until the last load operation has finished.
	void setCacheSize(const size_t& csize)
	{
		std::lock_guard<std::mutex> l{ m_dataMutex };
		m_cacheSize = csize;
	}
private:
	//! Load data into the cache
	bool load(size_t pos, size_t size) // method will offset read by header size
	{
		std::lock_guard<std::mutex> lock{ m_dataMutex };
		size_t truncatedSize{ (pos + size) < (size_t)m_header.m_40_dataSubchunkSize
			? size : (size_t)m_header.m_40_dataSubchunkSize - pos };
		if (pos < (size_t)m_header.m_40_dataSubchunkSize)
		{
			m_stream->seekg(((std::streampos)pos + (std::streampos)sizeof(WAV_HEADER))); // add header size
			m_data.resize(truncatedSize);
			if (m_stream->good())
			{
				m_stream->read(reinterpret_cast<char*>(&m_data[0]), truncatedSize);
				m_cachePos = pos;
				m_stream->clear(std::iostream::eofbit);
				return m_stream->good();
			}
		}
		return false;
	}
	//! Transform cached bytes into floats.
	/*!
	* \param posInCache
	* \param size
	* \param channels
	* \param stride
	* \param interleaved
	*/
	std::vector<float> samples(
		size_t posInCache,
		size_t size,
		std::set<int> channels = std::set<int>{},
		size_t stride = 0u,
		bool interleaved = true) // posInCache is pos relative to cachepos.
	{
		std::vector<float> result{};
		if (
			(posInCache + size) <= m_data.size() &&			// if caller is not overshooting the cache
			!channels.empty() 								// if caller has provided channels
			)
		{
			std::lock_guard<std::mutex> lock{ m_dataMutex };
			size_t bpc{ m_header.m_32_bytesPerBlock / (size_t)m_header.m_22_numChannels }; // bytes per channel

			if (interleaved)
				switch (m_header.m_34_bitsPerSample)
				{
				case 8: // unsigned 8-bit
					for (size_t i{ posInCache }; i < (posInCache + size); i += (m_header.m_32_bytesPerBlock * (1u + stride)))
					{
						for (auto ch : channels)
						{
							// NOTE: (a) see narrow_cast<T>(var) (b) addition defined in C++ as: T operator+(const T &a, const T2 &b);
							// EXCEPTIONS: Integer types smaller than int are promoted when an operation is performed on them.
							size_t cho{ (ch % m_header.m_22_numChannels) * bpc };
							result.emplace_back((float)
								(m_data[i + cho] - 128)  // unsigned, so offset by 2^7
								/ (128.f)); // divide by 2^7
						}
					}
					break;
				case 16: // signed 16-bit
					for (size_t i{ posInCache }; i < (posInCache + size); i += (m_header.m_32_bytesPerBlock * (1u + stride)))
					{
						for (auto ch : channels)
						{
							size_t cho{ (ch % m_header.m_22_numChannels) * bpc };
							result.emplace_back((float)
								((m_data[i + cho]) |
									(m_data[i + 1u + cho] << 8))
								/ (32768.f)); // divide by 2^15
						}
					}
					break;
				case 24: // signed 24-bit
					for (size_t i{ posInCache }; i < (posInCache + size); i += (m_header.m_32_bytesPerBlock * (1u + stride)))
					{
						for (auto ch : channels)
						{
							size_t cho{ (ch % m_header.m_22_numChannels) * bpc };
							// 24-bit is different to others: put the value into a 32-bit int with zeros at the (LSB) end
							result.emplace_back((float)
								((m_data[i + cho] << 8) |
									(m_data[i + 1u + cho] << 16) |
									(m_data[i + 2u + cho] << 24))
								/ (2147483648.f)); // divide by 2^31
						}
					}
					break;
				case 32: // signed 32-bit
					for (size_t i{ posInCache }; i < (posInCache + size); i += (m_header.m_32_bytesPerBlock * (1u + stride)))
					{
						for (auto ch : channels)
						{
							size_t cho{ (ch % m_header.m_22_numChannels) * bpc };
							result.emplace_back((float)
								(m_data[i + cho] |
									(m_data[i + 1u + cho] << 8) |
									(m_data[i + 2u + cho] << 16) |
									(m_data[i + 3u + cho] << 24))
								/ (2147483648.f));  // signed, so divide by 2^31
						}
					}
					break;
				default:
					break;
				}
			else
				switch (m_header.m_34_bitsPerSample)
				{
				case 8: // unsigned 8-bit
					for (auto ch : channels)
					{
						size_t cho{ (ch % m_header.m_22_numChannels) * bpc };
						for (size_t i{ posInCache }; i < (posInCache + size); i += (m_header.m_32_bytesPerBlock * (1u + stride)))
						{
							// NOTE: (a) see narrow_cast<T>(var) (b) addition defined in C++ as: T operator+(const T &a, const T2 &b);
							// EXCEPTIONS: Integer types smaller than int are promoted when an operation is performed on them.
							result.emplace_back((float)
								(m_data[i + cho] - 128)  // unsigned, so offset by 2^7
								/ (128.f)); // divide by 2^7
						}
					}
					break;
				case 16: // signed 16-bit
					for (auto ch : channels)
					{
						size_t cho{ (ch % m_header.m_22_numChannels) * bpc };
						for (size_t i{ posInCache }; i < (posInCache + size); i += (m_header.m_32_bytesPerBlock * (1u + stride)))
						{
							result.emplace_back((float)
								((m_data[i + cho]) |
									(m_data[i + 1u + cho] << 8))
								/ (32768.f)); // divide by 2^15
						}
					}
					break;
				case 24: // signed 24-bit
					for (auto ch : channels)
					{
						size_t cho{ (ch % m_header.m_22_numChannels) * bpc };
						for (size_t i{ posInCache }; i < (posInCache + size); i += (m_header.m_32_bytesPerBlock * (1u + stride)))
						{
							// 24-bit is different to others: put the value into a 32-bit int with zeros at the (LSB) end
							result.emplace_back((float)
								((m_data[i + cho] << 8) |
									(m_data[i + 1u + cho] << 16) |
									(m_data[i + 2u + cho] << 24))
								/ (2147483648.f)); // divide by 2^31
						}
					}
					break;
				case 32: // signed 32-bit
					for (auto ch : channels)
					{
						size_t cho{ (ch % m_header.m_22_numChannels) * bpc };
						for (size_t i{ posInCache }; i < (posInCache + size); i += (m_header.m_32_bytesPerBlock * (1u + stride)))
						{
							result.emplace_back((float)
								(m_data[i + cho] |
									(m_data[i + 1u + cho] << 8) |
									(m_data[i + 2u + cho] << 16) |
									(m_data[i + 3u + cho] << 24))
								/ (2147483648.f));  // signed, so divide by 2^31
						}
					}
					break;
				default:
					break;
				}
		}
		return result;
	}

	bool m_opened; /*!< Has the file been opened */
	std::unique_ptr<std::istream> m_stream; /*!< Input stream */

	WAV_HEADER m_header; /*!< Holds structure of header when opened, used in subsequent operations. */
	std::vector<uint8_t> m_data;/*!< Cached data holding part of the data chunk of the WAV file. */
	std::mutex m_dataMutex; /*!< Mutex to lock data when buffer is being extended */
	size_t m_cachePos; /*!< At what point, from the start of the data chunk (i.e. cachePos == idx - 44u), does the cached data in m_data begin at. */
	size_t m_cacheSize; /*!< How big should the cache (all channels) be in bytes */
	double m_cacheExtensionThreshold; /*!< Within interval [0,1]. When a caller gets audio, how far into the cache should the caller go before the cache is triggered to be extended? */
};
