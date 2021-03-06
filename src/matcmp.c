/* ============================= C MeatAxe ==================================
   File:        $Id: matcmp.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     Compare matrices.
   --------------------------------------------------------------------------
   (C) Copyright 1998 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */


#include "meataxe.h"
#include <stdlib.h>

MTX_DEFINE_FILE_INFO



/**
 ** @addtogroup mat
 ** @{
 **/

/**
 ** Compare two matrices
 ** If the matrices are equal, the return value is 0. Otherwise the return value is positive,
 ** if @em a is "greater" than @em b and negative, if @em a is "less" than @em b. The ordering
 ** matrices is defined as follows:
 **
 ** - If the matrices are over different fields, the matrix over the smaller field is smaller.
 ** - Otherwise, if the matrices have different number of columns, the matrix with the smaller
 **   number of columns is smaller.
 ** - Otherwise, if the matrices have different number of rows, the matrix with the smaller
 **   number of rows is smaller.
 ** - Otherwise, the relation is determined by the return value of FfCmpRow() on the first row
 **   that is not equal in both matrices.
 **
 ** In case an error occurs, the return value is -1. But note that a return value of -1 does
 ** not necessarily mean that an error has occurred.
 ** @param a First matrix.
 ** @param b Second matrix.
 ** @return 0 if the matrices are equal, nonzero otherwise (see description), -2 on error.
 **/

int MatCompare(const Matrix_t *a, const Matrix_t *b)
{
    int i;

    /* Check the arguments
       ------------------ */
    if (!MatIsValid(a) || !MatIsValid(b))
    {
	MTX_ERROR1("%E",MTX_ERR_BADARG);
	return -2;
    }

    /* Compare fields and dimensions
       ----------------------------- */
    if ((i = a->Field - b->Field) != 0)
	return i;
    if ((i = a->Noc - b->Noc) != 0)
	return i;
    if ((i = a->Nor - b->Nor) != 0)
	return i;

    /* Compare the entries row by row. We do not use memcmp on the
       whole matrix because we must ignore padding bytes.
       ----------------------------------------------------------- */
    FfSetField(a->Field);  /* No error checking */
    FfSetNoc(a->Noc);
    for (i = 0; i < a->Nor; ++i)
    {
	PTR pa = MatGetPtr(a,i);
	PTR pb = MatGetPtr(b,i);
	int diff = FfCmpRows(pa,pb);
	if (diff != 0)
	    return diff;
    }

    /* The matrices are equal!
       ----------------------- */
    return 0;
}

/**
 ** @}
 **/
