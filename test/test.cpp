#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <waveread.h>

int n()
{
	return 1;
}

TEST_CASE("nothing useful yet", "[asdf]" )
{
	REQUIRE( n() == 1 );
	REQUIRE( n() + 1 == 2);

	std::shared_ptr<std::ifstream> asdf{ new std::ifstream{"samples.zip"} };

	AudioReader waveReader{ asdf };
}

