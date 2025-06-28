#ifndef MAIN_CONSTANTES_H_
#define MAIN_CONSTANTES_H_
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DECLARATION DES BIBLIOTHEQUES
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>  // Ajout nécessaire pour les types uint8_t, uint16_t, etc.

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DECLARATION DES CONSTANTES
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr float kXrange = 2.f * 3.14159265359f;
// extern int kInferencesPerCycle;
// constantes qui interviennent dans task
#define NUM_BUFFERS 5                                            // Nombre de buffers pour les données audio
#define SLICE_DURATION_MS 250                                    // Durée de la tranche en millisecondes
#define READ_CHUNK_SIZE 128                                      // Taille de la tranche à lire à la fois
#define SLICE_DURATION_S (SLICE_DURATION_MS / 1000.0)            // durée de la tranche en s : 0.25 secondes
#define SAMPLES_PER_SLICE (int)(SAMPLE_RATE * SLICE_DURATION_S)  // 0.25 * 16000 = 4000
#define CLAMP -3
// DEFINITION DES CONSTANTES POUR SPECTROGRAMME
#define FFT_SIZE 64                                      // Taille de la FFT, doit être un multiple de 2 (pas nécessairement de 16)
#define SAMPLE_RATE 16000                                // Taux d'échantillonnage en Hz
#define DUREE 1                                          // Durée de la trame en secondes (20 ms)
#define DUREE_FRAME 0.02                                 // Durée de la trame en secondes (20 ms)
#define DUREE_STRIDE 0.01                                // Durée du stride (chevauchement) en secondes (10 ms)
#define FRAME_SIZE ((int)(DUREE_FRAME * SAMPLE_RATE))    // 320 échantillons
#define STRIDE_SIZE ((int)(DUREE_STRIDE * SAMPLE_RATE))  // 160 échantillons
// Calcul du nombre de frames par tranche
#define NUM_FRAMES (SAMPLES_PER_SLICE / STRIDE_SIZE)          // nombre de frames par tranche : (4000)/160 = 25 ;
#define NUM_FRAMES_TOTAL (DUREE * SAMPLE_RATE / STRIDE_SIZE)  // le nombre de frame total sera de 100
#define INPUT_SIZE ((FFT_SIZE / 2 + 1) * NUM_FRAMES_TOTAL)    // 99*33 echantillons à l'entrée du reseau de neurone
// nombre de sortie du reseau de neurone
#define OUTPUT_SIZE 3
#define BUFFER_SIZE 10  // Définis la taille du buffer
// echappement couleur
#define SEUIL_DETECTION 0.98
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"
// NO LOG !!
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESP_LOG_NONE
#endif /* MAIN_CONSTANTES_H_ */
