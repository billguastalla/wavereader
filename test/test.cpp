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


TEST_CASE("Check that unsupported files are stated as unsupported.", "[asdf]" )
{
//	for (auto unsupported : unsupportedFiles)
	//{
	//	std::string name{ assetPath + std::string{unsupported} };
	//	std::shared_ptr<std::ifstream> fileStream{ new std::ifstream{name} };
	//	Waveread waveReader{ fileStream,2048u,0.5};
	//	REQUIRE(waveReader.open());
	//}
}

TEST_CASE("Check that supported files are supported.", "[asdf]")
{
	for (auto supported : supportedFiles)
	{

	}
}

TEST_CASE("demo")
{
	std::shared_ptr<std::ifstream> fileStream{ new std::ifstream{"file.wav"} };
	Waveread r{ fileStream };

	// A: two channels: the first 128 samples of the first two channels of audio.
	std::vector<float> audio1{ r.audio(0u, 128u, { 0,1 }) };
	// B: one channel: get 128 samples of audio with stride 1: meaning, pick every other sample.
	std::vector<float> audio2{ r.audio(0u, 128u, {0}, 1u) };
}