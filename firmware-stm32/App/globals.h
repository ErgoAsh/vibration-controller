#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGULATION_SAMPLES_COUNT 64UL
#define CALIBRATION_SAMPLES_COUNT 1024UL
#define SEQUENCE_SAMPLES_COUNT 4096UL

static const float SENSOR_SENSITIVITY = 0.56f / 1000.0f; // V/um
static const uint32_t CALIBRATION_STAY_IN_PLACE_DELAY_MS = 10000;
static const float DT = 0.001f; // Time between measurements

extern uint8_t rx_buffer[64];
extern uint8_t tx_buffer[64];

extern bool button_state;
extern bool has_timer_tick1kHz_compared;

extern float calibration_mean;
extern volatile uint16_t calibration_samples[CALIBRATION_SAMPLES_COUNT];
extern volatile float regulation_samples[REGULATION_SAMPLES_COUNT];
extern volatile float sequence_samples[SEQUENCE_SAMPLES_COUNT];

extern uint32_t highest_fsm_id;

#endif /* GLOBALS_H_ */
