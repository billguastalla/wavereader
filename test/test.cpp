#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <waveread.hpp>
#include <array>

constexpr char assetPath[8] = "assets/";
constexpr std::array<char[255], 5> supportedFiles = {
	"8-bit_unsigned_Sine_Stereo.wav",
	"8-bit_Sine_Stereo.wav",
	"16-bit_signed_Sine_Stereo.wav",
	"24-bit_signed_Sine_Stereo.wav",
	"32-bit_signed_Sine_Stereo.wav",
};
constexpr std::array<char[255], 6> unsupportedFiles = {
	"32-bit_float_Sine_Stereo.wav",
	"64-bit_float_Sine_Stereo.wav",
	"A-Law_Sine_Stereo.wav",
	"IMA_ADPCM_Sine_Stereo.wav",
	"MS_ADPCM_Sine_Stereo.wav",
	"U-Law_Sine_Stereo.wav",
};


TEST_CASE("Check that unsupported files are stated as unsupported.")
{
	for (auto unsupported : unsupportedFiles)
	{
		std::string name{ assetPath + std::string{unsupported} };
		std::unique_ptr<std::istream> fileStream{ new std::ifstream{name} };
		Waveread waveReader{ fileStream,2048u,0.5};

		bool opened{ waveReader.open() };
		WAV_HEADER h{ waveReader.header() };

		REQUIRE(!opened);
	}
}

TEST_CASE("Check that all supported WAVE variants can load all data in file.")
{
	for (auto supported : supportedFiles)
	{
		std::string name{ assetPath + std::string{supported} };
		Waveread wr{ std::unique_ptr<std::istream>{new std::ifstream{name}} };
		REQUIRE(wr.open());
		
		std::vector<float> f{ wr.audio(0, std::numeric_limits<size_t>::max(), { 0,1 }) };
		std::cout << "name: " << name << "\t\tSample count: " << f.size() << std::endl;

		REQUIRE(f.size() == 882u);
	}

}

TEST_CASE("demo") // No assertions: just to check that demo in README.MD compiles.
{
	std::string name{ assetPath + std::string{supportedFiles[4]} };
	Waveread r{ std::unique_ptr<std::istream>{ new std::ifstream{name} } };

	// A: two channels: the first 128 samples of the first two channels of audio.
	std::vector<float> audio1{ r.audio(0u, 128u, { 0,1 }) };
	// B: one channel: get 128 samples of audio with stride 1: meaning, pick every other sample.
	std::vector<float> audio2{ r.audio(0u, 128u, {0}, 1u) };
}

TEST_CASE("Does a wavereader object moved into a container still produce audio data correctly?")
{
	for (auto supported : supportedFiles)
	{
		std::string name{ assetPath + std::string{supported} };
		Waveread wrA{ std::unique_ptr<std::istream>{ new std::ifstream{name} } };
		Waveread wrB{ std::move(wrA) };
		std::vector<float> aud{ wrB.audio(0, 128, { 0,1 }, 0u) };

		REQUIRE(aud.size() != 0u);

		// { Compiliation test: can we move the reader into a vector? }
		std::vector<Waveread> v;
		v.push_back(std::move(wrB));

		std::vector<float> aud2{ v.back().audio(0, 128, { 0,1 }, 0u) };
		REQUIRE(aud == aud2);
	}
}

TEST_CASE("Do adjacent and interleaved samples produce the same audio in a different arrangement?")
{
	for (auto supported : supportedFiles)
	{
		std::string name{ assetPath + std::string{supported} };
		Waveread wr{ std::unique_ptr<std::istream>{ new std::ifstream{name} } };

		size_t sampleCount{ 128u };

		std::vector<float> audioInterleaved{ wr.audio(sampleCount, sampleCount, { 0,1 }, 0u) };
		std::vector<float> audioAdjacent{ wr.audio(sampleCount, sampleCount, { 0,1 }, 0u,false) };

		REQUIRE(audioInterleaved.size() == audioAdjacent.size());
		REQUIRE(audioInterleaved.size() == (sampleCount * 2));

		std::vector<float> lh{}, rh{};
		bool samplesSame{ true };
		for (size_t i{ 0 }; i < audioInterleaved.size(); i += 2u)
		{
			samplesSame &= (audioInterleaved[i] == audioAdjacent[((i / 2))]);
			samplesSame &= (audioInterleaved[i + 1] == audioAdjacent[((i / 2) + sampleCount)]);
		}
		REQUIRE(samplesSame);
	}
}

TEST_CASE("Does closing and and resetting the wavereader work correctly?")
{
	for (auto supported : supportedFiles)
	{
		std::string name{ assetPath + std::string{supported} };
		Waveread wr{ std::unique_ptr<std::istream>{ new std::ifstream{name} } };

		size_t sampleCount{ 64u };

		std::vector<float> audio_initial{ wr.audio(sampleCount, sampleCount, { 0,1 }, 0u) };
		
		std::unique_ptr<std::istream> secondFileStream{ new std::ifstream{ name } };
		wr.reset(std::move(secondFileStream));

		std::vector<float> audio_reset{ wr.audio(sampleCount, sampleCount, { 0,1 }, 0u) };

		wr.close();

		std::vector<float> audio_closed{ wr.audio(sampleCount, sampleCount, { 0,1 }, 0u) };

		REQUIRE(audio_initial == audio_reset);
		REQUIRE(audio_initial == audio_closed);
	}
}