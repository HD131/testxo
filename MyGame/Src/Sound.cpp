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

bool CSound::InitializeOpenAL()
{
	// Позиция слушателя.
	ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };

	// Скорость слушателя.
	ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };

	// Ориентация слушателя. (Первые 3 элемента – направление «на», последние 3 – «вверх»)
	ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };
	pDevice = 0;
	// Открываем заданное по умолчанию устройство
	pDevice = alcOpenDevice( 0 );
	// Проверка на ошибки
	if ( !pDevice )
	{
		Log("Default sound device not present");
		return false;
	}
	pContext = 0;
	// Создаем контекст рендеринга
	pContext = alcCreateContext( pDevice, 0 );
	if ( !pContext ) 
	{
		Log("Default sound contex not present");
		return false;
	}
	// Делаем контекст текущим
	alcMakeContextCurrent( pContext );

	// Устанавливаем параметры слушателя
	// Позиция
	alListenerfv(AL_POSITION,    ListenerPos);
	// Скорость
	alListenerfv(AL_VELOCITY,    ListenerVel);
	// Ориентация
	alListenerfv(AL_ORIENTATION, ListenerOri);
	return true;
}

void CSound::DestroyOpenAL()
{
	// Очищаем все буффера
// 	for (TBuf::iterator i = Buffers.begin(); i != Buffers.end(); i++)
// 		alDeleteBuffers(1, &i->second.ID);
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
// 	if (!CheckALError()) 
// 		return false;

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