#include "main-app.h"

#include "adc.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"

#include "commands.h"
#include "globals.h"
#include "knn.h"
#include "utils.h"

#include "positioner-process.h"
#include "sequence-process.h"

uint32_t sample_counter = 0;
uint32_t regulation_sample_counter = 0;

uint32_t tick_counter = 0;
float calibration_mean = 0;
bool has_timer_tick1kHz_compared = false;

uint8_t rx_buffer[64];
uint8_t tx_buffer[64];

volatile uint16_t calibration_samples[1024];
volatile float regulation_samples[64];
volatile float sequence_samples[4096];

GPIO_PinState previous_button_state = GPIO_PIN_SET;
GPIO_PinState current_button_state = GPIO_PIN_SET;

state_machine_t *state_machines[] = {
    (state_machine_t *) &positioner_process,
    (state_machine_t *) &sequence_process,
};

void on_initialize()
{
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_buffer, 64);
    HAL_TIM_Base_Start_IT(&timer_1kHz);
    HAL_TIM_Base_Start_IT(&timer_regulation);
    HAL_TIM_PWM_Start(&timer_electromagnet_left, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&timer_electromagnet_right, TIM_CHANNEL_1);

    HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);

    init_positioner_process(&positioner_process);
    init_sequence_process(&sequence_process);
    test_knn();
}

bool has_changed_polarisation = false;
void on_loop_tick()
{
    dispatch_event(state_machines, 2);
    //HAL_ADC_Start_IT(&hadc2);

    if (has_timer_tick1kHz_compared) {
        has_timer_tick1kHz_compared = false;

        tick_counter++;
        if (tick_counter == 1000) {
            tick_counter = 0; // Executed every 1 s

            // if (HAL_ADC_PollForConversion(&hadc2, 10) == HAL_OK) {
            //     int32_t sample_value = (int32_t) HAL_ADC_GetValue(&hadc2);
            //     float sample_voltage = map(sample_value, 3200.0f, 31655.0f, -0.2112f, -1.6369f);
            //     uint8_t message[32];
            //     sprintf((char *) message, "ADC: %lu (%1.4f V)\n\r", sample_value, sample_voltage);
            //     dispatch_command_to_device(COMMAND_PRINT, message);
            //     HAL_ADC_Start(&hadc2);
            // }
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == timer_regulation.Instance) {
        // if (sequence_process.machine.State == &sequence_process_states[MEASUREMENT_REGULATION_STATE]
        //     || sequence_process.machine.State == &sequence_process_states[CALIBRATION_STATE]) {
        //     HAL_ADC_Start_IT(&hadc2);
        // }
    }

    if (htim->Instance == timer_1kHz.Instance) {
        has_timer_tick1kHz_compared = 1;

        if (sequence_process.machine.State == &sequence_process_states[MEASUREMENT_REGULATION_STATE]
            || sequence_process.machine.State == &sequence_process_states[CALIBRATION_STATE]) {
            HAL_ADC_Start_IT(&hadc2);
        }
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    uint32_t value = HAL_ADC_GetValue(&hadc2);
    // map value to real

    if (sequence_process.machine.State == &sequence_process_states[MEASUREMENT_REGULATION_STATE]) {
        float delta_sample = (float) value - calibration_mean;
        float sample_voltage = map(delta_sample, 3200.0f, 31655.0f, -0.2112f, -1.6369f);
        float sample_distance = sample_voltage / SENSOR_SENSITIVITY;

        sequence_samples[sample_counter] = sample_distance;
        regulation_samples[regulation_sample_counter] = sample_distance;

        sample_counter++;
        regulation_sample_counter++;

        if (regulation_sample_counter >= REGULATION_SAMPLES_COUNT) {
            regulation_sample_counter = 0;
            // regulate

            has_changed_polarisation = !has_changed_polarisation;
            if (has_changed_polarisation) {
                __HAL_TIM_SET_COMPARE(&timer_electromagnet_left, TIM_CHANNEL_1, 250);
                __HAL_TIM_SET_COMPARE(&timer_electromagnet_right, TIM_CHANNEL_1, 0);
            } else {
                __HAL_TIM_SET_COMPARE(&timer_electromagnet_left, TIM_CHANNEL_1, 0);
                __HAL_TIM_SET_COMPARE(&timer_electromagnet_right, TIM_CHANNEL_1, 750);
            }
        }

        if (sample_counter >= SEQUENCE_SAMPLES_COUNT) {
            sample_counter = 0;
            regulation_completed(&sequence_process);
        }
        return;
    }

    if (sequence_process.machine.State == &sequence_process_states[CALIBRATION_STATE]) {
        calibration_samples[sample_counter] = value;
        sample_counter++;
        if (sample_counter >= CALIBRATION_SAMPLES_COUNT) {
            sample_counter = 0;
            measurement_completed(&sequence_process);

            uint32_t sum = 0;
            for (int i = 0; i < CALIBRATION_SAMPLES_COUNT; i++) {
                sum += calibration_samples[i];
            }
            calibration_mean = sum / (float) CALIBRATION_SAMPLES_COUNT;
        }
        return;
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (Size == 1) {
        dispatch_command_to_device((to_device_command_t) rx_buffer[0], NULL);
    }

    HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_buffer, 64);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    positioner_location_t positioner_location;

    switch (GPIO_Pin) {
    case BLUE_PUSH_BUTTON_Pin:
        current_button_state = HAL_GPIO_ReadPin(BLUE_PUSH_BUTTON_GPIO_Port, BLUE_PUSH_BUTTON_Pin);
        if (previous_button_state == GPIO_PIN_SET && current_button_state == GPIO_PIN_RESET) {
            start_moving(&positioner_process, TARGET_LOCATION);
        }
        previous_button_state = current_button_state;
        return;

    case POSITIONER_1_Pin:
        positioner_location = LOCATION_1;
        break;
    case POSITIONER_2_Pin:
        positioner_location = LOCATION_2;
        break;
    case POSITIONER_3_Pin:
        positioner_location = LOCATION_3;
        break;
    case POSITIONER_4_Pin:
        positioner_location = LOCATION_4;
        break;
    case POSITIONER_5_Pin:
        positioner_location = LOCATION_5;
        break;
    case POSITIONER_6_Pin:
        positioner_location = LOCATION_6;
        break;
    case POSITIONER_7_Pin:
        positioner_location = LOCATION_7;
        break;
    case POSITIONER_8_Pin:
        positioner_location = LOCATION_8;
        break;
    case OPTICAL_ENDSTOP_LEFT_Pin:
        positioner_location = LIMIT_LEFT;
        limit_reached(&positioner_process, LIMIT_LEFT);
        return;
    case OPTICAL_ENDSTOP_RIGHT_Pin:
        positioner_location = LIMIT_RIGHT;
        limit_reached(&positioner_process, LIMIT_RIGHT);
        return;
    default:
        return;
    }

    // TODO move limiters to new_position_reached
    new_position_reached(&positioner_process, positioner_location);
}