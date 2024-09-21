#ifndef KNN_H_
#define KNN_H_

#include "stdint.h"

#define MAX_ITERATIONS 100
#define MAX_POINT_POWER 12
#define MAX_POINTS 2 ^ MAX_POINT_POWER
#define MAX_COMPARED_POINTS 8
#define NUM_CLOSEST 4

typedef struct
{
    int index;
    float x, v, a, u;
    float distance;
} data_point_t;

extern uint32_t time_elapsed[MAX_ITERATIONS];

void test_knn();

int find_closest_x(data_point_t arr[], int size, float target);
void find_closest_records_x(data_point_t arr[], int size, float target, int resultIndices[]);

int find_closest_v(data_point_t arr[], int size, float target);
void find_closest_records_v(data_point_t arr[], int size, float target, int resultIndices[]);

int find_closest_a(data_point_t arr[], int size, float target);
void find_closest_records_a(data_point_t arr[], int size, float target, int resultIndices[]);

void calculate_distance(data_point_t *point, float target_x, float target_v, float target_a);
int compare_points(const void *a, const void *b);

//float find_nearest_points(float target_x, float target_v, float target_a);
//float find_nearest_points_expanded(float target_x, float target_v,
//                                   float target_a);
#endif /* KNN_H_ */
