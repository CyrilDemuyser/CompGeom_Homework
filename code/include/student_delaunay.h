
#ifndef STUDENT_DELAUNAY_H
#define STUDENT_DELAUNAY_H

#include "student_triangulation.h"


void create_initial_triangle(mesh_st* mesh);

uint32_t stochastic_visibility_walk(mesh_st* mesh, double* point, uint32_t previous_elem);

#endif /* STUDENT_DELAUNAY_H */
