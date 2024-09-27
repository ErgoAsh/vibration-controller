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
uint32_t regulation_setpoints_counter = 0;

uint32_t tick_counter = 0;
float calibration_mean = 0;
bool has_timer_tick1kHz_compared = false;

uint8_t rx_buffer[64];
uint8_t tx_buffer[64];
bool has_usart_dma_rx_finished = false;
bool has_usart_dma_tx_finished = false;

volatile float sequence_samples[SEQUENCE_SAMPLES_COUNT];
volatile uint16_t calibration_samples[CALIBRATION_SAMPLES_COUNT];
volatile float regulation_samples[REGULATION_SAMPLES_COUNT];
volatile float regulation_setpoints[SEQUENCE_SAMPLES_COUNT / REGULATION_SAMPLES_COUNT];

GPIO_PinState previous_button_state = GPIO_PIN_SET;
GPIO_PinState current_button_state = GPIO_PIN_SET;

state_machine_t *state_machines[] = {
    (state_machine_t *) &positioner_process,
    (state_machine_t *) &sequence_process,
};

void on_initialize()
{
    HAL_UARTEx_ReceiveToIdle_IT(&uart, rx_buffer, 64);
    HAL_TIM_Base_Start_IT(&timer_1kHz);
    HAL_TIM_Base_Start_IT(&timer_regulation);
    HAL_TIM_PWM_Start(&timer_electromagnet_left, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&timer_electromagnet_right, TIM_CHANNEL_1);

    init_positioner_process(&positioner_process);
    init_sequence_process(&sequence_process);
    test_knn();

    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "Device has started running\n\r");
}

void on_loop_tick()
{
    dispatch_event(state_machines, 2);

    if (has_timer_tick1kHz_compared) {
        has_timer_tick1kHz_compared = false;

        tick_counter++;
        if (tick_counter == 1000) {
            tick_counter = 0; // Executed every 1 s

            // use if needed
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
    HAL_ADC_Stop_IT(&hadc2);

    if (sequence_process.machine.State == &sequence_process_states[MEASUREMENT_REGULATION_STATE]) {
        float delta_sample = (float) value - calibration_mean;

#if (USE_REAL_UNITS)
        float sample_voltage = map(delta_sample, 3200.0f, 31655.0f, -0.2112f, -1.6369f);
        float sample_distance = sample_voltage / SENSOR_SENSITIVITY / 1000 / 1000;
        sequence_samples[sample_counter] = sample_distance;
        regulation_samples[regulation_sample_counter] = sample_distance;
#else
        sequence_samples[sample_counter] = delta_sample;
        regulation_samples[regulation_sample_counter] = delta_sample;
#endif

        sample_counter++;
        regulation_sample_counter++;

        //if (sample_counter >= 3) {
        if (regulation_sample_counter >= REGULATION_SAMPLES_COUNT) {
            regulation_sample_counter = 0;

            data_point_t point = {
                .x = regulation_samples[REGULATION_SAMPLES_COUNT],
                .v = (regulation_samples[REGULATION_SAMPLES_COUNT] - regulation_samples[REGULATION_SAMPLES_COUNT - 1])
                     / DELTA_TIME,
                .a = (regulation_samples[REGULATION_SAMPLES_COUNT] - 2 * regulation_samples[REGULATION_SAMPLES_COUNT - 1]
                      + regulation_samples[REGULATION_SAMPLES_COUNT - 2])
                     / (DELTA_TIME * DELTA_TIME),
            };
#if (REGULATION_BY_SAMPLE_ARRAY_ENABLED)
            float u = regulate_with_sample_data(point);
#else
            float u = regulate_individuals_data(point);
#endif
            regulation_setpoints[regulation_setpoints_counter] = u;
            regulation_setpoints_counter++;

#if (REGULATION_ENABLED)

            if (u < 0) {
                __HAL_TIM_SET_COMPARE(&timer_electromagnet_left, TIM_CHANNEL_1, (int) roundf(-u));
                __HAL_TIM_SET_COMPARE(&timer_electromagnet_right, TIM_CHANNEL_1, 0);
            } else {
                __HAL_TIM_SET_COMPARE(&timer_electromagnet_left, TIM_CHANNEL_1, 0);
                __HAL_TIM_SET_COMPARE(&timer_electromagnet_right, TIM_CHANNEL_1, (int) roundf(u));
            }

#endif
        }

        if (sample_counter >= SEQUENCE_SAMPLES_COUNT) {
            sample_counter = 0;
            regulation_setpoints_counter = 0;
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
    if (huart->Instance == uart.Instance) {
        if (Size == 1) {
            dispatch_command_to_device((to_device_command_t) rx_buffer[0], NULL);
            HAL_UARTEx_ReceiveToIdle_IT(&uart, rx_buffer, 64);
        } else {
            int a = 0;
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == uart.Instance) {
        has_usart_dma_rx_finished = true;
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == uart.Instance) {
        has_usart_dma_tx_finished = true;
    }
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