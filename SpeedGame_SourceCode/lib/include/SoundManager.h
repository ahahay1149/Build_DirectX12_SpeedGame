#pragma once
#include <wrl/client.h>

//XAUDIO2.9
#include <xaudio2.h>
#include <xaudio2fx.h>
#include "WAVFileReader.h"		//MS公式が配布しているWAVファイル読み込みライブラリ
#include <list>
#include <vector>
#include <memory>

#pragma comment(lib, "xaudio2.lib")

using namespace std;
using Microsoft::WRL::ComPtr;

#define MAX_SOUNDS	32

class SoundContainer
{
private:
	std::unique_ptr<uint8_t[]>	m_waveFile;
	DirectX::WAVData			m_wavData;

public:
	SoundContainer(const WCHAR* fileName);

	DirectX::WAVData* getWavData()
	{
		return &m_wavData;
	}
};

class PlayingSoundContainer
{
private:
	//ReleaseがないのでComPtrには出来ない。
	IXAudio2SourceVoice* voice;
	IXAudio2SubmixVoice* submix;
	XAUDIO2_BUFFER		buf;

	bool				m_bPlaying;

public:
	PlayingSoundContainer()
	{
		voice = nullptr;
		submix = nullptr;
		buf = {};

		m_bPlaying = false;
	}

	HRESULT playWavData(ComPtr<IXAudio2>& pXAudio2, DirectX::WAVData* wavData);
	void stop();
	void releaseWavData();

	void update();

	bool isFinished()
	{
		return !m_bPlaying;
	}

	IXAudio2SourceVoice* getPSourceVoice()
	{
		return voice;
	}

	IXAudio2SubmixVoice* getPSubMix()
	{
		return submix;
	}

	XAUDIO2_BUFFER* getPAudioBuffer()
	{
		return &buf;
	}

	~PlayingSoundContainer();
};

class SoundManager
{
private:
	ComPtr<IXAudio2> m_pXaudio2;
	IXAudio2MasteringVoice* m_pMasteringVoice;
	
	vector<unique_ptr<SoundContainer>> m_soundContainers;
	vector<unique_ptr<PlayingSoundContainer>> m_playingContainers;

	int	m_seCount;
	int m_playIndex;

public:
	SoundManager();
	~SoundManager();

	void audioUpdate();

	HRESULT initSoundManager();
	void destructSoundManager();

	bool loadSoundFile(const WCHAR* fileName, int& soundId);
	bool deleteSoundFile(int soundId);

	int play(UINT soundId);
	void stop(UINT playingId);
};