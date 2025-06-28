/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DEFINITION DES BIBLIOTHEQUES
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "tasks.h"

#include "constantes.h"
#include "neurone.h"
#include "output_handler.h"
#include "traitement_signal.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DECLARATION DES VARIABLES STATIQUE
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
static int buffers_processed = 0;
static int flag_at_least_4_buffers = 0;
static SemaphoreHandle_t buffers_mutex;
QueueHandle_t buffer_queue =
    nullptr;  // Incorrect : Déclaration et initialisation
QueueHandle_t spectrogram_queue = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DECLARATION DES FONCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Tâche FreeRTOS pour contrôler les LEDs
void led_task(void *pvParameter) {
  float x, y, z;
  // Initialisation de la LED
  led_init();

  bool led_on = false;

  while (1) {
    readLastFromBuffer(&outputBuffer, &x, &y, &z);
    // Vérifier si la valeur dépasse le seuil
    if (z > SEUIL_DETECTION) {
      // Changer l'état de la LED
      if (led_on) {
        led_set_color(0, 0, 0, 0);
        led_refresh();
      } else {
        led_set_color(0, 0, 255, 0);
        led_refresh();
      }

      // Inverser l'état de la LED
      led_on = !led_on;

      // Délai pour éviter les changements trop rapides
      vTaskDelay(pdMS_TO_TICKS(520));
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// tache lecture micro et d'enregistrement des buffers
void micro_read_task(void *args) {
  micro_task_params_t *task_params = (micro_task_params_t *)args;
  size_t bytes_to_read =
      READ_CHUNK_SIZE;  // Taille du tampon de lecture, par exemple 2048 octets
  uint8_t *r_buf = (uint8_t *)malloc(bytes_to_read);
  if (!r_buf) {
    ESP_LOGE("micro_read_task", "Failed to allocate memory for read buffer");
    free(task_params);
    vTaskDelete(NULL);
    return;
  }

  int buffer_index = 0;  // Index du buffer actuel (0 à NUM_BUFFERS - 1)
  size_t r_bytes = 0;
  size_t total_bytes_read = 0;
  size_t bytes_needed =
      SAMPLES_PER_SLICE *
      sizeof(int16_t);  // Nombre total d'octets nécessaires pour 250 ms
  uint8_t *write_ptr = (uint8_t *)
      audio_buffers[buffer_index];  // Pointeur d'écriture dans le buffer

  while (1) {
    // Lecture des données du microphone en petits morceaux
    if (i2s_channel_read(task_params->rx_chan, r_buf, bytes_to_read, &r_bytes,
                         portMAX_DELAY) == ESP_OK) {
      if (r_bytes > 0) {
        // Copier les données dans le buffer circulaire
        size_t bytes_to_copy = r_bytes;
        if (total_bytes_read + bytes_to_copy > bytes_needed) {
          // Ne pas dépasser la taille du buffer
          bytes_to_copy = bytes_needed - total_bytes_read;
        }
        memcpy(write_ptr, r_buf, bytes_to_copy);
        write_ptr += bytes_to_copy;
        total_bytes_read += bytes_to_copy;

        if (total_bytes_read >= bytes_needed) {
          // Une tranche complète a été lue
          // Signaler qu'une nouvelle tranche est disponible via la file
          // d'attente buffer_queue
          ESP_LOGI("micro_read_task", "Affichage des premiers échantillons du buffer %d :", buffer_index);  // rajout pour debug
          for (int i = 0; i < 10; i++) {
            ESP_LOGI("audio_buffers", "audio_buffers[%d][%d] = %d", buffer_index, i, audio_buffers[buffer_index][i]);
          }

          ESP_LOGI("micro_read", "Envoi du buffer_index %d dans la queue", buffer_index);
          if (xQueueSend(buffer_queue, &buffer_index, portMAX_DELAY) !=
              pdPASS) {
            ESP_LOGE("micro_read", "Failed to send buffer index to queue");
          }

          // Réinitialiser pour la prochaine tranche
          buffer_index = (buffer_index + 1) % NUM_BUFFERS;
          write_ptr = (uint8_t *)audio_buffers[buffer_index];
          total_bytes_read = 0;

          ESP_LOGI("micro_read", "Buffer %d rempli", buffer_index);
        }
      } else {
        ESP_LOGW("micro_read_task", "No data read from I2S");
      }
    } else {
      ESP_LOGE("micro_read_task", "Failed to read data from I2S");
    }
  }
  free(r_buf);
  free(task_params);
  vTaskDelete(NULL);
}

// tache process audio où je calcul le spectrogramme de chaque tranche
void process_audio_task(void *pvParameter) {
  int buffer_to_process = 0;

  // Initialiser la fenêtre Hamming une seule fois
  generate_hamming_window();
  ESP_LOGI("process_audio_task", "Fenetre de Hamming initialisée");

  while (1) {
    // Attendre qu'un buffer soit disponible
    if (xQueueReceive(buffer_queue, &buffer_to_process, portMAX_DELAY) ==
        pdPASS) {
      ESP_LOGI("process_audio_task", "Buffer à traiter : %d", buffer_to_process);
      // Traiter chaque frame
      for (int frame_current = 0; frame_current < NUM_FRAMES; frame_current++) {
        // Traiter la frame
        process_spectrogramme_frame(
            &audio_buffers[buffer_to_process][frame_current * STRIDE_SIZE],
            &fft_output[0]);

        // Stocker le résultat dans le spectrogramme
        for (int i = 0; i <= FFT_SIZE / 2; i++) {
          spectrogramme_buffers[buffer_to_process][i][frame_current] =
              fft_output[i];
        }
        // ESP_LOGI("process_audio_task", "Spectrogramme de la tranche %d de 250 ms stocké dans spectrogramme_buffers", buffer_to_process);  // XXXXX
      }
      ESP_LOGI("process_audio_task", "Calcul du spectrogramme de la tranche %d de 250 ms et stockqge spectrogramme_buffers[%d]", buffer_to_process, buffer_to_process);
      // Envoyer l'index du spectrogramme traité en dernier dans la file
      // d'attente
      if (xQueueSend(spectrogram_queue, &buffer_to_process, portMAX_DELAY) !=
          pdPASS) {
        ESP_LOGE("process_audio_task", "Failed to send spectrogram to queue");
      }
      // traitment du buffer
      buffer_processed();
      ESP_LOGI("process_audio_task",
               "Spectrogramme calculé et envoyé à la queue : Buffer %d traité",
               buffer_to_process);

      ESP_LOGI("process_audio_task", "Affichage des premières valeurs du spectrogramme du buffer %d :", buffer_to_process);
      for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
          ESP_LOGI("spectrogramme_buffers", "spectrogramme_buffers[%d][%d][%d] = %f", buffer_to_process, i, j, spectrogramme_buffers[buffer_to_process][i][j]);
        }
      }
    } else {
      ESP_LOGW("tasks.cc", "Failed to receive buffer index from queue");
    }

    // Délai optionnel si nécessaire
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/*
recevoir l'index du buffer en cours et definir les index des 4 buffers à
assembler appliquer le clamp ET copier en meme temps, il faudra modifier la
fonction calculer la moyenne et l'ecart type faire la normalisation faire
l'inference stocker le resultat de l'inference

il faudra supprimer process_normalization dans sa forme actuelle dans .h et .cc
rajouter la declaration de la fonction dans main, dans .h et .cc
*/

void process_neural_network_task(void *pvParameter) {
  int buffer_to_process;
  // attendre qu'au moins 4 buffers soient traités
  while (!is_at_least_4_buffers_processed()) {
  };
  ESP_LOGE("process_neural_network_task", "4 buffers ont été complétés pour la premiere fois");

  while (1) {
    // relever le numéro de l'index du dernier buffer traité
    if (xQueueReceive(spectrogram_queue, &buffer_to_process, portMAX_DELAY) !=
        pdPASS) {
      // Erreur dans la réception du message
      ESP_LOGE("process_neural_network_task",
               "Erreur pour prendre la valeur de l'index à traiter dans "
               "spectrogram_queue.");
      return;
    }

    // Revenir en arrière de 4 buffers en gérant le buffer circulaire
    int start_buffer = (buffer_to_process - 3 + NUM_BUFFERS) % NUM_BUFFERS;

    // Traiter les 4 buffers (du plus ancien au plus récent)
    for (int b = 0; b < 4; b++) {
      int current_buffer = (start_buffer + b) %
                           NUM_BUFFERS;  // Calcul de l'indice du buffer à traiter

      // Copie des données du buffer actuel dans le spectrogramme principal
      // et application de l'écrêtage en même temps
      for (int i = 0; i <= FFT_SIZE / 2; i++) {
        for (int j = 0; j < NUM_FRAMES; j++) {
          // Copie avec écrêtage
          float value = spectrogramme_buffers[current_buffer][i][j];
          spectrogramme[i][j] = (value < CLAMP) ? CLAMP : value;
        }
      }
      ESP_LOGI("process_neural_network_task", ANSI_COLOR_RED "spectrogramme :" ANSI_COLOR_RESET);
      for (int i = 0; i <= 10; i++)
        ESP_LOGI("process_neural_network_task", ANSI_COLOR_RED "%f" ANSI_COLOR_RESET, spectrogramme[i][1]);

      // Log d'information pour indiquer quel buffer est traité
      ESP_LOGI("process_neural_network_task", "Buffer %d traité", current_buffer);
    }

    // calcul ecart type et moyenne et normalisation du spectrogramme
    process_normalization();
    ESP_LOGI("process_neural_network_task",
             "Normalisation d'un spectrogramme complet réalisé");

    ESP_LOGI("process_neural_network_task", ANSI_COLOR_YELLOW "spectrogramme :" ANSI_COLOR_RESET);
    for (int i = 0; i <= 10; i++)
      ESP_LOGI("process_neural_network_task", ANSI_COLOR_YELLOW "%f" ANSI_COLOR_RESET, spectrogramme[i][1]);

    // copie dans input_values les valeurs. par contre, attention, c'est pas optimum, c'est lent

    // inference
    neurone_inference(input_values, output_values);
    ESP_LOGI("process_neural_network_task", "Inférence réalisée");
    HandleOutput(output_values[0], output_values[1], output_values[2]);

    // stockage des resultats
    addToBuffer(&outputBuffer, output_values[0], output_values[1], output_values[2]);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DECLARATION DES FONCTIONS POUR GESTION DU BUFFER
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
void buffer_control_init() {
  buffers_mutex = xSemaphoreCreateMutex();
  if (buffers_mutex == NULL) {
    // Gérer l'échec de création du mutex
    ESP_LOGE("tasks.cc", "Failed to create mutex for buffers.");
  }
}

void buffer_processed() {
  if (xSemaphoreTake(buffers_mutex, portMAX_DELAY) == pdTRUE) {
    buffers_processed++;
    ESP_LOGI("buffer_processed",
             "Buffer processed. Total buffers processed: %d",
             buffers_processed);

    if (buffers_processed >= (NUM_BUFFERS - 1) &&
        flag_at_least_4_buffers == 0) {
      flag_at_least_4_buffers = 1;
      ESP_LOGI("buffer_processed", "At least 4 buffers have been processed.");
      // Optionnel : Déclencher une action, par exemple notifier une tâche
    }
    xSemaphoreGive(buffers_mutex);
  } else {
    ESP_LOGE("buffer_processed",
             "Failed to take the mutex in buffer_processed.");
  }
}

int is_at_least_4_buffers_processed() {
  int flag = 0;
  if (xSemaphoreTake(buffers_mutex, portMAX_DELAY) == pdTRUE) {
    flag = flag_at_least_4_buffers;
    // ESP_LOGI("is_at_least_4_buffers_processed",
    //  "Checking if at least 4 buffers are processed: %d", flag);
    xSemaphoreGive(buffers_mutex);
  } else {
    ESP_LOGE("is_at_least_4_buffers_processed",
             "Failed to take the mutex in is_at_least_4_buffers_processed.");
  }
  return flag;
}

/* initialisation des buffers */
void initialize_buffer_queue() {
  buffer_queue = xQueueCreate(
      NUM_BUFFERS, sizeof(int));  // Crée une queue de 10 éléments de type int
  if (buffer_queue == NULL) {
    // Gérer l'erreur de création de la queue
    ESP_LOGE("tasks.cc", "Failed to create buffer_queue");
  }
}

void initialize_spectrogram_queue() {
  spectrogram_queue = xQueueCreate(
      NUM_BUFFERS, sizeof(int));  // Crée une queue de 10 éléments de type int
  if (spectrogram_queue == NULL) {
    ESP_LOGE("tasks.cc", "Failed to create spectrogram queue");
  }
}
