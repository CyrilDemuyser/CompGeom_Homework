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
        return -1; // Invalid index
    }
    for (int i = 0; i < 3; i++) {
        if (mesh->elems[3 * elem_index + i] == VERTEX_INFINITY) {
            return 1; // It's a ghost triangle
        }
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

    for (int i = 0; i < 3; i++){
        mesh->elems[3*T + i] = mesh->elems[3*last + i];
        int current_twin = mesh->elems[3*last + i];
        mesh->elems[3*T + i] = current_twin;
        if (current_twin != -1){
            mesh->twin[current_twin] = 3*T + i;
        }
    }
    
    mesh->nelems -= 1;
    return 0;
}



// Renvoie 1 si tous les triangles passent le test du cercle vide, 0 sinon.
// FONCTION FULL CHATGPT !!
int check_empty_circles(mesh_st *mesh){
    for (uint32_t t = 0; t < mesh->nelems; ++t) {
        uint32_t a = mesh->elems[3 * t];
        uint32_t b = mesh->elems[3 * t + 1];
        uint32_t c = mesh->elems[3 * t + 2];

        // Détecte aussi un ghost qui aurait survécu à discard_ghost.
        if (a >= mesh->nnodes || b >= mesh->nnodes || c >= mesh->nnodes) {
            printf("Triangle %u : indice de sommet invalide.\n", t);
            return 0;
        }

        double *A = &mesh->nodes[2 * a];
        double *B = &mesh->nodes[2 * b];
        double *C = &mesh->nodes[2 * c];

        // Ton maillage stocke les triangles finis dans le sens antihoraire.
        if (orient2d(A, B, C) <= 0) {
            printf("Triangle %u (%u, %u, %u) : plat ou mal oriente.\n",
                   t, a, b, c);
            return 0;
        }

        for (uint32_t p = 0; p < mesh->nnodes; ++p) {
            if (p == a || p == b || p == c)
                continue;

            double *P = &mesh->nodes[2 * p];
            if (incircle(A, B, C, P) > 0) {
                printf("Violation de Delaunay : le point %u est dans "
                       "le cercle du triangle %u (%u, %u, %u).\n",
                       p, t, a, b, c);
                return 0;
            }
        }
    }
    return 1;
}