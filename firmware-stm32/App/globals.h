#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "hsm.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGULATION_ENABLED 1
#define REGULATION_BY_SAMPLE_ARRAY_ENABLED 1
#define USE_REAL_UNITS 0 // Use it if you want real units (m, m/s, m/s^2)

#define INITIAL_LOCATION LOCATION_1
#define TARGET_LOCATION LOCATION_3

#define REGULATION_SAMPLES_COUNT 4UL
#define CALIBRATION_SAMPLES_COUNT 1024UL
#define SEQUENCE_SAMPLES_COUNT 8192UL
#define REGULATION_SETPOINT_COUNT SEQUENCE_SAMPLES_COUNT

#define POINTS_PER_INDIVIDUAL 4096 // TODO move to config

typedef struct _data_point_t
{
    int index;
    float x, v, a, u;
    float distance;
} data_point_t;

static const float SENSOR_SENSITIVITY = 0.56f / 1000.0f; // V/um
static const float DELTA_TIME = 1 / 1000.0f;             // Time between measurements
static const uint32_t CALIBRATION_STAY_IN_PLACE_DELAY_MS
    = 1000; // It was equal to 10s but sending data takes very long time

extern state_machine_t *state_machines[];
extern uint8_t rx_buffer[64];
extern uint8_t tx_buffer[64];
extern bool has_usart_dma_rx_finished;
extern bool has_usart_dma_tx_finished;

extern bool button_state;
extern bool has_timer_tick1kHz_compared;

extern float calibration_mean;
extern volatile float sequence_samples[SEQUENCE_SAMPLES_COUNT];
extern volatile uint16_t calibration_samples[CALIBRATION_SAMPLES_COUNT];
extern volatile float regulation_samples[REGULATION_SAMPLES_COUNT];
extern volatile float regulation_setpoints[REGULATION_SETPOINT_COUNT];

extern data_point_t individual[POINTS_PER_INDIVIDUAL];

extern uint32_t sample_counter;
extern uint32_t regulation_sample_counter;
extern uint32_t regulation_setpoints_counter;

extern uint32_t highest_fsm_id;
extern float time_mean;

#endif /* GLOBALS_H_ */
