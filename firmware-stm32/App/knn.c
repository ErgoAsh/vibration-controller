#include "knn.h"
#include "stm32h7xx_hal.h"

#include "array_a.h"
#include "array_v.h"
#include "array_x.h"
#include "math.h"
#include "tim.h"

#include <stdlib.h>

uint32_t time_elapsed[MAX_ITERATIONS];
uint32_t sum_time_elapsed = 0;
float time_mean;
float best_u[MAX_ITERATIONS];

float sum_u = 0;
float sum_distance = 0;
float u = 0;

int result_indices_x[8];
int result_indices_v[8];
int result_indices_a[8];

uint32_t iteration = 0;

void test_knn()
{
    data_point_t compared_point = {0, 351.0, 112.0, 15.0};

    iteration = 0;
    for (iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
        sum_u = 0;
        sum_distance = 0;
        u = 0;

        uint32_t time_1 = 0;
        HAL_TIM_Base_Stop(&timer_software_profiler);
        HAL_TIM_Base_Start(&timer_software_profiler);
        __HAL_TIM_SET_COUNTER(&timer_software_profiler, 100);
        find_closest_records_x(points_x, MAX_POINTS, compared_point.x, result_indices_x);
        find_closest_records_v(points_v, MAX_POINTS, compared_point.v, result_indices_v);
        find_closest_records_a(points_a, MAX_POINTS, compared_point.a, result_indices_a);

        data_point_t points[MAX_COMPARED_POINTS * 3];
        for (int i = 0; i < MAX_COMPARED_POINTS; i++) {
            points[i] = points_x[result_indices_x[MAX_COMPARED_POINTS * 1 - i]];
            calculate_distance(&points[i], compared_point.x, compared_point.v, compared_point.a);
        }

        for (int i = MAX_COMPARED_POINTS; i < MAX_COMPARED_POINTS * 2; i++) {
            points[i] = points_v[result_indices_v[MAX_COMPARED_POINTS * 2 - i]];
            calculate_distance(&points[i], compared_point.x, compared_point.v, compared_point.a);
        }

        for (int i = MAX_COMPARED_POINTS * 2; i < MAX_COMPARED_POINTS * 3; i++) {
            points[i] = points_a[result_indices_a[MAX_COMPARED_POINTS * 3 - i]];
            calculate_distance(&points[i], compared_point.x, compared_point.v, compared_point.a);
        }

        qsort(points, MAX_COMPARED_POINTS * 3, sizeof(data_point_t), compare_points);

        sum_u += points[0].u;
        sum_u += points[1].u;
        sum_u += points[2].u;
        sum_u += points[3].u;

        sum_distance += points[0].distance;
        sum_distance += points[1].distance;
        sum_distance += points[2].distance;
        sum_distance += points[3].distance;

        u += (points[0].u * points[0].distance) / sum_distance;
        u += (points[1].u * points[1].distance) / sum_distance;
        u += (points[2].u * points[2].distance) / sum_distance;
        u += (points[3].u * points[3].distance) / sum_distance;
        u = u / 4;

        uint32_t time_2 = __HAL_TIM_GET_COUNTER(&timer_software_profiler);
        HAL_TIM_Base_Stop(&timer_software_profiler);
        // Calc sum time
        time_elapsed[iteration] = time_2 - time_1;
        sum_time_elapsed += time_elapsed[iteration];
    }
    // Calc mean time
    time_mean = (float) sum_time_elapsed / MAX_ITERATIONS;
}

void calculate_distance(data_point_t *point, float target_x, float target_v, float target_a)
{
    point->distance = sqrt((point->x - target_x) * (point->x - target_x) + (point->v - target_v) * (point->v - target_v)
                           + (point->a - target_a) * (point->a - target_a));
}

int compare_points(const void *a, const void *b)
{
    data_point_t *pointA = (data_point_t *) a;
    data_point_t *pointB = (data_point_t *) b;
    return (pointA->distance - pointB->distance);
}

// Function to find the index of the element with the closest x value using binary search
int find_closest_x(data_point_t arr[], int size, float target)
{
    uint32_t index = 0;
    uint32_t k = MAX_POINT_POWER;
    for (int i = k - 1; i >= 0; i--) {
        float value = arr[index | (1 << i)].x;
        if (value <= target) {
            index |= (1 << i);
        }
    }
    return index;
}

void find_closest_records_x(data_point_t arr[], int size, float target, int resultIndices[])
{
    int closestIndex = find_closest_x(arr, size, target);
    int left = closestIndex - 1, right = closestIndex + 1;
    int count = 0;

    resultIndices[count++] = closestIndex;

    while (count < MAX_COMPARED_POINTS) {
        if (left >= 0 && right < size) {
            if (fabs(arr[left].x - target) < fabs(arr[right].x - target)) {
                resultIndices[count++] = left--;
            } else {
                resultIndices[count++] = right++;
            }
        } else if (left >= 0) {
            resultIndices[count++] = left--;
        } else if (right < size) {
            resultIndices[count++] = right++;
        } else {
            break;
        }
    }
}

