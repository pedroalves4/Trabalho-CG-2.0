#include "glcSound.h"
#include <iostream>


glcSound::glcSound()
{
    alutInit(NULL, 0);
    alGetError();
}

glcSound::~glcSound()

{
    this->KillALData();
}

void glcSound::SetNumberOfSounds(int sources)
{
    this->numberOfSources = sources;

    // Allocate buffers and sources based on the required number of sources
    this->Buffers = new ALuint(sources);
    this->Sources = new ALuint(sources);
    alGenSources(sources, Sources);
}

void glcSound::AddSound(int id, string file)
{
    // Create buffer from file
    Buffers[id] = alutCreateBufferFromFile (file.c_str());

    if(alGetError() != AL_NO_ERROR) this->PrintError("glcSound::AddSound", "Error creating buffers.");

    // Create source from buffer
    alSourcei (Sources[id], AL_BUFFER,   Buffers[id] );

    if(alGetError() != AL_NO_ERROR) this->PrintError("glcSound::AddSound", "Error creating Sources.");
}

void glcSound::PrintError(string func, string error)
{
    cout << "In " << func << " - " << "error";
    exit(1);
}

void glcSound::PlaySound(int id)
{
    alSourcePlay(this->Sources[id]);
}

/*void glcSound::KillALData()
{
    alDeleteSources(this->numberOfSources, Sources);

    ALCcontext *context = alcGetCurrentContext();
    alcDestroyContext(context);

    alutExit();
}*/
