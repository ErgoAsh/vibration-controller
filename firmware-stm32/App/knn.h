#ifndef KNN_H_
#define KNN_H_

#include "globals.h"

#define MAX_ITERATIONS 100
#define MAX_POINT_POWER 12
#define MAX_POINTS 2 ^ MAX_POINT_POWER
#define MAX_COMPARED_POINTS 8
#define NUM_CLOSEST 4

extern uint32_t time_elapsed[MAX_ITERATIONS];

float regulate_with_sample_data(data_point_t compared_point);
float regulate_individuals_data(data_point_t compared_point);
void test_knn();

int find_closest_x(data_point_t arr[], int size, float target);
void find_closest_records_x(data_point_t arr[], int size, float target, int resultIndices[]);

int find_closest_v(data_point_t arr[], int size, float target);
void find_closest_records_v(data_point_t arr[], int size, float target, int resultIndices[]);

int find_closest_a(data_point_t arr[], int size, float target);
void find_closest_records_a(data_point_t arr[], int size, float target, int resultIndices[]);

void calculate_distance(data_point_t *point, float target_x, float target_v, float target_a);
int compare_points(const void *a, const void *b);

data_point_t *get_points_sorted_by_x();
data_point_t *get_points_sorted_by_v();
data_point_t *get_points_sorted_by_a();

//float find_nearest_points(float target_x, float target_v, float target_a);
//float find_nearest_points_expanded(float target_x, float target_v,
//                                   float target_a);
#endif /* KNN_H_ */
