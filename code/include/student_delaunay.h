
#ifndef STUDENT_DELAUNAY_H
#define STUDENT_DELAUNAY_H

#include "student_triangulation.h"



void create_initial_triangle(mesh_st* mesh, uint32_t* initial_vertices);

uint32_t stochastic_visibility_walk(mesh_st* mesh, double* point, uint32_t previous_elem);

uint32_t grow_cavity_BFS(mesh_st* mesh, double* point, uint32_t T0, uint32_t* cavity, boundary_st* boundary );

int retriangulating(mesh_st* mesh, uint32_t p, uint32_t* cavity, boundary_st* boundary, int cavity_count);

int discard_ghost(mesh_st *mesh);

#endif /* STUDENT_DELAUNAY_H */
