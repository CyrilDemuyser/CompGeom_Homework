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

    

}