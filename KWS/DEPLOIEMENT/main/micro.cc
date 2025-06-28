/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DECLARATION DES BIBLIOTHEQUES
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#include "micro.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "constantes.h"
#include "variables.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////// DEFINITION DES FONCTIONS
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Fonction d'initialisation du micro qui retourne les paramètres I2S
// Fonction d'initialisation du micro qui retourne les paramètres I2S
micro_task_params_t* micro_init(void)
{
    // Allouer de la mémoire pour les paramètres de la tâche
    micro_task_params_t* task_params = (micro_task_params_t*)malloc(sizeof(micro_task_params_t));
    if (!task_params) {
        ESP_LOGE("micro_init", "Failed to allocate memory for task parameters");
        return NULL;
    }

    // Configuration du canal I2S
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_AUTO,              // Laisser le système assigner le numéro de canal automatiquement
        .role = I2S_ROLE_MASTER,         // Mode maître
        .dma_desc_num = 8,               // Nombre de tampons DMA
        .dma_frame_num = 128,            // Longueur de chaque tampon DMA
        .auto_clear = true               // Effacement automatique du buffer de transmission
    };

    i2s_chan_handle_t rx_chan;
    esp_err_t ret = i2s_new_channel(&chan_cfg, NULL, &rx_chan);  // Créer le canal RX I2S
    if (ret != ESP_OK) {
        ESP_LOGE("micro_init", "Failed to create I2S channel: %s", esp_err_to_name(ret));
        free(task_params);
        return NULL;
    }

    // Configuration standard I2S
    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = SAMPLE_RATE,  // Fréquence d'échantillonnage
            .clk_src = I2S_CLK_SRC_DEFAULT,     // Source d'horloge par défaut
            .mclk_multiple = I2S_MCLK_MULTIPLE_256  // Multiplicateur de MCLK par défaut
        },
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,  // Échantillon de 16 bits
            .slot_mode = I2S_SLOT_MODE_MONO,             // Mode mono
            .slot_mask = I2S_STD_SLOT_RIGHT,             // Utiliser uniquement le canal droit
            .ws_width = 16,                // Largeur de l'horloge WS par défaut
            .ws_pol = 1,                // Polarité normale pour WS
            .bit_shift = 1                   // Alignement sur le bit de poids fort
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,   // Pas de broche MCLK utilisée
            .bclk = I2S_BCLK_PIN,      // Broche BCLK (horloge série)
            .ws = I2S_WS_PIN,          // Broche WS (Word Select)
            .dout = I2S_GPIO_UNUSED,   // Pas de sortie de données
            .din = I2S_DATA_IN_PIN,    // Broche d'entrée des données (SD)
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    // Initialisation du mode standard I2S
    ret = i2s_channel_init_std_mode(rx_chan, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE("micro_init", "Failed to initialize I2S standard mode: %s", esp_err_to_name(ret));
        i2s_del_channel(rx_chan);
        free(task_params);
        return NULL;
    }

    ret = i2s_channel_enable(rx_chan);
    if (ret != ESP_OK) {
        ESP_LOGE("micro_init", "Failed to enable I2S channel: %s", esp_err_to_name(ret));
        i2s_del_channel(rx_chan);
        free(task_params);
        return NULL;
    }

    // Stocker les paramètres dans la structure
    task_params->rx_chan = rx_chan;
    task_params->buffer_size = I2S_BUFFER_SIZE;

    return task_params;
}