// Function to find the index of the element with the closest x value using binary search
int find_closest_v(data_point_t arr[], int size, float target)
{
    uint32_t index = 0;
    uint32_t k = MAX_POINT_POWER;
    for (int i = k - 1; i >= 0; i--) {
        float value = arr[index | (1 << i)].v;
        if (value <= target) {
            index |= (1 << i);
        }
    }
    return index;
}

void find_closest_records_v(data_point_t arr[], int size, float target, int resultIndices[])
{
    int closestIndex = find_closest_v(arr, size, target);
    int left = closestIndex - 1, right = closestIndex + 1;
    int count = 0;

    resultIndices[count++] = closestIndex;

    while (count < MAX_COMPARED_POINTS) {
        if (left >= 0 && right < size) {
            if (fabs(arr[left].v - target) < fabs(arr[right].v - target)) {
                resultIndices[count++] = left--;
            } else {
                resultIndices[count++] = right++;
            }
        } else if (left >= 0) {
            resultIndices[count++] = left--;
        } else if (right < size) {
            resultIndices[count++] = right++;
        } else {
            break;
        }
    }
}

// Function to find the index of the element with the closest x value using binary search
int find_closest_a(data_point_t arr[], int size, float target)
{
    uint32_t index = 0;
    uint32_t k = MAX_POINT_POWER;
    for (int i = k - 1; i >= 0; i--) {
        float value = arr[index | (1 << i)].a;
        if (value <= target) {
            index |= (1 << i);
        }
    }
    return index;
}

void find_closest_records_a(data_point_t arr[], int size, float target, int resultIndices[])
{
    int closestIndex = find_closest_a(arr, size, target);
    int left = closestIndex - 1, right = closestIndex + 1;
    int count = 0;

    resultIndices[count++] = closestIndex;

    while (count < MAX_COMPARED_POINTS) {
        if (left >= 0 && right < size) {
            if (fabs(arr[left].a - target) < fabs(arr[right].a - target)) {
                resultIndices[count++] = left--;
            } else {
                resultIndices[count++] = right++;
            }
        } else if (left >= 0) {
            resultIndices[count++] = left--;
        } else if (right < size) {
            resultIndices[count++] = right++;
        } else {
            break;
        }
    }
}

//
//void calculate_distance(data_point_t *in_point, float target_x, float target_v,
//		float target_a)
//{
//	in_point->distance = sqrt(
//			(in_point->x - target_x) * (in_point->x - target_x)
//					+ (in_point->v - target_v) * (in_point->v - target_v)
//					+ (in_point->a - target_a) * (in_point->a - target_a));
//}
//

//
//float find_nearest_points(float target_x, float target_v, float target_a)
//{
//	data_point_t points[MAX_POINTS];
//	sum_distance = 0;
//	sum_u = 0;
//	num_points = 0;
//
//	for (int i = 0; i < MAX_POINTS; i++)
//	{
//		data_point_t point =
//		{ .index = i, .x = x_data[i], .v = v_data[i], .a = a_data[i], .u =
//				u_data[i], };
//		calculate_distance(&point, target_x, target_v, target_a);
//		points[num_points++] = point;
//	}
//
//	qsort(points, num_points, sizeof(data_point_t), compare_points);
//
//	for (int i = 0; i < 4 && i < num_points; i++)
//	{
//		// printf("Point %d: x = %f, v = %f, a = %f, u = %f, distance = %f\n", i +
//		// 1, points[i].x, points[i].v, points[i].a, points[i].u,
//		// points[i].distance);
//		sum_u += points[i].u;
//		sum_distance += points[i].distance;
//	}
//
//	float u = 0;
//	for (int i = 0; i < 4 && i < num_points; i++)
//	{
//		u = (points[i].u * points[i].distance) / sum_distance;
//	}
//
//	// printf("Najlepsze nastawy wynosza: %f", u);
//	return u;
//}
//
//float find_nearest_points_expanded(float target_x, float target_v,
//		float target_a)
//{
//	data_point_t points[MAX_POINTS];
//	sum_distance = 0;
//	sum_u = 0;
//	num_points = 0;
//
//	point = (data_point_t
//			)
//			{ .index = 0, .x = x_data[0], .v = v_data[0], .a = a_data[0], .u =
//					u_data[0], };
//	calculate_distance(&point, target_x, target_v, target_a);
//	points[num_points++] = point;
//	point = (data_point_t
//			)
//			{ .index = 5999, .x = x_data[5999], .v = v_data[5999], .a =
//					a_data[5999], .u = u_data[5999], };
//	calculate_distance(&point, target_x, target_v, target_a);
//	points[num_points++] = point;
//
//	qsort(points, num_points, sizeof(data_point_t), compare_points);
//
//	sum_u += points[0].u;
//	sum_distance += points[1].distance;
//	sum_u += points[1].u;
//	sum_distance += points[1].distance;
//	sum_u += points[2].u;
//	sum_distance += points[2].distance;
//	sum_u += points[3].u;
//	sum_distance += points[3].distance;
//
//	u = (points[0].u * points[0].distance) / sum_distance;
//	u = (points[1].u * points[1].distance) / sum_distance;
//	u = (points[2].u * points[2].distance) / sum_distance;
//	u = (points[3].u * points[3].distance) / sum_distance;
//
//	// printf("Najlepsze nastawy wynosza: %f", u);
//	return u;
//}
