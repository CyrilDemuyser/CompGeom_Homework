// sections 1.2 and 1.4-1.6
#include "student_delaunay.h"


void create_initial_triangle(mesh_st* mesh) {
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
    if (orientation == 0) {
        // Points are collinear, need to find a non-collinear point
        for (int i = 3; i < mesh->nnodes; ++i) {
            third_point = &mesh->nodes[2 * i];
            orientation = orient2d(first_point, second_point, third_point);
            if (orientation != 0) {
                c = i;
                break;
            }
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
    // 2
    mesh->elems[3*2 + 0] = c;
    mesh->elems[3*2 + 1] = b;
    mesh->elems[3*2 + 2] = VERTEX_INFINITY;
    // 3
    mesh->elems[3*3 + 0] = a;
    mesh->elems[3*3 + 1] = c;
    mesh->elems[3*3 + 2] = VERTEX_INFINITY;
    mesh->nelems = 4;

    // Set up half-edges for the initial triangle and ghost triangles
    mesh->twin[0] = 3;
    mesh->twin[1] = 6;
    mesh->twin[2] = 9;
    mesh->twin[3] = 0;
    mesh->twin[4] = 11;
    mesh->twin[5] = 7;
    mesh->twin[6] = 1;
    mesh->twin[7] = 5;
    mesh->twin[8] = 10;
    mesh->twin[9] = 2;
    mesh->twin[10] = 8;
    mesh->twin[11] = 4;

}

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
        int random_index = rand() % 3;
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
        if (failsafe_counter > 1000) {
            printf("Error in student_delaunay/%s: Exceeded maximum iterations.\n", __func__);
            return -1;
        }
    }
}

void grow_cavity_BFS(mesh_st* mesh, double* point, uint32_t T0, uint8_t* cavity, uint32_t* boundary_edges) {
    uint8_t* in_cavity = calloc(mesh->nelems, sizeof(*in_cavity));


    uint32_t head = 0;  // Prochain triangle à traiter
    uint32_t count = 0; // Nombre de triangles traités

    cavity[count++] = T0;
    in_cavity[T0] = 1;

    while (head < count) {
        uint32_t T = cavity[head++]; // pop front

        for (int i = 0; i < 3; i++){
            uint32_t he = T * 3 + i;
            int twin_index = mesh->twin[he];
            if (twin_index == -1) {
                continue; // No twin, skip
            }

            uint32_t N = (uint32_t)(twin_index / 3);
            double N_points[3];
            N_points[0] = mesh->nodes[2 * mesh->elems[3 * N + 0]];
            N_points[1] = mesh->nodes[2 * mesh->elems[3 * N + 1]];
            N_points[2] = mesh->nodes[2 * mesh->elems[3 * N + 2]];
            if (in_cavity[N] != 1 && incircle(N_points[0], N_points[1], N_points[2], point) > 0){
                in_cavity[N] = 1;
                cavity[count++] = N;
            }
        }
    }

    // Extract the boundary edges
    // boundary <- all half-edges in cavity whose twin is outside cavity
    uint32_t* unordered_boundary_edges = (uint32_t*)malloc(sizeof(uint32_t) * count * 3);
    uint32_t last_added_index = 0;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t T = cavity[i];
        for (int j = 0; j < 3; j++) {
            uint32_t he = T * 3 + j;
            int twin_index = mesh->twin[he];
            if (twin_index == -1) {
                unordered_boundary_edges[he] = 1; // Mark as boundary edge
                last_added_index = he;
                continue;
            }
            uint32_t N = (uint32_t)(twin_index / 3);
            if (in_cavity[N] != 1) {
                unordered_boundary_edges[he] = 1; // Mark as boundary edge
                last_added_index = he;
            }
        }
    }
    // order boundary by following its connected edge loop
    uint32_t current = last_added_index;
    int boundary_count = 0;
    while (1){
        boundary_edges[boundary_count] = current; // Mark as part of the ordered boundary
        boundary_count++;
        unordered_boundary_edges[current] = 0; // Mark as processed
        uint32_t next_he = get_he_next(current);
        // TODO
        }
    
}