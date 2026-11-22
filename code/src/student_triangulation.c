#include "student_triangulation.h"
#include <stdlib.h>
#include <stdio.h>
#include "student_delaunay.h"
#include "student_hilbert.h"
#include "student_mesh.h"
#include "student_refinement.h"
#include "predicates.h"

// Documentation can be found in student_triangulation.h


/*
REASONMENT BEHIND THESE FUNCTIONS:
Incremental Delaunay triangulation approach:
→ each new point is inserted into an existing triangulation
→ the affected region is retriangulated

SEQUENCE OF OPERATIONS:
1. Initialise the predicates            :   exactinit()  
→ Prepare robust geometric tests such as orient2d and incircle.
2. Allocate mesh memory                 :   mesh_allocate_triangles(), mesh_allocate_twins()
→ Reserve space for triangles, half-edges and vertex connectivity.
3. Create the initial triangulation     :   create_initial_triangle()
→ Build one real triangle and its surrounding ghost triangles.
4. Determine the insertion order        :   compute_hilbert_keys() → sort_points_by_hilbert()
→ Order the points spatially to improve the efficiency of subsequent searches.
5. Insert points one by one             :   stochastic_visibility_walk() → grow_cavity_BFS() → retriangulating()
→ Locate each point, identify the triangles to remove, and create new triangles around it.
6. Remove ghost triangles                :   discard_ghost()
→ Keep the final triangulation of the actual input points.
7. Optional: Check Delaunay condition    :   check_empty_circles()
→ Ensure that the triangulation remains valid and efficient as points are added.
*/

