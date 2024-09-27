#include "proto-serialization.h"

#include "commands.h"
#include "globals.h"
#include "knn.h"
#include "usart.h"

// Original solution: https://community.st.com/t5/stm32-mcus/dma-is-not-working-on-stm32h7-devices/ta-p/49498
// DMA cannot access 0x20000000 so we have to manually change it to 0x24000000
// Add this to .ld linker file when CubeMX override it:
//   .dma_buffer : /* Space before ':' is critical */
//   {
//     *(.dma_buffer)
//   } >RAM
#if defined(__ICCARM__)
#define DMA_BUFFER _Pragma("location=\".large_buffer\"")
#else
#define DMA_BUFFER __attribute__((section(".large_buffer")))
#endif

volatile DMA_BUFFER uint8_t frame_rx[16];
volatile DMA_BUFFER uint8_t frame_tx[16];

void wait_for_dma_tx()
{
    while (!has_usart_dma_tx_finished) {
        __NOP();
    }
    has_usart_dma_tx_finished = false;
}

void wait_for_dma_rx()
{
    while (!has_usart_dma_rx_finished) {
        __NOP();
    }
    has_usart_dma_rx_finished = false;
}

void send_sequence_data()
{
    float dt = DELTA_TIME; // Period between two measurements

    // Point without velocity or acceleration
    data_point_t p0 = {
        .index = 0,
        .x = sequence_samples[0],
        .v = 0,
        .a = 0,
        .u = regulation_setpoints[0],
    };
#if !REGULATION_ENABLED
    p0.u = 0;
#endif

    memcpy(&frame_tx[0], &p0.x, sizeof(float));
    memcpy(&frame_tx[4], &p0.v, sizeof(float));
    memcpy(&frame_tx[8], &p0.a, sizeof(float));
    memcpy(&frame_tx[12], &p0.u, sizeof(float));

    HAL_UART_Transmit_DMA(&uart, (uint8_t *) frame_tx, sizeof(frame_tx));
    wait_for_dma_tx();

    // Point without acceleration
    data_point_t p1 = {
        .index = 1,
        .x = sequence_samples[1],
        .v = (sequence_samples[2] - sequence_samples[1]) / dt,
        .a = 0,
        .u = regulation_setpoints[1],
    };
#if !REGULATION_ENABLED
    p1.u = 0;
#endif

    memcpy(&frame_tx[0], &p1.x, sizeof(float));
    memcpy(&frame_tx[4], &p1.v, sizeof(float));
    memcpy(&frame_tx[8], &p1.a, sizeof(float));
    memcpy(&frame_tx[12], &p1.u, sizeof(float));

    HAL_UART_Transmit_DMA(&uart, (uint8_t *) frame_tx, sizeof(frame_tx));
    wait_for_dma_tx();

    for (int i = 2; i < SEQUENCE_SAMPLES_COUNT; i++) {
        float dt = DELTA_TIME; // Period between two measurements
        float x = sequence_samples[i];
        float v = (sequence_samples[i] - sequence_samples[i - 1]) / dt;
        float a = (sequence_samples[i] - 2 * sequence_samples[i - 1] + sequence_samples[i - 2]) / (dt * dt);
        //float u = regulation_setpoints[i / REGULATION_SETPOINT_COUNT]; // Division without remainder
        float u = 2137;
#if !REGULATION_ENABLED
        u = 0;
#endif

        memcpy(&frame_tx[0], &x, sizeof(float));
        memcpy(&frame_tx[4], &v, sizeof(float));
        memcpy(&frame_tx[8], &a, sizeof(float));
        memcpy(&frame_tx[12], &u, sizeof(float));

        HAL_UART_Transmit_DMA(&uart, (uint8_t *) frame_tx, sizeof(frame_tx));
        wait_for_dma_tx();
        //HAL_Delay(1);
    }
}

int compare_by_x(const void *a, const void *b)
{
    data_point_t *pointA = (data_point_t *) a;
    data_point_t *pointB = (data_point_t *) b;

    if (pointA->x < pointB->x)
        return -1;
    else if (pointA->x > pointB->x)
        return 1;
    else
        return 0;
}

int compare_by_v(const void *a, const void *b)
{
    data_point_t *pointA = (data_point_t *) a;
    data_point_t *pointB = (data_point_t *) b;

    if (pointA->v < pointB->v)
        return -1;
    else if (pointA->v > pointB->v)
        return 1;
    else
        return 0;
}

int compare_by_a(const void *a, const void *b)
{
    data_point_t *pointA = (data_point_t *) a;
    data_point_t *pointB = (data_point_t *) b;

    if (pointA->a < pointB->a)
        return -1;
    else if (pointA->a > pointB->a)
        return 1;
    else
        return 0;
}

void receive_individual_data()
{
    for (int i = 0; i < 16; i++) {
        frame_rx[i] = 0;
    }
    uint32_t *addr = (uint32_t *) &frame_rx;
    uint32_t *addr2 = (uint32_t *) frame_rx;
    uint32_t *addr63 = (uint32_t *) &frame_tx;
    uint32_t *addr6 = (uint32_t *) frame_tx;
    uint16_t length = 0;
    HAL_UARTEx_ReceiveToIdle(&uart, (uint8_t *) &frame_rx, sizeof(frame_rx), &length, HAL_MAX_DELAY);
    //wait_for_dma_rx();

    memcpy(&length, &frame_rx, sizeof(uint16_t));
    for (int i = 0; i < 16; i++) {
        frame_rx[i] = 0;
    }

    for (int i = 0; i < length; i++) {
        HAL_UART_Receive(&uart, (uint8_t *) frame_rx, sizeof(frame_rx), HAL_MAX_DELAY);
        //wait_for_dma_tx();

        data_point_t *points = get_points_sorted_by_x();
        memcpy(&points[i].x, &frame_rx[0], sizeof(float));
        memcpy(&points[i].v, &frame_rx[4], sizeof(float));
        memcpy(&points[i].a, &frame_rx[8], sizeof(float));
        memcpy(&points[i].u, &frame_rx[12], sizeof(float));
    }

    memcpy(get_points_sorted_by_v(), get_points_sorted_by_x(), sizeof(data_point_t) * 4096);
    memcpy(get_points_sorted_by_a(), get_points_sorted_by_x(), sizeof(data_point_t) * 4096);
    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "STOP: Individual regulation data has been sent\n\r");

    qsort(get_points_sorted_by_x(), 4096, sizeof(data_point_t), compare_by_x);
    qsort(get_points_sorted_by_v(), 4096, sizeof(data_point_t), compare_by_v);
    qsort(get_points_sorted_by_a(), 4096, sizeof(data_point_t), compare_by_a);
    dispatch_command_to_host(COMMAND_PRINT_ON_CONSOLE, "INFO: Individual regulation data has been sorted\n\r");

    HAL_UARTEx_ReceiveToIdle_IT(&uart, rx_buffer, 64);
}