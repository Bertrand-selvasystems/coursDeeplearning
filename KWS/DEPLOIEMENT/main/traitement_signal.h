
#ifndef TRAITEMENT_SIGNAL_H
#define TRAITEMENT_SIGNAL_H
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DEFINITION DES BIBLIOTHEQUES
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdlib.h>

#include "constantes.h"
#include "esp_dsp.h"
#include "esp_log.h"
#include "variables.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// PROTOCOLE DES FONCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// Génération d'une fenêtre Hamming pré-calculée
void generate_hamming_window();

// Conversion des échantillons int en float
void convert_int_to_float(const int *input, float *output);

// Application de la fenêtre Hamming avec DSP
void apply_hamming_window(float *frame, const float *window);

// FFT avec le DSP de l'ESP32
void calculate_fft(float *frame, float *fft_output);

// Pipeline complet FFT
void process_FFT(const int *int_frame, float *fft_output);

// Calcul du spectrogramme pour toutes les trames sans chevauchement
void process_spectrogramme_frame(int16_t *audio_frame, float *fft_output);

// // Application d'un clamp sur le spectrogramme
// void apply_clamp(float clamp);

// Calcul de la moyenne et de l'écart-type du spectrogramme
void calculate_mean_standardDeviation(float moyenne, float ecartType);

// Normalisation du spectrogramme
void apply_normalization(float *moyenne, float *ecartType);

// Pipeline complet de normalisation
void process_normalization();

#ifdef __cplusplus
}
#endif

#endif  // SPECTROGRAM_H