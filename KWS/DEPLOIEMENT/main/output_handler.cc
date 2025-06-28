/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DECLARATION DES BIBLIOTHEQUES
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "output_handler.h"

#include "constantes.h"
#include "esp_timer.h"  // Inclure le header pour les fonctions de timing
#include "tensorflow/lite/micro/micro_log.h"
#include "variables.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DEFINITION DES FONCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
void HandleOutput(float x_value, float y_value, float z_value) {
  static int64_t last_call_time = 0;            // Variable statique pour stocker le temps du dernier appel
  int64_t current_time = esp_timer_get_time();  // Obtenir le temps actuel en microsecondes

  // Calculer la différence de temps depuis le dernier appel
  int64_t time_diff = current_time - last_call_time;
  last_call_time = current_time;  // Mettre à jour le temps du dernier appel

  // Log des valeurs courantes de x, y, z et du temps écoulé en millisecondes
  if (z_value > SEUIL_DETECTION) {
    MicroPrintf(ANSI_COLOR_RED "x_value: %f, y_value: %f, z_value: %f, time_diff: %lld ms" ANSI_COLOR_RESET,
                static_cast<double>(x_value), static_cast<double>(y_value), static_cast<double>(z_value),
                time_diff / 1000);  // Convertir de microsecondes à millisecondes
  } else {
    MicroPrintf(ANSI_COLOR_GREEN "x_value: %f, y_value: %f, z_value: %f, time_diff: %lld ms" ANSI_COLOR_RESET, x_value, y_value, z_value, time_diff / 1000);
  }
}

void addToBuffer(RollingBuffer *buffer, float x, float y, float z) {
  // Ajoute les nouvelles valeurs à l'index actuel
  buffer->data[buffer->index][0] = x;
  buffer->data[buffer->index][1] = y;
  buffer->data[buffer->index][2] = z;

  // Incrémente l'index et revient à zéro si nécessaire
  buffer->index = (buffer->index + 1) % BUFFER_SIZE;
}

void readLastFromBuffer(RollingBuffer *buffer, float *x, float *y, float *z) {
  // Calcule l'index de la dernière valeur ajoutée
  int lastIndex = (buffer->index - 1 + BUFFER_SIZE) % BUFFER_SIZE;

  // Lit les valeurs à cet index
  *x = buffer->data[lastIndex][0];
  *y = buffer->data[lastIndex][1];
  *z = buffer->data[lastIndex][2];
}