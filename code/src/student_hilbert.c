// Hilbert section 1.3.
#include "student_hilbert.h"

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
// Mais comme on va trier les clefs, on va perdre l'association avec les points
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

static uint64_t sort_points_by_hilbert(){
    // TODO
}