// Mesh storage and connectivity kind of like triangulation and refinement but maybe more about the data structure than the algorithm.
#include "student_mesh.h"
#include <stdlib.h>
#include <stdio.h>
#include "predicates.h"

int mesh_allocate_triangles(mesh_st* mesh, uint32_t min_nelems){
    if (mesh->elems == NULL){
        mesh->elems = (uint32_t*)malloc(sizeof(uint32_t) * min_nelems);
        if (mesh->elems == NULL){
            return -1; // Allocation failed
        }
    }
    return 1; // Allocation successful
}

int mesh_allocate_twins(mesh_st* mesh, uint32_t min_nelems){
    if (mesh->twin == NULL){
        mesh->twin = (int*)malloc(sizeof(int) * min_nelems);
        if (mesh->twin == NULL){
            return -1; // Allocation failed
        }
    }
    return 1; // Allocation successful
}

int is_ghost_triangle(mesh_st* mesh, uint32_t elem_index) {
    if (elem_index >= mesh->nelems) {
        return 0; // Invalid index
    }
    if (mesh->elems[3 * elem_index + 2] == VERTEX_INFINITY) {
        return 1; // It's a ghost triangle
    }
    return 0; // Not a ghost triangle
}


void print_triangles(mesh_st* mesh) {
    for (uint32_t i = 0; i < mesh->nelems; ++i) {
        printf("Triangle %u: (%u, %u, %u)\n", i, 
               mesh->elems[3 * i], 
               mesh->elems[3 * i + 1], 
               mesh->elems[3 * i + 2]);
    }
}

int remove_triangle(mesh_st* mesh, uint32_t T){
    // Set twin of connected to -1
    if (T >= mesh->nelems){
        printf("Error in student_mesh/%s: T outside of capacity\n", __func__);
        return -1;
    }
    int he = 3 * T;
    for (int i = 0; i < 3; i++){
        int current_twin = mesh->twin[he + i];
        if (current_twin != -1){
            mesh->twin[current_twin] = -1;
        }
    }

    // Remove triangle from list
    uint32_t last = mesh->nelems - 1;
    if (T == last){
        mesh->nelems -= 1;
        return 0;
    }

    for (int i = 0; i < 3; ++i) {
        uint32_t old_he = 3 * last + i;
        uint32_t new_he = 3 * T + i;

        mesh->elems[new_he] = mesh->elems[old_he];

        int current_twin = mesh->twin[old_he];
        mesh->twin[new_he] = current_twin;

        if (current_twin != -1) {
            mesh->twin[current_twin] = (int)new_he;
        }
    }
    
    mesh->nelems -= 1;
    return 0;
}



// Renvoie 1 si tous les triangles passent le test du cercle vide, 0 sinon.
// FONCTION FULL CHATGPT !!
int check_empty_circles(mesh_st *mesh) {
    for (uint32_t t = 0; t < mesh->nelems; ++t) {
        uint32_t *v = &mesh->elems[3*t];
        double *A = &mesh->nodes[2*v[0]], *B = &mesh->nodes[2*v[1]], *C = &mesh->nodes[2*v[2]];
        if (orient2d(A, B, C) <= 0) return 0;
        for (int i = 0; i < 3; i++) {
            int tw = mesh->twin[3*t + i];
            if (tw == -1) continue;
            uint32_t opp = mesh->elems[3*(tw/3) + (tw%3 + 2) % 3]; // sommet opposé
            if (incircle(A, B, C, &mesh->nodes[2*opp]) > 0) return 0;
        }
    }
    return 1;
}