/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/
#include "QYFmod.h"
#include "fmod.hpp"
#include "fmod_errors.h"
#include "QYUtil.h"

QYFmod g_QYFmod;
QYFmod* g_pQYFmod = &g_QYFmod;

void(*Common_Private_Error)(FMOD_RESULT, const char *, int);
bool ERRCHECK_fn(FMOD_RESULT result, const char *file, int line)
{
	if (result != FMOD_OK)
	{
		if (Common_Private_Error)
		{
			Common_Private_Error(result, file, line);
		}
		LOGE_("%s(%d): FMOD error %d - %s", file, line, result, FMOD_ErrorString(result));
		return false;
	}
	return true;
}
#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)

QYFmod::QYFmod()
	: m_bInit(false)
{
}

QYFmod::~QYFmod()
{
}

bool QYFmod::Init(char* szExternalPath)
{
	m_szExternalPath = szExternalPath;
	bool ret = true;
	FMOD_RESULT result;
	void *extradriverdata = 0;

	//Common_Init(&extradriverdata);

	/*
		Create a System object and initialize
	*/
	result = FMOD::System_Create(&m_system);
	ret &= ERRCHECK(result);

	result = m_system->init(32, FMOD_INIT_NORMAL, extradriverdata);
	ret &= ERRCHECK(result);

	result = m_system->createSound(Common_MediaPath("drumloop.wav"), FMOD_DEFAULT, 0, &m_sound1);
	ret &= ERRCHECK(result);

	result = m_sound1->setMode(FMOD_LOOP_OFF);	/* drumloop.wav has embedded loop points which automatically makes looping turn on, */
	ret &= ERRCHECK(result);					/* so turn it off here.  We could have also just put FMOD_LOOP_OFF in the above CreateSound call. */

	result = m_system->createSound(Common_MediaPath("jaguar.wav"), FMOD_DEFAULT, 0, &m_sound2);
	ret &= ERRCHECK(result);

	result = m_system->createSound(Common_MediaPath("swish.wav"), FMOD_DEFAULT, 0, &m_sound3);
	ret &= ERRCHECK(result);

	m_bInit = ret;
	return ret;
}

void QYFmod::Release()
{
	FMOD_RESULT result;
	result = m_sound1->release();
	ERRCHECK(result);
	result = m_sound2->release();
	ERRCHECK(result);
	result = m_sound3->release();
	ERRCHECK(result);
	result = m_system->close();
	ERRCHECK(result);
	result = m_system->release();
	ERRCHECK(result);

	for (std::vector<char *>::iterator item = m_vecPathList.begin(); item != m_vecPathList.end(); ++item)
	{
		free(*item);
	}
	m_vecPathList.clear();
}

const char* QYFmod::Common_MediaPath(const char *fileName)
{
	char *filePath = (char *)calloc(256, sizeof(char));
	////strcat(filePath, "file:///android_asset/");//The URI "file:///android_asset/" points to YourProject/app/src/main/assets/.
	sprintf(filePath, "%s/%s", m_szExternalPath, fileName);
	m_vecPathList.push_back(filePath);
	return filePath;
}

//void Common_Sleep(unsigned int ms)
//{
//	usleep(ms * 1000);
//}
bool QYFmod::Update(bool playSound1, bool playSound2, bool playSound3)
{
	if (!m_bInit)
	{
		LOGE_("@@QYFmod::Update, return false because !m_bInit ");
		return false;
	}

	bool ret = true;
	FMOD_RESULT result;

	//Common_Update();
	
	if (playSound1)
	{
		result = m_system->playSound(m_sound1, 0, false, &m_channel);
		ret &= ERRCHECK(result);
	}
	
	if (playSound2)
	{
		result = m_system->playSound(m_sound2, 0, false, &m_channel);
		ret &= ERRCHECK(result);
	}
	
	if (playSound3)
	{
		result = m_system->playSound(m_sound3, 0, false, &m_channel);
		ret &= ERRCHECK(result);
	}

	result = m_system->update();
	ret &= ERRCHECK(result);

	{
		unsigned int ms = 0;
		unsigned int lenms = 0;
		bool         playing = 0;
		bool         paused = 0;
		int          channelsplaying = 0;

		if (m_channel)
		{
			FMOD::Sound *currentsound = 0;

			result = m_channel->isPlaying(&playing);
			if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
			{
				ret &= ERRCHECK(result);
			}

			result = m_channel->getPaused(&paused);
			if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
			{
				ret &= ERRCHECK(result);
			}

			result = m_channel->getPosition(&ms, FMOD_TIMEUNIT_MS);
			if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
			{
				ret &= ERRCHECK(result);
			}

			m_channel->getCurrentSound(&currentsound);
			if (currentsound)
			{
				result = currentsound->getLength(&lenms, FMOD_TIMEUNIT_MS);
				if ((result != FMOD_OK) && (result != FMOD_ERR_INVALID_HANDLE) && (result != FMOD_ERR_CHANNEL_STOLEN))
				{
					ret &= ERRCHECK(result);
				}
			}
		}

		m_system->getChannelsPlaying(&channelsplaying, NULL);
	}

	//Common_Sleep(50);

	return ret;
}