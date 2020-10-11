# waveread

*Version 0.1 released!*

<img src="https://billguastalla.com/binaries/wavereader/github_resources/wave-highlight.png" width="250" alt="audio displayed in the style of an oscilloscope">

[![Language](https://img.shields.io/badge/language-C++-red.svg?style=flat-square)](https://github.com/billguastalla/wavereader/blob/master/src/waveread.hpp)
[![CMake](https://github.com/billguastalla/wavereader/workflows/CMake/badge.svg?branch=master)](https://github.com/billguastalla/wavereader/actions?query=workflow%3ACMake)

[![Code Review](https://www.zomis.net/codereview/shield/?qid=250440)](https://codereview.stackexchange.com/q/250440/231226)
[![License](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)](https://github.com/billguastalla/wavereader/blob/master/LICENSE.MD)

Waveread is a reluctant re-invention of the wheel, arising from the absence of a modern WAV file reader library with a caching mechanism and a low memory footprint.
So this is what it is, a tiny C++11 library that reads audio from common variants of the WAVE file specification.

It leaves the IO implementation to the std::istream you provide it with. The caching mechanism will load audio in the background, emulating the buffering effect of an online player.

<img src="https://billguastalla.com/binaries/wavereader/github_resources/loadingbar.png" width="250" alt="media player loading bar">

Waveread supports uncompressed WAVE files with 8-bit unsigned, or 16,24,32-bit signed integers.

## usage

You can include <a href="https://raw.githubusercontent.com/billguastalla/wavereader/master/src/waveread.hpp">waveread.hpp</a> in your project as a single file. Construct a `std::istream`, pass it to a `std::unique_ptr` and provide that to an instance of `Waveread`. Then get audio using `audio()`

```cpp
#include <waveread.h>

int main()
{
	std::unique_ptr<std::istream> stream{ new std::ifstream{ "file.wav" } };
	Waveread wr{ std::move(stream) };

	// A: two channels: the first 128 samples of the first two channels of audio.
	std::vector<float> audio1{ wr.audio(0u, 128u, { 0,1 }) };
	// B: one channel: get 128 samples of audio with stride 1: meaning, pick every other sample.
	std::vector<float> audio2{ wr.audio(0u, 128u, {0}, 1u) };
}
```

### sample layout
One thing worth noticing is that with a mono file, if you ask for a left and right track like in (A), you'll get two copies of the mono track: the channels parameter in audio() is a `std::set<uint16_t>` and you can pass out of bounds channels that will be modulo'd to produce audio.

Like in the WAVE specification itself, samples returned from `audio(..)` are interleaved by default. So when asking for two samples from two channels, you'll get a vector that looks like this
```
C1S1 C2S1 C1S2 C2S2
```
However you can ask for samples from each channel to be grouped together by setting the last parameter 'bool interleaved' in `audio()` to false. Calls made this way will produce a vector that look like this
```
C1S1 C1S2 C2S1 C2S2
```

## cmake

Waveread uses cmake to produce a build system in the canonical way,
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

## documentation

Documentation is <a href="https://billguastalla.com/binaries/wavereader/docs/html/index.html">here</a> and the waveread class is <a href="https://billguastalla.com/binaries/wavereader/docs/html/class_waveread.html">here</a>.

## changelog

*Release 0.1*:

   1. Initial release. Documentation, tests, instructions and continuous integration.
   2. Reader handles 8/16/24/32 bit uncompressed RIFF WAVE files.
   3. Reader is non-copyable but moveable into containers.
