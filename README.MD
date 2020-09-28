# Wavereader

Waveread is a tiny library that lets you load and read audio from common WAVE file formats.

It is designed for programs which need to use audio data chronologically, piece by piece.
It has a small caching mechanism to minimise memory footprint, which will load audio in the background.

## Formats supported

WaveReader supports uncompressed WAVE files with 8-bit unsigned,
or 16,24,32-bit signed integers.
