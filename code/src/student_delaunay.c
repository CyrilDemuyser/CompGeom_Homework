// sections 1.2 and 1.4-1.6
#include "student_delaunay.h"
#include <stdlib.h>
#include <stdio.h>
#include "predicates.h"
#include "student_mesh.h"
#include "He_st.h"

void create_initial_triangle(mesh_st* mesh, uint32_t* initial_vertices) {
    // Check if at least 3 points in nodes
    if (mesh->nnodes < 3) {
        // Not enough points to form a triangle
        printf("Error in student_delaunay.c/%s: Not enough points to form a triangle.\n", __FUNCTION__);
        return;
    }

    // Pick three non-collinear points
    int a = 0, b = 1, c = 2;
    double* first_point  = &mesh->nodes[2 * a];
    double* second_point = &mesh->nodes[2 * b];
    double* third_point  = &mesh->nodes[2 * c];

    // Check if the points are collinear
    double orientation = orient2d(first_point, second_point, third_point);
    int valid_triangle_flag = 0;
    if (orientation == 0) {
        // Points are collinear, need to find a non-collinear point
        for (int i = 3; i < mesh->nnodes; ++i) {
            third_point = &mesh->nodes[2 * i];
            orientation = orient2d(first_point, second_point, third_point);
            if (orientation != 0) {
                c = i;
                valid_triangle_flag = 1;
                break;
            }
        }
        if (valid_triangle_flag != 1){
            mesh->nelems = 0;
            return;
        }
    }
    if (orientation < 0) {
        // Ensure counter-clockwise orientation
        int temp = b;
        b = c;
        c = temp;
        second_point = &mesh->nodes[2 * b];
        third_point = &mesh->nodes[2 * c];
    }

    // Create the initial triangle
    mesh->elems[0] = a;
    mesh->elems[1] = b;
    mesh->elems[2] = c;
    mesh->nelems = 1;

    // Create ghost triangle
    // 1
    mesh->elems[3*1 + 0] = b;
    mesh->elems[3*1 + 1] = a;
    mesh->elems[3*1 + 2] = VERTEX_INFINITY; // Ghost vertex
    mesh->nelems += 1;
    // 2
    mesh->elems[3*2 + 0] = c;
    mesh->elems[3*2 + 1] = b;
    mesh->elems[3*2 + 2] = VERTEX_INFINITY;
    mesh->nelems += 1;
    // 3
    mesh->elems[3*3 + 0] = a;
    mesh->elems[3*3 + 1] = c;
    mesh->elems[3*3 + 2] = VERTEX_INFINITY;
    mesh->nelems += 1;

    // Set up half-edges for the initial triangle and ghost triangles
    mesh->twin[0] = 3;
    mesh->twin[1] = 6;
    mesh->twin[2] = 9;
    mesh->twin[3] = 0;
    mesh->twin[4] = 11; //Boundary edge 11
    mesh->twin[5] = 7; //Boundary edge 7
    mesh->twin[6] = 1;
    mesh->twin[7] = 5; //Boundary edge 5
    mesh->twin[8] = 10; //Boundary edge 10
    mesh->twin[9] = 2;
    mesh->twin[10] = 8; //Boundary edge 8
    mesh->twin[11] = 4; //Boundary edge 4

    initial_vertices[0] = a;
    initial_vertices[1] = b;
    initial_vertices[2] = c;
}

// Task 1.4
uint32_t stochastic_visibility_walk(mesh_st* mesh, double* point, uint32_t previous_elem) {
    uint32_t T = previous_elem;
    int failsafe_counter = 0; // To prevent infinite loops
    while (1) {
        if (is_ghost_triangle(mesh, T)) {
            if (orient2d(&mesh->nodes[2 * mesh->elems[3 * T + 0]], 
                         &mesh->nodes[2 * mesh->elems[3 * T + 1]], 
                         point) > 0) {
                uint32_t T0 = T;
                return T0;
            }
            int twin_index = mesh->twin[T * 3];
            T = twin_index / 3;
            continue;
        }
        uint32_t edges[3] = {mesh->elems[3 * T + 0], mesh->elems[3 * T + 1], mesh->elems[3 * T + 2]};
        int edge_order[3] = {0, 1, 2};
        for (int i = 2; i > 0; i--) {
            int j = rand() % (i + 1);
            int temp = edge_order[i];
            edge_order[i] = edge_order[j];
            edge_order[j] = temp;
        }
        int flag = 0;
        for (int i = 0; i < 3; i++) {
            uint32_t s = edges[edge_order[i]];
            uint32_t t = edges[(edge_order[i] + 1) % 3];
            if (orient2d(&mesh->nodes[2 * s], &mesh->nodes[2 * t], point) < 0) {
                int twin_index = mesh->twin[T * 3 + edge_order[i]];
                T = twin_index/3;

                flag = 1;
                break;
            }
        }
        if (!flag) {
            // No edge tested negative: p is inside T
            return T;
        }
        failsafe_counter++;
        if (failsafe_counter > 10000000) {
            printf("Error in student_delaunay/%s: Exceeded maximum iterations.\n", __func__);
            return -1;
        }
    }
}

