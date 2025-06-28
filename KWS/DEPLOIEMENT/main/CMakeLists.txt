
#
# Main component of TF Micro project 'micro_speech'.
#

idf_component_register(
    SRCS main.cc model.cc output_handler.cc led.cc neurone.cc tasks.cc micro.cc traitement_signal.cc variables.cc
    PRIV_REQUIRES spi_flash esp_timer 
    REQUIRES driver esp-dsp
    INCLUDE_DIRS ".")
