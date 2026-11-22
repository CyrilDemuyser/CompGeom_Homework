// section 1.7
#include "student_alpha.h"

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
