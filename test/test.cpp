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

TEST_CASE("Check that supported files all load data.", "[asdf]")
{
	for (auto supported : supportedFiles)
	{
		std::string name{ assetPath + std::string{supported} };
		std::shared_ptr<std::ifstream> fileStream{ new std::ifstream{name} };

		Waveread wr{ fileStream };
		REQUIRE(wr.open());
		
		std::vector<float> f{ wr.audio(0, std::numeric_limits<size_t>::max(), { 0,1 }) };
		std::cout << "name: " << name << "\t\tSample count: " << f.size() << std::endl;

		REQUIRE(f.size() == 882u);
	}

}

TEST_CASE("demo")
{
	std::string name{ assetPath + std::string{supportedFiles[4]} };
	std::shared_ptr<std::ifstream> fileStream{ new std::ifstream{name} };
	Waveread r{ fileStream };

	// A: two channels: the first 128 samples of the first two channels of audio.
	std::vector<float> audio1{ r.audio(0u, 128u, { 0,1 }) };
	// B: one channel: get 128 samples of audio with stride 1: meaning, pick every other sample.
	std::vector<float> audio2{ r.audio(0u, 128u, {0}, 1u) };
}

TEST_CASE("Does a moved object still produce audio data?")
{
	std::string name{ assetPath + std::string{supportedFiles[4]} };
	std::shared_ptr<std::ifstream> fileStream{ new std::ifstream{name} };
	Waveread a{ fileStream };
	Waveread b{ std::move(a) };
	std::vector<float> aud{ b.audio(0, 128, { 0,1 }, 0u) };

	REQUIRE(aud.size() != 0u);

	// { Compiliation test: can we move the reader into a vector? }
	std::vector<Waveread> v;
	v.push_back(std::move(b));

	std::vector<float> aud2interleaved{ v.back().audio(128, 128, { 0,1 }, 0u) };
	std::vector<float> aud2adjacent{ v.back().audio(128, 128, { 0,1 }, 0u,false)};

	REQUIRE(aud2interleaved.size() != 0u);
	REQUIRE(aud2adjacent.size() != 0u);




}
