// Hilbert section 1.3.
#include "student_hilbert.h"
#include <math.h>
#include "HXTSort.h"

// We are using HXTSort -> docs on its repo page : https://git.immc.ucl.ac.be/hextreme/HXTSort/-/blob/master/doc/doc.pdf?ref_type=heads
// It's all in the header file
// HXTSORT64 DOC:
/*
    HXTSORT64( TYPE , TYPE* array , uint64_t n , uint64_t max ,
    uint64_t (*getKey)(TYPE*, const void*) ,
    void* user_data )

    Sort an array of elements each associated with a 32 bits key.

    Return :  (void) This is a MACRO

    Parameter :
    [in] type of the elements in array
    [in out] array the array to sort
    [in] n number of element in array
    [in] max upper bound on the return value of
    getKey
    [in] (*getKey)(TYPE*, const void*) function that return the unsigned
    key associated to an element
    [in] user_data the void pointer given to get_key
*/


// Function that return the unsigned key associated to an element
// On a deux tableaux : les clefs Hilbert et juste les points
// Au début on a Hilber[0] = clef du point 0 -> points[0]
// Mais comme on va trier les points, on va perdre l'association avec les clefs
// Donc on va lire l'index du point et le passer dans Hilbert
static uint64_t get_point_key(uint32_t* point_id_ptr, const void* user_data){
    // Retrouver le tableau de clefs transmis à HXTSort.
    const uint64_t* hilbert_keys = user_data;
    // Lire le numéro du point dans l'élément à trier.
    uint32_t point_id = *point_id_ptr;
    // Lire la clef associée à ce point.
    uint64_t point_key = hilbert_keys[point_id];
    return point_key;
}

void sort_points_by_hilbert(uint32_t* points_order, uint32_t npoints, const uint64_t* hilbert_keys){
    if (npoints == 0) {
        return 0; // No points to sort
    }

    uint64_t max_key = 0;
    for (uint32_t i = 0; i < npoints; i++){
        points_order[i] = i;
        if (hilbert_keys[i] > max_key) {
            max_key = hilbert_keys[i];
        }
    }

    HXTSORT64(uint32_t, points_order, npoints, max_key, get_point_key, hilbert_keys);

}

void compute_hilbert_keys(double* points, uint32_t npoints, uint64_t* hilbert_keys, uint32_t depth){
    if (npoints == 0) {
        return;
    }

    double x_min = points[0]; double x_max = points[0];
    double y_min = points[1]; double y_max = points[1];
    for (uint32_t i = 0; i < npoints; i++){
        double x = points[2*i];
        double y = points[2*i + 1];
        if (x < x_min) x_min = x;
        if (x > x_max) x_max = x;
        if (y < y_min) y_min = y;
        if (y > y_max) y_max = y;
    }
    double x0 = (x_min + x_max) / 2.0;
    double y0 = (y_min + y_max) / 2.0;
    double half_size = fmax(x_max - x_min, y_max - y_min) / 2.0;

    for (uint32_t i = 0; i < npoints; i++){
        double x = points[2*i];
        double y = points[2*i + 1];

        int bits[depth];

        hilbertCoordinates(x, y, x0, y0, half_size, 0.0, 0.0, half_size, depth, bits);

        uint64_t key = 0;
        for (int j = 0; j < depth; ++j) {
            key = 4 * key + (uint64_t)bits[j];
        }
        hilbert_keys[i] = key;
    }
}

static void swap(double *a, double *b) {
  double temp = *a;
  *a = *b;
  *b = temp;
}

void hilbertCoordinates ( double x, double y,
			  double x0, double y0,
			  double xBlue, double yBlue,
			  double xRed , double yRed,
			  int Depth, int bits [] )
{
  for (int i=0;i<Depth;i++){
    double coordRed  = (x-x0)*xRed +  (y-y0)*yRed;
    double coordBlue = (x-x0)*xBlue + (y-y0)*yBlue;
    xRed /=2;  yRed /=2;  xBlue /=2;  yBlue /=2; 
    if (coordBlue >= 0 && coordRed >= 0){
      bits[i] = 2;
      x0 += (xRed + xBlue); y0 += (yRed + yBlue);      
    } 
    else if (coordBlue <= 0 && coordRed >= 0){
      bits[i] = 3;
      x0 -= (xBlue-xRed); y0 -= (yBlue-yRed);      
      swap (&xRed, &xBlue); swap (&yRed, &yBlue);
      xRed = -xRed; xBlue = -xBlue;
      yRed = -yRed; yBlue = -yBlue;
    } 
    else if (coordBlue <= 0 && coordRed <= 0){
      bits[i] = 0;
      x0 -= (xRed + xBlue); y0 -= (yRed + yBlue);      
      swap (&xRed, &xBlue); swap (&yRed, &yBlue);
    } 
    else if (coordBlue >= 0 && coordRed <= 0){
      bits[i] = 1;
      x0 += (xBlue - xRed); y0 += (yBlue - yRed);      
    }
  }
}