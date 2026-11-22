
#ifndef STUDENT_ALPHA_H
#define STUDENT_ALPHA_H

#include <math.h>
#include "predicates.h"
#include "student_mesh.h"
#include "student_triangulation.h"

/* UNDERSTANDING OF THE ALPHA SHAPE:
The alpha shape is a generalization of the convex hull of a set of points. 
It captures the shape of the point set more accurately by allowing for concavities and holes.

→ Conceptually:
    1. We have a working Delaunay triangulation of the point set.
    2. Select an alpha parameter (a positive real number).
    3. For each triangle in the Delaunay triangulation:
    - Compute the circumradius R of the triangle.
    - If R <= alpha, keep the triangle; otherwise, discard it.
    4. The remaining triangles form the alpha shape.

CALCULATING THE CIRCUMRADIUS:
Given a triangle with vertices A, B, C, the circumradius R can be computed using the formula:
    R = (abc) / (4 * Area)
    Where:
    - a, b, c are the lengths of the sides opposite to vertices A, B, C respectively.
    - Area is the area of the triangle, which can be computed using the determinant method
    Area = 0.5 * |Ax(By - Cy) + Bx(Cy -Ay) + Cx(Ay - By)| 
→ no need to compute the circumcenter coordinates!
*/

double triangle_circumradius(
    // Computes the circumradius of a triangle given its triangle number.
    mesh_st const *mesh,
    uint32_t triangle
);

void alpha_filter(
    // Filters the triangles of a Delaunay triangulation based on the alpha shape criterion.
    mesh_st *mesh,
    double alpha
);

#endif /* STUDENT_ALPHA_H */
