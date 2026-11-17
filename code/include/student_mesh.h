
#ifndef STUDENT_MESH_H
#define STUDENT_MESH_H

#include "student_triangulation.h"

int mesh_allocate_triangles(mesh_st* mesh, uint32_t min_nelems);

int is_ghost_triangle(mesh_st* mesh, uint32_t elem_index);

#endif /* STUDENT_MESH_H */
