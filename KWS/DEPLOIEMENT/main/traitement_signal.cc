#include "traitement_signal.h"

#include <math.h>

#include "constantes.h"
#include "esp_dsp.h"
#include "esp_log.h"
#include "variables.h"

static float window[FRAME_SIZE];  // Tableau pour la fenêtre Hamming
// Déclaration des variables statiques
static float clamp;
static float moyenne;
static float ecartType;

// entrée : NULL, sortie window[FRAME_SIZE]
// attention : marche pour une frame unique de 0.02s de 320 elements
void generate_hamming_window() {
  for (int i = 0; i < FRAME_SIZE; i++) {
    window[i] = 0.54f - 0.46f * cosf(2 * M_PI * i / (FRAME_SIZE - 1));
  }
}

// entrée : pointeur sur un tableau d'int16_t,
// sortie : pointeur sur un tableau de float
// attention : marche pour une frame unique de 0.02s de 320 elements
void convert_int16_to_float(const int16_t *input, float *output) {
  for (int i = 0; i < FRAME_SIZE; i++) {
    output[i] = (float)input[i];
  }
}

// entrée : pointeur sur un tableau frame[FRAME_SIZE] de float
// et pointeur sur un tableau window[FRAME_SIZE] de float
// sortie : pointeur sur un tableau frame de float
// attention : marche pour un FFT de 33 elements pour une frame unique de 0.02s
// de 320 elements
void apply_hamming_window(float *frame, float *window) {
  dsps_mul_f32(frame, window, frame, FRAME_SIZE, 1, 1, 1);
}

// FFT avec le DSP de l'ESP32
// entrée : pointeur sur un tableau frame[FRAME_SIZE] de float
// sortie : pointeur sur un tableau fft_output[1 + FFT_SIZE / 2] de float
// attention : porte le calcul de la FFT de 33 elements pour une frame de 0.02s
// attention : sort un FFT pour une frame unique de 0.02s
void calculate_fft(float *frame, float *fft_output) {
  float y_cf[FFT_SIZE * 2];  // Signal d'entrée complexe (max 512 points, donc
                             // 1024 pour stocker réel et imaginaire)
  float real_squared[1 + FFT_SIZE / 2];
  float imag_squared[1 + FFT_SIZE / 2];
  // Initialisation de la FFT
  dsps_fft2r_init_fc32(NULL, FFT_SIZE);

  // Préparation du signal complexe
  for (int i = 0; i < FFT_SIZE; i++) {
    y_cf[i * 2 + 0] = frame[i];  // Partie réelle du signal
    y_cf[i * 2 + 1] = 0;         // Partie imaginaire initialisée à 0
  }

  // Calcul de la FFT
  dsps_fft2r_fc32(y_cf, FFT_SIZE);
  // Réorganisation des données par bit-reversal
  dsps_bit_rev_fc32(y_cf, FFT_SIZE);
  // Création de tableaux temporaires pour stocker les résultats des carrés
  dsps_mul_f32(&y_cf[0], &y_cf[0], real_squared, 1 + FFT_SIZE / 2, 2, 2, 1);
  dsps_mul_f32(&y_cf[1], &y_cf[1], imag_squared, 1 + FFT_SIZE / 2, 2, 2, 1);
  dsps_add_f32(real_squared, imag_squared, fft_output, 1 + FFT_SIZE / 2, 1, 1,
               1);
  // calcul racine carré
  dsps_sqrt_f32(fft_output, fft_output, 1 + FFT_SIZE / 2);
  dsps_mulc_f32(fft_output, fft_output, (1.0 / (FFT_SIZE)), 1 + FFT_SIZE / 2, 1,
                1);
  for (int i = 0; i <= FFT_SIZE / 2; i++)
    fft_output[i] = log10f(fft_output[i] + 1e-6);
}

// // entrée : pointeur sur un float : clamp,
// // sortie : spectrogramme[FFT_SIZE / 2 + 1][NUM_FRAMES_TOTAL] de float
// // attention : porte sur tout le spectrogramme, pour 1s
// void apply_clamp(float clamp) {
//   for (int j = 0; j < NUM_FRAMES_TOTAL; j++) {
//     for (int i = 0; i <= FFT_SIZE / 2; i++) {
//       if (spectrogramme[i][j] < clamp)
//         spectrogramme[i][j] = clamp;
//     }
//   }
// }

// entrée : pointeur sur un float : clamp,
// sortie : spectrogramme[FFT_SIZE / 2 + 1][NUM_FRAMES_TOTAL] de float
// attention : porte sur UNE  FRAME
void process_spectrogramme_frame(int16_t *audio_frame, float *fft_output) {
  float frame[FRAME_SIZE];

  // Conversion des données en float
  convert_int16_to_float(audio_frame, frame);

  // Application de la fenêtre Hamming
  apply_hamming_window(frame, window);

  // Calcul de la FFT
  calculate_fft(frame, fft_output);
}

// entrée : spectrogramme[FFT_SIZE / 2 + 1][NUM_FRAMES_TOTAL] de float
// sortie : pointeur sur un float : moyenne, pointeur sur un float : ecartType
// attention : porte sur tout le spectrogramme, pour 1s
void calculate_mean_standardDeviation(float *moyenne, float *ecartType) {
  float sum = 0;
  int count = (FFT_SIZE / 2 + 1) * NUM_FRAMES_TOTAL;  // Assure-toi que cette valeur est correcte

  // Calcul de la valeur moyenne
  for (int j = 0; j < NUM_FRAMES_TOTAL; j++) {
    for (int i = 0; i <= FFT_SIZE / 2; i++) {
      sum += spectrogramme[i][j];
    }
  }
  *moyenne = sum / count;

  // Calcul de l'écart-type
  float variance = 0;
  for (int j = 0; j < NUM_FRAMES_TOTAL; j++) {
    for (int i = 0; i <= FFT_SIZE / 2; i++) {
      variance += pow(spectrogramme[i][j] - *moyenne, 2);
    }
  }
  *ecartType = sqrt(variance / count);
}

// entrée : speectrogramme[FFT_SIZE / 2 + 1][NUM_FRAMES_TOTAL] de float
// sortie :  speectrogramme[FFT_SIZE / 2 + 1][NUM_FRAMES_TOTAL] de float
// attention : porte sur tout le spectrogramme, pour 1s
void apply_normalization(float moyenne, float ecartType) {
  // Étape 1 : Soustraction de la moyenne sur chaque élément du spectrogramme
  dsps_addc_f32((float *)spectrogramme, (float *)spectrogramme, INPUT_SIZE,
                -moyenne, 1, 1);
  // Étape 2 : Division par l'écart-type
  dsps_mulc_f32((float *)spectrogramme, (float *)spectrogramme, INPUT_SIZE,
                1.0 / ecartType, 1, 1);
}

// entrée : clamp, moyenne, ecartType en float et spectrogramme[FFT_SIZE / 2 +
// 1][NUM_FRAMES_TOTAL] de float sortie :  speectrogramme[FFT_SIZE / 2 +
// 1][NUM_FRAMES_TOTAL] de float attention : porte sur tout le spectrogramme,
// pour 1s
void process_normalization() {
  calculate_mean_standardDeviation(&moyenne, &ecartType);
  ESP_LOGI("process_normalization", "Moyenne calculée : %f", moyenne);
  ESP_LOGI("process_normalization", "Écart-type calculé : %f", ecartType);
  apply_normalization(moyenne, ecartType);
}