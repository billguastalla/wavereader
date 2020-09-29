# waveread

Release 0.1 coming soon!

<img src="https://billguastalla.com/binaries/wavereader/github_resources/wave-highlight.png" width="250" alt="audio displayed in the style of an oscilloscope">

![License](https://img.shields.io/badge/license-MIT-green.svg?style=flat-square) 
![Language](https://img.shields.io/badge/language-C++-blue.svg?style=flat-square) 

Waveread is a reluctant re-invention of the wheel, arising from the absence of a modern WAV file reader library with a low memory footprint.
So this is what it is, a tiny C++11 library that reads audio from common variants of the WAVE file specification.

It leaves the IO implementation to the std::istream you provide it with. It has a caching mechanism to minimise memory footprint, which will load audio in the background, emulating the buffering effect of an online player.

<img src="https://billguastalla.com/binaries/wavereader/github_resources/loadingbar.png" width="250" alt="media player loading bar">

Waveread supports uncompressed WAVE files with 8-bit unsigned, or 16,24,32-bit signed integers.

## usage

Construct a `std::istream`, pass it to a `std::shared_ptr` and provide that to an instance of `Waveread`. Then get audio using `audio()`

```cpp
#include "waveread.h"

int main()
{
	std::shared_ptr<std::ifstream> fileStream{ new std::ifstream{"file.wav"} };
	Waveread r{ fileStream };

	// A: two channels: the first 128 samples of the first two channels of audio.
	std::vector<float> audio1{ r.audio(0u, 128u, { 0,1 }) };
	// B: one channel: get 128 samples of audio with stride 1: meaning, pick every other sample.
	std::vector<float> audio2{ r.audio(0u, 128u, {0}, 1u) };
}
```

### sample layout
One thing worth noticing is that with a mono file, if you ask for a left and right track like in (A), you'll get two copies of the mono track: the channels parameter in audio() is a `std::set<uint16_t>` and you can pass out of bounds channels that will be modulo'd to produce audio.

Like in the WAVE specification itself, samples returned from `audio(..)` are interleaved. So when asking for two samples from two channels, you'll get a vector that looks like this
```
C1S1 C2S1 C1S2 C2S2
```

## cmake

You build Waveread in the canonical way,
```
git clone https://www.github.com/billguastalla/wavereader.git
cd wavereader
mkdir build
cd build
cmake ./ ../
```
To run tests, you'll need to be connected to the internet at build time, as Waveread will retrieve test assets from a remote server.

<!-- Waveread is MIT licensed, so you are free to just take a copy of <a href="https://raw.githubusercontent.com/billguastalla/wavereader/master/src/waveread.h">waveread.h</a> -->
<!-- and copy it into your project, as long as you provide a copy of the <a href="https://raw.githubusercontent.com/billguastalla/wavereader/master/LICENSE.MD">license</a>. -->

