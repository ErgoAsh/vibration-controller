#ifndef PROTO_SERIALIZATION_H_
#define PROTO_SERIALIZATION_H_

#include "sequence_data.capnp.h"

typedef struct SequenceData sequence_data_t;

int serialize_sequence_data(uint8_t *output_buffer, size_t *output_buffer_len);
int deserialize_sequence_data(sequence_data_t *output_data, uint8_t *input_buffer, size_t input_buffer_len);
void send_sequence_data();

#endif /* PROTO_SERIALIZATION_H_ */