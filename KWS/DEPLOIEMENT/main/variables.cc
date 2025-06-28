// variables.cpp
#include "variables.h"

#include "constantes.h"
#include "esp_log.h"

// Définition et initialisation des variables extern
float input_values[(1 + FFT_SIZE / 2) * (NUM_FRAMES_TOTAL - 1)] = {
    0};  // Initialise tous les éléments à 0
float output_values[3] = {0};
float fft_output[FFT_SIZE / 2 + 1] = {0};
int16_t audio_buffers[NUM_BUFFERS][SAMPLES_PER_SLICE] = {
    0};  // Définition et initialisation
float spectrogramme_buffers[NUM_BUFFERS][FFT_SIZE / 2 + 1][NUM_FRAMES] = {0};
float spectrogramme[FFT_SIZE / 2 + 1][NUM_FRAMES_TOTAL] = {0};
// définition des variables externe pour le stockage des resutlats
RollingBuffer outputBuffer = {{0}, 0};
