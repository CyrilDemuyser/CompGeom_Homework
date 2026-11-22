// section 1.7
#include <stdlib.h>
#include "student_alpha.h"

// Task 4
static double distance(double const *A, double const *B){
    // Helper function to compute the Euclidean distance between two points A and B in 2D space.
    double dx = B[0] - A[0];
    double dy = B[1] - A[1];

    return sqrt(dx * dx + dy * dy);
}

double triangle_circumradius(mesh_st const *mesh, uint32_t t){
    uint32_t *v = &mesh->elems[3 * t];

    double const *A = &mesh->nodes[2 * v[0]];
    double const *B = &mesh->nodes[2 * v[1]];
    double const *C = &mesh->nodes[2 * v[2]];

    double a = distance(B, C);
    double b = distance(C, A);
    double c = distance(A, B);

    double twice_area = fabs(orient2d(A, B, C));

    if (twice_area == 0.0) {  // a degenerate triangle has zero area
        return INFINITY;
    }

    return (a * b * c) / (2.0 * twice_area);
}

void alpha_filter(mesh_st *mesh, double alpha){
    // Filters the triangles of a Delaunay triangulation based on the alpha shape criterion.
    uint8_t *to_delete = malloc(sizeof(uint8_t) * mesh->nelems);

    for (uint32_t t = 0; t < mesh->nelems; ++t) {
        double R = triangle_circumradius(mesh, t);

        if (R > alpha) {
            to_delete[t] = 1; // delete this triangle
        }
        else {
            to_delete[t] = 0; // keep this triangle
        }
    }

    mesh_delete_elements(mesh, to_delete);
    free(to_delete); // no longer needed after deletion
}

// Task 5
double triangle_local_alpha(
    mesh_st const *mesh,
    uint32_t t,
    alpha_method_t method)
{
    /*
    Computes the local alpha value for a triangle 
    based on the specified method (average, minimum, or maximum) 
    of the size field values at its nodes.
    */ 
    uint32_t const *v = &mesh->elems[3 * t]; // vertex indices of triangle t

    double alpha_i = mesh->size_field[v[0]];
    double alpha_j = mesh->size_field[v[1]];
    double alpha_k = mesh->size_field[v[2]];

    if (method == ALPHA_AVERAGE) {
        return (alpha_i + alpha_j + alpha_k) / 3.0;
    }
    
    if (method == ALPHA_MINIMUM) {
        double minimum = alpha_i;
        if (alpha_j < minimum) {
            minimum = alpha_j;
        }
        if (alpha_k < minimum) {
            minimum = alpha_k;
        }

        return minimum;
    }

    if (method == ALPHA_MAXIMUM) {
        double maximum = alpha_i;
        if (alpha_j > maximum) {
            maximum = alpha_j;
        }
        if (alpha_k > maximum) {
            maximum = alpha_k;
        }

        return maximum;
    }

    return -1.0; // if invalid method (should not happen)
}

int alpha_filter_local(mesh_st *mesh, alpha_method_t method){
        if (method != ALPHA_AVERAGE &&
        method != ALPHA_MINIMUM &&
        method != ALPHA_MAXIMUM) {
        printf("Error: invalid alpha method.\n"); // check that the selected method is valid
        return -1;
    }

    uint8_t *to_delete = malloc(sizeof(uint8_t) * mesh->nelems);

    for (uint32_t t = 0; t < mesh->nelems; ++t) { // examine every triangle

        double R = triangle_circumradius(mesh, t);
        double alpha_T = triangle_local_alpha(mesh, t, method);
        if (R > alpha_T) {
            to_delete[t] = 1;
        }
        else {
            to_delete[t] = 0;
        }
    }

    mesh_delete_elements(mesh, to_delete);
    free(to_delete);

    return 0;
}
