/* Copyright (c) 2007 W.P. van Paassen
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* To be able to use midi, install package freepats  which contains midi instruments */

#include "SDLMixerMusicInterface.h"
#include "SexyAppBase.h"
#include "PakInterface.h"

using namespace Sexy;

SDLMixerMusicInfo::SDLMixerMusicInfo()
{
    music = NULL;
    ;
    mVolume = 1.0;
    mStopOnFade = false;
    mRepeats = false;
    mPosition = 0;
    mIsActive = false;
    mBuffer = NULL;
}

SDLMixerMusicInterface::SDLMixerMusicInterface(HWND theHWnd)
{
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
    }
    mMasterVolume = 1.0;
    mCurrentMusic = -1;
}

SDLMixerMusicInterface::~SDLMixerMusicInterface()
{
    StopAllMusic();
    SDLMixerMusicMap::iterator anItr = mMusicMap.begin();
    while (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL) {
            Mix_FreeMusic(aMusicInfo->music);
            aMusicInfo->music = NULL;
        }

        if (aMusicInfo->mBuffer != NULL) {
            delete[] aMusicInfo->mBuffer;
            aMusicInfo->mBuffer = NULL;
        }

        ++anItr;
    }

    int numtimesopened, frequency, channels;
    Uint16 format;
    numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
    while (numtimesopened--)
        Mix_CloseAudio();
}

bool SDLMixerMusicInterface::LoadMusic(int theSongId, const std::string& theFileName)
{
    SDLMixerMusicInfo aMusicInfo;

    bool pak = GetPakPtr()->isLoaded();
    std::string myFileName;
    if (pak)
        myFileName = ReplaceBackSlashes(theFileName);
    else
        myFileName = GetAppResourceFileName(theFileName);

    Mix_Music* m = NULL;

    if (pak) {

        PFILE* file = NULL;

        file = p_fopen(myFileName.c_str(), "r");
        if (file == NULL)
            file = p_fopen((myFileName + ".ogg").c_str(), "r");
        if (file == NULL)
            file = p_fopen((myFileName + ".OGG").c_str(), "r");
        if (file == NULL)
            file = p_fopen((myFileName + ".mp3").c_str(), "r");
        if (file == NULL)
            file = p_fopen((myFileName + ".MP3").c_str(), "r");
        if (file == NULL)
            file = p_fopen((myFileName + ".mid").c_str(), "r");
        if (file == NULL)
            file = p_fopen((myFileName + ".MID").c_str(), "r");
        if (file == NULL)
            file = p_fopen((myFileName + ".mod").c_str(), "r");
        if (file == NULL)
            file = p_fopen((myFileName + ".MOD").c_str(), "r");

        if (file == NULL)
            return false;

        int size = file->mRecord->mSize - file->mPos;
        aMusicInfo.mBuffer = new Uint8[size];
        int res = p_fread((void*) aMusicInfo.mBuffer, sizeof (Uint8), size * sizeof (Uint8), file);
        if (size != res)
            return false;

        SDL_RWops* rw = SDL_RWFromMem(aMusicInfo.mBuffer, size);
        m = Mix_LoadMUS_RW(rw);
        p_fclose(file);
    } else {
        int aLastDotPos = myFileName.rfind('.');
        int aLastSlashPos = (int) myFileName.rfind('/');

        if (aLastDotPos > aLastSlashPos) {
            m = Mix_LoadMUS(myFileName.c_str());
        } else {
            m = Mix_LoadMUS((myFileName + ".ogg").c_str());
            if (m == NULL)
                m = Mix_LoadMUS((myFileName + ".OGG").c_str());
            if (m == NULL)
                m = Mix_LoadMUS((myFileName + ".mp3").c_str());
            if (m == NULL)
                m = Mix_LoadMUS((myFileName + ".MP3").c_str());
            if (m == NULL)
                m = Mix_LoadMUS((myFileName + ".mid").c_str());
            if (m == NULL)
                m = Mix_LoadMUS((myFileName + ".MID").c_str());
            if (m == NULL)
                m = Mix_LoadMUS((myFileName + ".mod").c_str());
            if (m == NULL)
                m = Mix_LoadMUS((myFileName + ".MOD").c_str());
        }
    }

    if (m != NULL) {
        aMusicInfo.music = m;
        mMusicMap.insert(SDLMixerMusicMap::value_type(theSongId, aMusicInfo));
        return true;
    }

    return false;
}