void mesh_triangulate(mesh_st * mesh) {
    // init the predicate
    exactinit();
    // Reuse existing memory if it exists, otherwise allocate and compute
    printf("Starting triangulation for %u nodes.\n", mesh->nnodes);
    
    uint32_t triangle_capacity = 2 * mesh->nnodes - 2; // max number of triangles in a 2D triangulation with n vertices is 2n - 2
    uint32_t half_edge_capacity = 3 * triangle_capacity; // each triangle has 3 half-edges
    
    if (mesh_allocate_triangles(mesh, half_edge_capacity) < 0 || mesh_allocate_twins(mesh, half_edge_capacity) < 0) {
        fprintf(stderr, "Allocation du maillage impossible.\n");
        return;
    }
    mesh->elems_capacity = triangle_capacity;
    mesh->startHE = malloc((size_t)mesh->nnodes * sizeof(*mesh->startHE));
    if (mesh->startHE == NULL) {
        fprintf(stderr, "Allocation de startHE impossible.\n");
        return;
    }

    for (uint32_t v = 0; v < mesh->nnodes; ++v)
        mesh->startHE[v] = UINT32_MAX;  // Aucune demi-arête associée pour l'instant
    // Create initial triangles
    uint32_t* initial_vertices = malloc(3 * sizeof(uint32_t));
    create_initial_triangle(mesh, initial_vertices);
    printf("After create initial triangle : nelems = %u\n", mesh->nelems);
    if (mesh->nelems == 0){
        return;
    }
    //printf("Initial triangle created with %u elements.\n", mesh->nelems);
    //print_triangles(mesh);
    // Hilbert
    /* FROM COURSE LESSONS NOTES:
    •	2 points that are close in space, are not necessarily close in the the curve (might be far away along the “rope”, if we were to straighten it)
    •	2 points that are close in the the curve, are close!
    → USEFUL because the next point to insert is likely to be relatively close to the previously inserted point. 
    → This can make the visibility walk faster, since it can start from the previous triangle 
    rather than searching the entire mesh from scratch.
    */
    uint32_t* points_order = (uint32_t*)malloc(sizeof(uint32_t) * mesh->nnodes);
    uint64_t* hilbert_keys = (uint64_t*)malloc(sizeof(uint64_t) * mesh->nnodes);
    compute_hilbert_keys(mesh->nodes, mesh->nnodes, hilbert_keys, 32);
    //printf("Computed Hilbert keys for %u nodes.\n", mesh->nnodes);
    sort_points_by_hilbert(points_order, mesh->nnodes, hilbert_keys);
    //printf("Sorted points by Hilbert order.\n");
    uint32_t T0 = 0; // Start with the first triangle (the initial triangle)
    boundary_st boundary = {0};
    uint32_t *cavity = malloc((size_t)triangle_capacity * sizeof(*cavity));
    uint32_t *in_cavity = calloc(triangle_capacity, sizeof(*in_cavity));

    boundary.edges   = malloc((size_t)half_edge_capacity * sizeof(*boundary.edges));
    boundary.origin  = malloc((size_t)half_edge_capacity * sizeof(*boundary.origin));
    boundary.outside = malloc((size_t)half_edge_capacity * sizeof(*boundary.outside));
    for (uint32_t i = 0; i < mesh->nnodes; i++) {
        //printf("Processing point %u (Hilbert order %u).\n", points_order[i], i);
        //print_triangles(mesh);
        uint32_t p = points_order[i];
        if (p == initial_vertices[0] || p == initial_vertices[1] || p == initial_vertices[2]) {
            continue; // Skip the initial triangle vertices
        }
        uint32_t T_current = stochastic_visibility_walk(mesh, &mesh->nodes[2 * p], T0);
        T0 = T_current;
        //printf("Point %u: Visibility walk returned triangle %u.\n", p, T0);
        if (T_current == VERTEX_INFINITY) {
            printf("Error in student_triangulation.c/%s: Visibility walk failed for point %u.\n", __FUNCTION__, p);
            continue;
        }

        // Une fois cavity et boundary.edges alloués, le BFS remplit boundary.nedges.
        uint32_t cavity_count = grow_cavity_BFS(
            mesh, &mesh->nodes[2 * p], T_current, cavity, &boundary, in_cavity
        );

        // Après vérification des deux allocations :
        retriangulating(mesh, p, cavity, &boundary, cavity_count);

        
    }
    free(boundary.outside);
    free(boundary.origin);
    free(boundary.edges);
    free(in_cavity);
    free(cavity);
    

    //printf("MESH BEFORE DISCARDING\n");
    //print_triangles(mesh);

    
    discard_ghost(mesh);

    //printf("MESH AFTER DISCARDING\n");
    //print_triangles(mesh);

    /*
    int check_flag = check_empty_circles(mesh);
    if (check_flag){
        //printf("C'est bon :)\n");
    }
    else{
        printf("CE N'EST PAS UNE DELAUNAY 0.0\n");
    }
    */

    // Free
    free(points_order);
    free(hilbert_keys);
}

boundary_st mesh_get_boundary(mesh_st const * mesh) {
    // Do the thing...
    return (boundary_st){
        .nodes=mesh->nodes,
        .edges=NULL,
        .nnodes=mesh->nnodes,
        .nedges=0};
}

uint32_t mesh_delete_elements(mesh_st* mesh, uint8_t const * to_delete){
    // Do the thing...
    return mesh->nelems;
}

uint32_t mesh_adapt(mesh_st* mesh, double** fields, uint32_t const nfields){

    // Helper to access the real data.
    double * nodes = mesh->nodes;
    double * size_field = mesh->size_field;
    uint32_t* elems = mesh->elems;
    double * pfields = (fields != NULL && nfields > 0) ? *fields : NULL;
    uint32_t nnodes = mesh->nnodes;
    uint32_t nelems = mesh->nelems;

    // Do the thing...

    // Do not forget to write the pointer to the double pointer afterward
    // so that the caller can see you've reallocated data
    mesh->nodes = nodes;
    mesh->size_field = size_field;
    mesh->elems = elems;
    mesh->nnodes = nnodes;
    mesh->nelems = nelems;

    if (fields != NULL && nfields > 0) {
        *fields = pfields;
    }
    return 0;
}
