/*
  Name:        glcSound.h
  Copyright:   Version 0.1
  Author:      Rodrigo Luis de Souza da Silva
  Date:        26/10/2017
  Description: Add sound through OpenAL
*/

#ifndef _GLC_SOUND_H
#define _GLC_SOUND_H

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "al.h"
#include "alc.h"
#include "alut.h"

using namespace std;

class glcSound
{
    public:
        glcSound();
        ~glcSound();
        void SetNumberOfSounds(int);
        void AddSound(int id, string name);
        void PlaySound(int sound = 0);

    protected:
        ALboolean LoadData(int id);
        ALboolean LoadALData();
        void KillALData();
        void PrintError(string func, string error);

    private:
        int numberOfSources;
        ALuint *Buffers;  // Buffers to hold sound data
        ALuint *Sources;  // Sources are points of emitting sound
};

#endif


