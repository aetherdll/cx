// 7 DEVELOMENT. ALL RIGHTS RESERVED.

#ifndef CX_SOUND_H
#define CX_SOUND_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cx_Sound cx_Sound;

int cx_sound_init(void);
void cx_sound_shutdown(void);
cx_Sound* cx_sound_load(const char* filepath);
void cx_sound_play(cx_Sound* sound, int loop);
void cx_sound_stop(cx_Sound* sound);
void cx_sound_free(cx_Sound* sound);

#ifdef __cplusplus
}
#endif

#endif

#ifdef CX_SOUND_IMPLEMENTATION
#ifndef CX_SOUND_IMPLEMENTATION_ONCE
#define CX_SOUND_IMPLEMENTATION_ONCE

#include 
#include 
#include 

#ifdef _WIN32
#include 
#include 
#pragma comment(lib, "winmm.lib")
#endif

struct cx_Sound {
    int sample_rate;
    int channels;
    int bits_per_sample;
    void* buffer_data;
    int buffer_size;
    char filepath[260];
};

int cx_sound_init(void) {
    return 1;
}

void cx_sound_shutdown(void) {
}

static void* cx__load_wav(const char* filepath, int* sample_rate, int* channels, int* size) {
    FILE* f = fopen(filepath, "rb");
    if (!f) return NULL;

    char header[44];
    if (fread(header, 1, 44, f) != 44) {
        fclose(f);
        return NULL;
    }

    if (strncmp(header, "RIFF", 4) != 0 || strncmp(header + 8, "WAVE", 4) != 0) {
        fclose(f);
        return NULL;
    }

    *channels = header[22];
    *sample_rate = *(int*)(header + 24);
    int data_size = *(int*)(header + 40);

    void* data = malloc(data_size);
    if (fread(data, 1, data_size, f) != data_size) {
        free(data);
        fclose(f);
        return NULL;
    }

    *size = data_size;
    fclose(f);
    return data;
}

cx_Sound* cx_sound_load(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (!ext || (strcmp(ext, ".wav") != 0 && strcmp(ext, ".WAV") != 0)) {
        return NULL;
    }

    cx_Sound* snd = (cx_Sound*)malloc(sizeof(cx_Sound));
    if (!snd) return NULL;

    memset(snd, 0, sizeof(cx_Sound));
    strncpy(snd->filepath, filepath, sizeof(snd->filepath) - 1);

    snd->buffer_data = cx__load_wav(filepath, &snd->sample_rate, &snd->channels, &snd->buffer_size);
    if (!snd->buffer_data) {
        free(snd);
        return NULL;
    }

    return snd;
}

void cx_sound_play(cx_Sound* sound, int loop) {
    if (!sound) return;

    #ifdef _WIN32
    PlaySoundA(sound->filepath, NULL, SND_FILENAME | SND_ASYNC | (loop ? SND_LOOP : 0));
    #endif
}

void cx_sound_stop(cx_Sound* sound) {
    if (!sound) return;
    #ifdef _WIN32
    PlaySoundA(NULL, 0, 0);
    #endif
}

void cx_sound_free(cx_Sound* sound) {
    if (sound) {
        cx_sound_stop(sound);
        if (sound->buffer_data) {
            free(sound->buffer_data);
        }
        free(sound);
    }
}

#endif
#endif