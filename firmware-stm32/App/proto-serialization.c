#include "proto-serialization.h"

#include "globals.h"
#include "knn.h"
#include "usart.h"

ssize_t buffer_len = 0;
uint8_t buf[4096];

// Original solution: https://community.st.com/t5/stm32-mcus/dma-is-not-working-on-stm32h7-devices/ta-p/49498
// DMA cannot access 0x20000000 so we have to manually change it to 0x24000000
// Add this to .ld linker file when CubeMX override it:
//   .dma_buffer : /* Space before ':' is critical */
//   {
//     *(.dma_buffer)
//   } >RAM
#if defined(__ICCARM__)
#define DMA_BUFFER _Pragma("location=\".dma_buffer\"")
#else
#define DMA_BUFFER __attribute__((section(".dma_buffer")))
#endif
DMA_BUFFER uint8_t frame[16];

int serialize_sequence_data(uint8_t *output_buffer, size_t *output_buffer_len)
{
    // float dt = DELTA_TIME; // Period between two measurements

    // struct capn c;
    // capn_init_malloc(&c);

    // // Create pointer to root structure
    // capn_ptr cr = capn_root(&c);
    // struct capn_segment *cs = cr.seg;

    // // Create pointer to data
    // SequenceData_ptr sdp = new_SequenceData(cs);
    // struct SequenceData sequence_data;

    // Point_list points = new_Point_list(cs, SEQUENCE_SAMPLES_COUNT);
    // sequence_data.data = points;

    // // Point without velocity or acceleration
    // struct Point p0 = {
    //     .index = 0,
    //     .x = sequence_samples[0],
    //     .v = 0,
    //     .a = 0,
    //     .u = 2137, // FIXME
    // };
    // set_Point(&p0, points, 0);

    // // Point without acceleration
    // struct Point p1 = {
    //     .index = 1,
    //     .x = sequence_samples[1],
    //     .v = (sequence_samples[2] - sequence_samples[1]) / dt,
    //     .a = 0,
    //     .u = 2137, // FIXME
    // };
    // set_Point(&p1, points, 1);

    // for (int i = 2; i < SEQUENCE_SAMPLES_COUNT; i++) {
    //     float x = sequence_samples[i];
    //     float v = (sequence_samples[i] - sequence_samples[i - 1]) / dt;
    //     float a = (v - ((sequence_samples[i - 1] - sequence_samples[i - 2]) / dt)) / dt;

    //     struct Point p = {
    //         .index = i,
    //         .x = x,
    //         .v = v,
    //         .a = a,
    //         .u = 2137, // FIXME
    //     };
    //     set_Point(&p, points, i);
    // }

    // write_SequenceData(&sequence_data, sdp);
    // int setp_ret = capn_setp(capn_root(&c), 0, sdp.p);

    // // Write structure data to buffer
    // *output_buffer_len = capn_write_mem(&c, output_buffer, 4096, 0 /* packed */);

    // capn_free(&c);
    // return setp_ret;
    return 0;
}

int deserialize_sequence_data(sequence_data_t *output_data, uint8_t *input_buffer, size_t input_buffer_len)
{
    // // Create capnproto object
    // struct capn c;
    // int init_mem_ret = capn_init_mem(&c, input_buffer, input_buffer_len, 0 /* packed */);

    // // Create pointer to root structure
    // SequenceData_ptr root_p;
    // root_p.p = capn_getp(capn_root(&c), 0, 1);
    // read_SequenceData(output_data, root_p);

    // capn_free(&c);
    // return init_mem_ret;
    return 0;
}

void wait_for_dma()
{
    while (!has_usart_dma_finished) {
        __NOP();
    }
    has_usart_dma_finished = false;
}

float fir_coeff[16] = {
    0.009374095127456596,
    0.015248989077090274,
    0.031685501000184441,
    0.056109929517806932,
    0.083909101523129026,
    0.109443176158426622,
    0.127342907899538227,
    0.133772599392735642,
    0.127342907899538227,
    0.109443176158426636,
    0.083909101523129040,
    0.056109929517807001,
    0.031685501000184454,
    0.015248989077090261,
    0.009374095127456596,
};

