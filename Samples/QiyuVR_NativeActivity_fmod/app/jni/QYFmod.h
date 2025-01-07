/*******************************************************
Copyright (c) 2021 IQIYISMART, Inc. All Rights Reserved.
*******************************************************/
#ifndef _QYFMOD_H_
#define _QYFMOD_H_

#include <vector>

namespace FMOD
{
	class System;
	class Sound;
	class Channel;
}

class QYFmod
{
public:
	QYFmod();
	~QYFmod();

	bool Init(char* szExternalPath);
	void Release();
	bool Update(bool playSound1, bool playSound2, bool playSound3);

	inline bool IsInitialized() const { return m_bInit; }
protected:
	const char* Common_MediaPath(const char *fileName);

protected:
	bool m_bInit;
	char* m_szExternalPath;

	FMOD::System     *m_system;
	FMOD::Sound      *m_sound1, *m_sound2, *m_sound3;
	FMOD::Channel    *m_channel = 0;
	//FMOD_RESULT       result;
	//void             *extradriverdata = 0;
	
	std::vector<char *> m_vecPathList;
};

extern QYFmod* g_pQYFmod;

#endif//_QYFMOD_H_