#pragma once

#include "pch.h"
#include "CHIP8.h"

class AudioPlayer
{
public:
	// Generate a Audio Player with WAV file.
	AudioPlayer(const std::string& filePath);
	~AudioPlayer();

	void Connect(CHIP8 *chip8);
	
	// Play one beep sound.
	void Beep();
private:
	// Call back function for audio device to generate the sound sample.
	static void audio_callback(void* userdate, Uint8* stream, int stream_len);

	// Reset wav file pointer and and length;
	void reset();
private:
	CHIP8				*m_chip8;

	Uint8				*m_audio_buf;
	Uint8				*m_origin_buf;
	Uint32				m_audio_len;
	Uint32				m_origin_len;
	SDL_AudioDeviceID	m_id;
};