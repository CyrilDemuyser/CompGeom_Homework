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
