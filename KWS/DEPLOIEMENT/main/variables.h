/*
 * variables.h
 *
 *  Created on: 13 sept. 2024
 *      Author: Bertrand
 */

#ifndef MAIN_VARIABLES_H_
#define MAIN_VARIABLES_H_

#include <constantes.h>  // Ajout nécessaire pour les types uint8_t, uint16_t, etc.
#include <stdint.h>      // Ajout nécessaire pour les types uint8_t, uint16_t, etc.

#include "driver/i2s.h"         // Inclure l'en-tête I2S pour i2s_chan_handle_t
#include "freertos/FreeRTOS.h"  // Inclusion nécessaire pour QueueHandle_t
#include "freertos/queue.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DECLARATION DES VARIABLES GLOBALES
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Structure pour stocker les paramètres I2S
typedef struct {
  i2s_chan_handle_t rx_chan;  // Handle pour le canal de réception I2S
  uint32_t buffer_size;       // Taille du buffer I2S
} micro_task_params_t;

typedef struct {
  float data[BUFFER_SIZE][3];  // Tableau pour stocker les valeurs de x, y, z
  int index;                   // Index pour le prochain élément à écrire
} RollingBuffer;

extern float input_values[(FFT_SIZE / 2 + 1) * 99];  // Déclaration sans initialisation
extern float output_values[OUTPUT_SIZE];             // Déclaration sans initialisation
extern float fft_output[FFT_SIZE / 2 + 1];
extern QueueHandle_t buffer_queue;  // Déclaration sans initialisation
extern QueueHandle_t spectrogram_queue;
extern int16_t audio_buffers[NUM_BUFFERS][SAMPLES_PER_SLICE];  // Déclaration externe (ajoutez ceci si nécessaire)
extern float spectrogramme[FFT_SIZE / 2 + 1][NUM_FRAMES_TOTAL];
extern float spectrogramme_buffers[NUM_BUFFERS][FFT_SIZE / 2 + 1][NUM_FRAMES];
extern RollingBuffer outputBuffer;  // Initialisation du buffer et de l'index

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// PROTOCOLES DES FONCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
void initialize_buffer_queue();
void initialize_spectrogram_queue();

#endif /* MAIN_VARIABLES_H_ */
