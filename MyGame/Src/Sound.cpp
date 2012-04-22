#include "Sound.h"

ALboolean CSound::CheckALCError()
{
	ALenum ErrCode;
	std::string Err = "ALC error: ";
	if ((ErrCode = alcGetError(pDevice)) != ALC_NO_ERROR)
	{
		Err += (char *)alcGetString(pDevice, ErrCode);
		//ERRMSG(Err.data());
		return AL_FALSE;
	}
	return AL_TRUE;
}

ALboolean CSound::CheckALError()
{
	ALenum ErrCode;
	std::string Err = "OpenAL error: ";
	if ((ErrCode = alGetError()) != AL_NO_ERROR)
	{
		Err += (char *)alGetString(ErrCode);
		//ERRMSG(Err.data());
		return AL_FALSE;
	}
	return AL_TRUE;
}

CSound::CSound()
{
	// Позиция слушателя.
	ListenerPos[0] = 0.0;
	ListenerPos[1] = 0.0;
	ListenerPos[2] = 0.0;

	// Скорость слушателя.
	ListenerVel[0] = 0.0;
	ListenerVel[1] = 0.0;
	ListenerVel[2] = 0.0;

	// Ориентация слушателя. (Первые 3 элемента – направление «на», последние 3 – «вверх»)
	ListenerOri[0] =  0.0; 
	ListenerOri[1] =  0.0;
	ListenerOri[2] = -1.0;
	ListenerOri[3] =  0.0;
	ListenerOri[4] =  1.0;
	ListenerOri[5] =  0.0;

	pDevice = 0;
	// Открываем заданное по умолчанию устройство
	pDevice = alcOpenDevice( 0 );

	if ( pDevice )
	{
		Log("Open sound device");
		pContext = 0;
		pContext = alcCreateContext( pDevice, 0 );
		if ( pContext )
			alcMakeContextCurrent( pContext );	// Делаем контекст текущим
		else
		{
			Log("Default sound contex not present");
			alcCloseDevice( pDevice );
			return;
		}
	}	
	else
	{
		Log("Default sound device not present");
		return;
	}
}

void CSound::Play()
{	
	char		*filename;
	ALuint		source;
	ALfloat		mPos[3] = {-10.0, 0.0, 0.0};
	ALfloat		mVel[3] = {0.0, 0.0, 0.0};

	unsigned int buffer;
	ALvoid		*p;
	ALsizei		size;
	ALsizei		rate; 
	ALenum		format;
	ALboolean	loop;

	filename = "galil.wav";	

	// Устанавливаем параметры слушателя
	// Позиция	
	alListenerfv( AL_POSITION,	  ListenerPos);
	// Скорость
	alListenerfv( AL_VELOCITY,    ListenerVel);
	// Ориентация
	alListenerfv( AL_ORIENTATION, ListenerOri);

	alGenSources (1, &source);
	/* Тон звука; Усиление с расстоянием */
	alSourcef  (source, AL_PITCH, 1.0f);
	alSourcef  (source, AL_GAIN, 1.0f);
	alSourcefv (source, AL_POSITION, mPos);
	alSourcefv (source, AL_VELOCITY, mVel);
	/* Зациклен ли? */
	alSourcef (source, AL_LOOPING, AL_FALSE);

	/* Загрузить данные */
	alGenBuffers (1, &buffer);
	alutLoadWAVFile ((ALbyte *) filename, &format, &p, &size, &rate, &loop);
	alBufferData (buffer, format, p, size, rate);
	alutUnloadWAV ( format, p, size, rate );
	alSourcei( source, AL_BUFFER, buffer );
	
	alSourcePlay (source);
	ALint status;
	do
	{
		alutSleep (0.1f);
		alGetSourcei (source, AL_SOURCE_STATE, &status);
	} 
	while (status == AL_PLAYING);

	alSourceStop (source);

	alDeleteSources (1, &source);
	alDeleteBuffers (1, &buffer);
}

CSound::~CSound()
{
	// Выключаем текущий контекст
	alcMakeContextCurrent(0);
	// Уничтожаем контекст
	alcDestroyContext(pContext);
	// Закрываем звуковое устройство
	alcCloseDevice(pDevice);
}








bool remSnd::Open(const std::string &Filename, bool Looped, bool Streamed)
{
	// Проверяем файл на наличие
	//ifstream a(Filename.c_str());
// 	if (!a.is_open()) 
// 		return false;
// 	a.close();

	mLooped    = Looped;

	// Создаем источник соответствующий нашему звуку
	alGenSources(1, &mSourceID);

	alSourcef (mSourceID, AL_PITCH,    1.0f);
	alSourcef (mSourceID, AL_GAIN,    1.0f);
	alSourcefv(mSourceID, AL_POSITION,  mPos);
	alSourcefv(mSourceID, AL_VELOCITY,  mVel);
	alSourcei (mSourceID, AL_LOOPING,  mLooped);

	// Extract ext
// 	std::string Ext = ExtractFileExt(Filename);
// 	if (Ext == "WAV") 
		return LoadWavFile(Filename);
	/*if (Ext == "OGG") 
	{
	mStreamed = Streamed;
	return LoadOggFile(Filename, Streamed);
	} */
	return false;
}

void remSnd::Play()
{
	alSourcePlay(mSourceID);
}

void remSnd::Close()
{
	alSourceStop(mSourceID);
	if (alIsSource(mSourceID)) alDeleteSources(1, &mSourceID);
}

void remSnd::Stop()
{
	alSourceStop(mSourceID);
}

void remSnd::Move(float X, float Y, float Z)
{
	ALfloat Pos[3] = { X, Y, Z };
	alSourcefv(mSourceID, AL_POSITION, Pos);
}

bool remSnd::LoadWavFile(const std::string &Filename)
{/*
	// Структура содержащая аудиопараметры
	SndInfo    buffer;
	// Формат данных в буфере
	ALenum    format;
	// Указатель на массив данных звука
	ALvoid    *data;
	// Размер этого массива
	ALsizei    size;
	// Частота звука в герцах
	ALsizei    freq;
	// Идентификатор циклического воспроизведения
	ALboolean  loop;
	// Идентификатор буфера
	ALuint    BufID = 0;

	// Заполняем SndInfo данными
	buffer.Filename = Filename;
	// Ищем, а нет ли уже существующего буфера с данным звуком?
	for (TBuf::iterator i = Buffers.begin(); i != Buffers.end(); i++)
	{
		if (i->second.Filename == Filename) BufID = i->first;
	}

	// Если звук загружаем впервые
	if (!BufID)
	{
		// Создаём буфер
		alGenBuffers(1, &buffer.ID);
		if (!CheckALError()) return false;
		// Загружаем данные из wav файла
		alutLoadWAVFile((ALbyte *)Filename.data(), &format, &data,
			&size, &freq, &loop);
		if (!CheckALError()) return false;

		buffer.Format      = format;
		buffer.Rate      = freq;
		// Заполняем буфер данными
		alBufferData(buffer.ID, format, data, size, freq);
		if (!CheckALError()) return false;
		// Выгружаем файл за ненадобностью
		alutUnloadWAV(format, data, size, freq);
		if (!CheckALError()) return false;

		// Добавляем этот буфер в массив
		Buffers[buffer.ID] = buffer;
	}
	else 
		buffer = Buffers[BufID];

	// Ассоциируем буфер с источником
	alSourcei (mSourceID, AL_BUFFER, buffer.ID);
*/
	return true;
}


