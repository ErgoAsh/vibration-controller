#include "sequence_data.capnp.h"
/* AUTO GENERATED - DO NOT EDIT */
#ifdef __GNUC__
# define capnp_unused __attribute__((unused))
# define capnp_use(x) (void) (x);
#else
# define capnp_unused
# define capnp_use(x)
#endif


Point_ptr new_Point(struct capn_segment *s) {
	Point_ptr p;
	p.p = capn_new_struct(s, 24, 0);
	return p;
}
Point_list new_Point_list(struct capn_segment *s, int len) {
	Point_list p;
	p.p = capn_new_list(s, len, 24, 0);
	return p;
}
void read_Point(struct Point *s capnp_unused, Point_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->index = capn_read32(p.p, 0);
	s->x = capn_to_f32(capn_read32(p.p, 4));
	s->v = capn_to_f32(capn_read32(p.p, 8));
	s->a = capn_to_f32(capn_read32(p.p, 12));
	s->u = capn_read8(p.p, 16);
}
void write_Point(const struct Point *s capnp_unused, Point_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_write32(p.p, 0, s->index);
	capn_write32(p.p, 4, capn_from_f32(s->x));
	capn_write32(p.p, 8, capn_from_f32(s->v));
	capn_write32(p.p, 12, capn_from_f32(s->a));
	capn_write8(p.p, 16, s->u);
}
void get_Point(struct Point *s, Point_list l, int i) {
	Point_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_Point(s, p);
}
void set_Point(const struct Point *s, Point_list l, int i) {
	Point_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_Point(s, p);
}

SequenceData_ptr new_SequenceData(struct capn_segment *s) {
	SequenceData_ptr p;
	p.p = capn_new_struct(s, 0, 1);
	return p;
}
SequenceData_list new_SequenceData_list(struct capn_segment *s, int len) {
	SequenceData_list p;
	p.p = capn_new_list(s, len, 0, 1);
	return p;
}
void read_SequenceData(struct SequenceData *s capnp_unused, SequenceData_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	s->data.p = capn_getp(p.p, 0, 0);
}
void write_SequenceData(const struct SequenceData *s capnp_unused, SequenceData_ptr p) {
	capn_resolve(&p.p);
	capnp_use(s);
	capn_setp(p.p, 0, s->data.p);
}
void get_SequenceData(struct SequenceData *s, SequenceData_list l, int i) {
	SequenceData_ptr p;
	p.p = capn_getp(l.p, i, 0);
	read_SequenceData(s, p);
}
void set_SequenceData(const struct SequenceData *s, SequenceData_list l, int i) {
	SequenceData_ptr p;
	p.p = capn_getp(l.p, i, 0);
	write_SequenceData(s, p);
}
