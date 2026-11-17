
#ifndef STUDENT_HILBERT_H
#define STUDENT_HILBERT_H

#include "student_triangulation.h"

static uint64_t get_point_key(uint32_t* point_id_ptr, const void* user_data);

void sort_points_by_hilbert(uint32_t* points_order, uint32_t npoints, const uint64_t* hilbert_keys);

void compute_hilbert_keys(double* points, uint32_t npoints, uint64_t* hilbert_keys, uint32_t depth);

static void swap(double *a, double *b);

void hilbertCoordinates ( double x, double y,
			  double x0, double y0,
			  double xBlue, double yBlue,
			  double xRed , double yRed,
			  int Depth, int bits [] );

#endif /* STUDENT_HILBERT_H */
