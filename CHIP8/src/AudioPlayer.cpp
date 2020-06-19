#include "pch.h"
#include "AudioPlayer.h"

AudioPlayer::AudioPlayer(const std::string& filePath)
	: m_chip8(nullptr), m_audio_buf(nullptr), m_audio_len(0),
	  m_origin_buf(nullptr), m_origin_len(0)
{
	// Only support WAV file.
	std::string fileType = filePath.substr(filePath.find_last_of(".") + 1);
	if (fileType != "wav")
	{
		printf("Audio Error: Unsupport audio file type : %s\n", fileType);
		exit(1);
	}

	SDL_AudioSpec desiredSpec, obtainedSpec;

	if (SDL_LoadWAV(filePath.c_str(), &desiredSpec, &m_audio_buf, &m_audio_len) == NULL)
	{
		printf("Audio Error: Cannot load %s as an audio file.\n", filePath);
		exit(1);
	}

	// Store origin buffer position and length for resetting
	m_origin_buf = m_audio_buf;
	m_origin_len = m_audio_len;

	desiredSpec.callback = AudioPlayer::audio_callback;
	desiredSpec.userdata = this;

	// To make sure audio device support given wav file format,
	// set allow_change to 0.
	m_id = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &obtainedSpec, 0);
	if (m_id == 0)
	{
		printf("SDL_Error: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_PauseAudioDevice(m_id, 0);
};

AudioPlayer::~AudioPlayer()
{
	SDL_FreeWAV(m_audio_buf);
	SDL_CloseAudioDevice(m_id);
};

void AudioPlayer::audio_callback(void* userdata, Uint8* stream, int stream_len)
{
	AudioPlayer* audioPlayer = static_cast<AudioPlayer*>(userdata);

	if (audioPlayer->m_audio_len == 0)
		return;

	Uint32 sampleLen = static_cast<Uint32>(stream_len);
	sampleLen = (sampleLen > audioPlayer->m_audio_len ? audioPlayer->m_audio_len : sampleLen);

	// Only play given WAV file once for every Beep call.
	if (audioPlayer->m_audio_len - sampleLen <= 0)
	{
		for (int i = 0; i < stream_len; i++)
			stream[i] = 0;
		return;
	}

	SDL_memcpy(stream, audioPlayer->m_audio_buf, sampleLen);

	audioPlayer->m_audio_buf += sampleLen;
	audioPlayer->m_audio_len -= sampleLen;
};

void AudioPlayer::reset()
{
	m_audio_buf = m_origin_buf;
	m_audio_len = m_origin_len;
};

void AudioPlayer::Connect(CHIP8* chip8)
{
	m_chip8 = chip8;
};

void AudioPlayer::Beep()
{
	if (m_chip8 == nullptr)
	{
		printf("Audio Error: Failed to connect to CHIP\n");
		exit(1);
	}

	// By resetting buffer pointer to original position,
	// allowing audio device to replay the "beep" sound.
	if (m_chip8->sound_timer == 1)
		reset();
};