void fir_filter(float *input, float *output, int buffer_size, float *coeff, int tap_num)
{
    for (int i = 0; i < buffer_size; i++) {
        output[i] = 0; // Wyzerowanie wartości wyjściowej dla każdej próbki

        // Przeliczenie odpowiedzi filtra dla każdej próbki
        for (int j = 0; j < tap_num; j++) {
            if (i - j >= 0) { // Zapewniamy, że nie wyjdziemy poza zakres próbek
                output[i] += coeff[j] * input[i - j];
            }
        }
    }
}

void send_sequence_data()
{
    float dt = DELTA_TIME; // Period between two measurements

    //fir_filter(sequence_samples, sequence_samples, SEQUENCE_SAMPLES_COUNT, fir_coeff, 15);

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

    memcpy(&frame[0], &p0.x, sizeof(float));
    memcpy(&frame[4], &p0.v, sizeof(float));
    memcpy(&frame[8], &p0.a, sizeof(float));
    memcpy(&frame[12], &p0.u, sizeof(float));

    HAL_UART_Transmit_DMA(&uart, (uint8_t *) frame, sizeof(frame));
    wait_for_dma();

    // Point without acceleration
    data_point_t p1 = {
        .index = 1,
        .x = sequence_samples[1],
        .v = (sequence_samples[2] - sequence_samples[1]) / dt,
        .a = 0,
        .u = regulation_setpoints[0],
    };
#if !REGULATION_ENABLED
    p1.u = 0;
#endif

    memcpy(&frame[0], &p1.x, sizeof(float));
    memcpy(&frame[4], &p1.v, sizeof(float));
    memcpy(&frame[8], &p1.a, sizeof(float));
    memcpy(&frame[12], &p1.u, sizeof(float));

    HAL_UART_Transmit_DMA(&uart, (uint8_t *) frame, sizeof(frame));
    wait_for_dma();

    for (int i = 2; i < SEQUENCE_SAMPLES_COUNT; i++) {
        float dt = DELTA_TIME; // Period between two measurements
        float x = sequence_samples[i];
        float v = (sequence_samples[i] - sequence_samples[i - 1]) / dt;
        float a = (sequence_samples[i] - 2 * sequence_samples[i - 1] + sequence_samples[i - 2]) / (dt * dt);
        float u = regulation_setpoints[i / 64]; // Division without remainder
#if !REGULATION_ENABLED
        u = 0;
#endif

        memcpy(&frame[0], &x, sizeof(float));
        memcpy(&frame[4], &v, sizeof(float));
        memcpy(&frame[8], &a, sizeof(float));
        memcpy(&frame[12], &u, sizeof(float));

        HAL_UART_Transmit_DMA(&uart, (uint8_t *) frame, sizeof(frame));
        wait_for_dma();
        //HAL_Delay(1);
    }
}

void receive_individual_data()
{
    HAL_UART_Receive_DMA(&uart, (uint8_t *) frame, sizeof(frame));
    wait_for_dma();

    for (int i = 2; i < SEQUENCE_SAMPLES_COUNT; i++) {
        float dt = DELTA_TIME; // Period between two measurements
        float x = sequence_samples[i];
        float v = (sequence_samples[i] - sequence_samples[i - 1]) / dt;
        float a = (sequence_samples[i] - 2 * sequence_samples[i - 1] + sequence_samples[i - 2]) / (dt * dt);
        float u = regulation_setpoints[i / 64]; // Division without remainder
#if !REGULATION_ENABLED
        u = 0;
#endif

        memcpy(&frame[0], &x, sizeof(float));
        memcpy(&frame[4], &v, sizeof(float));
        memcpy(&frame[8], &a, sizeof(float));
        memcpy(&frame[12], &u, sizeof(float));

        HAL_UART_Transmit_DMA(&uart, (uint8_t *) frame, sizeof(frame));
        wait_for_dma();
        //HAL_Delay(1);
    }
}