// Task 1.5
uint32_t grow_cavity_BFS(mesh_st* mesh, double* point, uint32_t T0, uint32_t* cavity, boundary_st* boundary, uint32_t* in_cavity) {
    // BFS = Breadth-First Search
    //printf("Starting BFS cavity growth from triangle %u.\n", T0);


    uint32_t head = 0;  // Prochain triangle à traiter
    uint32_t cavity_count = 0; // Nombre de triangles traités

    cavity[cavity_count++] = T0;
    in_cavity[T0] = 1;

    while (head < cavity_count) {
        uint32_t T = cavity[head++]; // pop front

        for (int i = 0; i < 3; i++){
            uint32_t he = T * 3 + i;
            int twin_index = mesh->twin[he];
            if (twin_index == -1) {
                continue; // No twin, skip
            }

            uint32_t N = (uint32_t)twin_index / 3;
            if (in_cavity[N])
                continue;

            uint32_t *v = &mesh->elems[3 * N];
            double *a = &mesh->nodes[2 * (size_t)v[0]];
            double *b = &mesh->nodes[2 * (size_t)v[1]];

            int belongs;
            if (v[2] == VERTEX_INFINITY) {
                double side = orient2d(a, b, point);
                belongs = side > 0.0;
                if (side == 0.0) {
                    belongs = ((a[0] <= point[0] && point[0] <= b[0]) ||
                            (b[0] <= point[0] && point[0] <= a[0])) &&
                            ((a[1] <= point[1] && point[1] <= b[1]) ||
                            (b[1] <= point[1] && point[1] <= a[1]));
                }
            } else {
                double *c = &mesh->nodes[2 * (size_t)v[2]];
                belongs = incircle(a, b, c, point) > 0.0;
            }

            if (belongs) {
                in_cavity[N] = 1;
                cavity[cavity_count++] = N;
            }
        }
    }

    //printf("Cavity grown with %u triangles.\n", cavity_count);

    // Extract the boundary edges
    // boundary <- all half-edges in cavity whose twin is outside cavity
    uint32_t found_boundary = 0;
    uint32_t last_added_index = 0;
    for (uint32_t i = 0; i < cavity_count; i++) {
        uint32_t T = cavity[i];
        for (int j = 0; j < 3; j++) {
            uint32_t he = T * 3 + j;
            int twin_index = mesh->twin[he];
            if (twin_index == -1) {
                last_added_index = he;
                found_boundary = 1;
                continue;
            }
            uint32_t N = (uint32_t)(twin_index / 3);
            if (in_cavity[N] != 1) {
                last_added_index = he;
                found_boundary = 1;
            }
        }
        if (found_boundary) {
            break; // Stop after finding the first boundary edge
        }
    }
    //printf("Boundary extracted with %u edges.\n", last_added_index + 1);
    // order boundary by following its connected edge loop
    uint32_t current = last_added_index;
    uint32_t boundary_count = 0;

    do {
        boundary->origin[boundary_count] = mesh->elems[current];
        boundary->outside[boundary_count] = mesh->twin[current];
        boundary->edges[boundary_count++] = current;
        

        int next = get_he_next((int)current);
        while (mesh->twin[next] != -1 &&
            in_cavity[mesh->twin[next] / 3]) {
            next = get_he_next(mesh->twin[next]);
        }

        current = (uint32_t)next;
    } while (current != last_added_index);

    boundary->nedges = boundary_count;

    for (uint32_t j = 0; j < cavity_count; ++j) {
        in_cavity[cavity[j]] = 0;
    }

    return cavity_count;
}


int retriangulating(mesh_st* mesh, uint32_t p, uint32_t* cavity, boundary_st* boundary, uint32_t cavity_count) {
    uint32_t old_nelems = mesh->nelems;
    uint32_t new_nelems = old_nelems + boundary->nedges - cavity_count;
    int first_pa = -1;
    int prev_bp = -1;

    for (uint32_t i = 0; i < boundary->nedges; i++){
        uint32_t T;
        if (i < cavity_count){
            T = cavity[i];
        } else {
            T = old_nelems + (i - cavity_count);
        }
        uint32_t a = boundary->origin[i];
        uint32_t b = boundary->origin[(i + 1) % boundary->nedges];

        int base = (int)(3 * T);
        int he_ab = base;
        if (a == VERTEX_INFINITY)      he_ab = base + 2;  // (b, p, a)
        else if (b == VERTEX_INFINITY) he_ab = base + 1;  // (p, a, b)
        int he_bp = get_he_next(he_ab);
        int he_pa = get_he_next(he_bp);

        mesh->elems[he_ab] = a;
        mesh->elems[he_bp] = b;
        mesh->elems[he_pa] = p;
        
        int out = boundary->outside[i];
        mesh->twin[he_ab] = out;
        if (out != -1) mesh->twin[out] = he_ab;

        if (i == 0) first_pa = he_pa;
        else { mesh->twin[prev_bp] = he_pa; mesh->twin[he_pa] = prev_bp; }
        prev_bp = he_bp;

        if (a != VERTEX_INFINITY) mesh->startHE[a] = (uint32_t)he_ab;
        mesh->startHE[p] = (uint32_t)he_pa;
    }

    mesh->twin[prev_bp]  = first_pa;
    mesh->twin[first_pa] = prev_bp;
    mesh->nelems = new_nelems;
    return 0;
}


int discard_ghost(mesh_st *mesh){
    for (int64_t t = (int64_t)mesh->nelems - 1; t >= 0; --t) {
        //printf("Checking triangle %lld for ghost status.\n", t);
        if (is_ghost_triangle(mesh, (uint32_t)t)) {
            if (remove_triangle(mesh, (uint32_t)t) == -1) {
                printf("Error in student_delaunay.c/%s.\n", __func__);
                return -1;
            }
        }
    }
    return 0;
}