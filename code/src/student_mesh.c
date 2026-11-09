// Mesh storage and connectivity kind of like triangulation and refinement but maybe more about the data structure than the algorithm.
#include "student_mesh.h"

int mesh_allocate_triangles(mesh_st* mesh, uint32_t min_nelems){
    if (mesh->elems == NULL){
        mesh->elems = (uint32_t*)malloc(sizeof(uint32_t) * min_nelems);
        if (mesh->elems == NULL){
            return -1; // Allocation failed
        }
    }
}

int is_ghost_triangle(mesh_st* mesh, uint32_t elem_index) {
    if (elem_index >= mesh->nelems) {
        return -1; // Invalid index
    }
    for (int i = 0; i < 3; i++) {
        if (mesh->elems[3 * elem_index + i] == VERTEX_INFINITY) {
            return 1; // It's a ghost triangle
        }
    }
    return 0; // Not a ghost triangle
}
