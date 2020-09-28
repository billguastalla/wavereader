# Waveread

![audio displayed in the style of an oscilloscope](https://billguastalla.com/binaries/wavereader/github_resources/wave-highlight.png)

Waveread is a tiny library that reads audio from common variants of the WAVE file specification.

It has a small caching mechanism to minimise memory footprint, which will load audio in the background.
The caching system is a bit like for an online player.
![media player loading bar](https://billguastalla.com/binaries/wavereader/github_resources/loadingbar.png)
As such it is designed in mind of programs that use audio data chronologically, piece by piece.

## Formats supported

Waveread supports uncompressed WAVE files with 8-bit unsigned,
or 16,24,32-bit signed integers.