void SDLMixerMusicInterface::UnloadMusic(int theSongId)
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL) {
            Mix_FreeMusic(aMusicInfo->music);
            aMusicInfo->music = NULL;
        }

        if (aMusicInfo->mBuffer != NULL) {
            delete[] aMusicInfo->mBuffer;
            aMusicInfo->mBuffer = NULL;
        }

        mMusicMap.erase(anItr);
    }
}

void SDLMixerMusicInterface::UnloadAllMusic()
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.begin();
    while (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL) {
            Mix_FreeMusic(aMusicInfo->music);
            aMusicInfo->music = NULL;
        }

        if (aMusicInfo->mBuffer != NULL) {
            delete[] aMusicInfo->mBuffer;
            aMusicInfo->mBuffer = NULL;
        }

        ++anItr;
    }
    mMusicMap.clear();
}

void SDLMixerMusicInterface::PauseAllMusic()
{
    Mix_PauseMusic();
}

void SDLMixerMusicInterface::ResumeAllMusic()
{
    Mix_ResumeMusic();
}

void SDLMixerMusicInterface::DeactivateAllMusic()
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.begin();
    while (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL)
            aMusicInfo->mIsActive = false;
        ++anItr;
    }
}

void SDLMixerMusicInterface::PlayMusic(int theSongId, int theOffset, bool noLoop)
{
    DeactivateAllMusic();

    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo != NULL) {
            aMusicInfo->mIsActive = true;

            Mix_PlayMusic(aMusicInfo->music, noLoop ? 1 : -1);
            if (theOffset != 0) {
                Mix_SetMusicPosition(theOffset);
            }
            Mix_VolumeMusic((int) ((float) mMasterVolume * aMusicInfo->mVolume * 128));

            mCurrentMusic = theSongId;
        }
    } else {
        //TODO Load Music from sound id
    }
}

void SDLMixerMusicInterface::PauseMusic(int theSongId)
{
    PauseAllMusic();
}

void SDLMixerMusicInterface::ResumeMusic(int theSongId)
{
    ResumeAllMusic();
}

void SDLMixerMusicInterface::StopMusic(int theSongId)
{
    StopAllMusic();
}

void SDLMixerMusicInterface::StopAllMusic()
{
    Mix_HaltMusic();
}

void SDLMixerMusicInterface::FadeIn(int theSongId, int theOffset, double theSpeed, bool noLoop)
{
    DeactivateAllMusic();

    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->music != NULL) {
            aMusicInfo->mIsActive = true;
            Mix_FadeInMusicPos(aMusicInfo->music, noLoop ? 1 : -1, (int) (1.0f / theSpeed), theOffset);
            mCurrentMusic = theSongId;
        }
    }
}

void SDLMixerMusicInterface::FadeOut(int theSongId, bool stopSong, double theSpeed)
{
    FadeOutAll(true, theSpeed);
}

void SDLMixerMusicInterface::FadeOutAll(bool stopSong, double theSpeed)
{
    Mix_FadeOutMusic((int) (1.0f / theSpeed));
}

void SDLMixerMusicInterface::SetVolume(double theVolume)
{
    mMasterVolume = (float) theVolume;

    SDLMixerMusicMap::iterator anItr = mMusicMap.find(mCurrentMusic);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        if (aMusicInfo->mIsActive && Mix_PlayingMusic())
            Mix_VolumeMusic((int) (mMasterVolume * aMusicInfo->mVolume * 128.0f));
    }
}

void SDLMixerMusicInterface::SetSongVolume(int theSongId, double theVolume)
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        aMusicInfo->mVolume = (float) theVolume;
        if (aMusicInfo->mIsActive && Mix_PlayingMusic())
            Mix_VolumeMusic((int) (mMasterVolume * aMusicInfo->mVolume * 128.0f));
    }
}

bool SDLMixerMusicInterface::IsPlaying(int theSongId)
{
    SDLMixerMusicMap::iterator anItr = mMusicMap.find(theSongId);
    if (anItr != mMusicMap.end()) {
        SDLMixerMusicInfo* aMusicInfo = &anItr->second;
        return aMusicInfo->mIsActive && Mix_PlayingMusic();
    }
    return false;
}

void SDLMixerMusicInterface::Update()
{
}
