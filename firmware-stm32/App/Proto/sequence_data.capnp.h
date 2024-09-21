#ifndef CAPN_FCF97E9F5618CDCB
#define CAPN_FCF97E9F5618CDCB
/* AUTO GENERATED - DO NOT EDIT */
#include <capnp_c.h>

#if CAPN_VERSION != 1
#error "version mismatch between capnp_c.h and generated code"
#endif

#ifndef capnp_nowarn
# ifdef __GNUC__
#  define capnp_nowarn __extension__
# else
#  define capnp_nowarn
# endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct Point;
struct SequenceData;

typedef struct {capn_ptr p;} Point_ptr;
typedef struct {capn_ptr p;} SequenceData_ptr;

typedef struct {capn_ptr p;} Point_list;
typedef struct {capn_ptr p;} SequenceData_list;

struct Point {
	uint32_t index;
	float x;
	float v;
	float a;
	uint8_t u;
};

static const size_t Point_word_count = 3;

static const size_t Point_pointer_count = 0;

static const size_t Point_struct_bytes_count = 24;


struct SequenceData {
	Point_list data;
};

static const size_t SequenceData_word_count = 0;

static const size_t SequenceData_pointer_count = 1;

static const size_t SequenceData_struct_bytes_count = 8;


Point_ptr new_Point(struct capn_segment*);
SequenceData_ptr new_SequenceData(struct capn_segment*);

Point_list new_Point_list(struct capn_segment*, int len);
SequenceData_list new_SequenceData_list(struct capn_segment*, int len);

void read_Point(struct Point*, Point_ptr);
void read_SequenceData(struct SequenceData*, SequenceData_ptr);

void write_Point(const struct Point*, Point_ptr);
void write_SequenceData(const struct SequenceData*, SequenceData_ptr);

void get_Point(struct Point*, Point_list, int i);
void get_SequenceData(struct SequenceData*, SequenceData_list, int i);

void set_Point(const struct Point*, Point_list, int i);
void set_SequenceData(const struct SequenceData*, SequenceData_list, int i);

#ifdef __cplusplus
}
#endif
#endif
