
#ifndef STUDENT_MESH_H
#define STUDENT_MESH_H

#include "student_triangulation.h"

int mesh_allocate_triangles(mesh_st* mesh, uint32_t min_nelems);

int is_ghost_triangle(mesh_st* mesh, uint32_t elem_index);

int mesh_allocate_twins(mesh_st* mesh, uint32_t min_nelems);

void print_triangles(mesh_st* mesh);

#endif /* STUDENT_MESH_H */
