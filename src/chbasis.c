/* ============================= C MeatAxe ==================================
   File:        $Id: chbasis.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     Change basis.
   --------------------------------------------------------------------------
   (C) Copyright 1997 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */


#include "meataxe.h"
#include <stdlib.h>
#include <string.h>

MTX_DEFINE_FILE_INFO


/**
!section obj.matrep
 ** Change basis.
 ** @param rep
    Matrix representation.
 ** @param trans   
    Basis transformation matrix.
 ** @return
    $0$ on success, $-1$ on error.
!description
    This function performs a change of basis on a matrix representation.
    |trans| is the transformation matrix $T$. The rows of |trans| must 
    contain the new basis vectors, expressed in the old basis. Then, the 
    transformed generators are given by
    \[
	g_i' = T g_i T^{-1}
    \]
 ** @see 
 **/

int MrChangeBasis(MatRep_t *rep, const Matrix_t *trans)

{
    /* Check arguments
       --------------- */
    if (!MrIsValid(rep))
    {
	MTX_ERROR1("rep: %E",MTX_ERR_BADARG);
	return -1;
    }
    if (rep->NGen <= 0)
	return 0;
    if (trans->Field != rep->Gen[0]->Field || 
	trans->Nor != rep->Gen[0]->Nor ||
	trans->Noc != rep->Gen[0]->Noc)
    {
	MTX_ERROR1("%E",MTX_ERR_INCOMPAT);
	return -1;
    }
    return ChangeBasis(trans, rep->NGen, (const Matrix_t **)(rep->Gen), rep->Gen);
}


/** Conjugate a list @em gen of @em ngen square matrices over the same
 *  field and of the same dimensions by a mattrix @em trans
 *  and write the result into @em newgen. If @em gen == @em newgen, then
 *  the previous content of @em newgen will be overridden.
 *  Return -1 on error and 0 on success. **/
int ChangeBasis(const Matrix_t *trans, int ngen, const Matrix_t *gen[],
	Matrix_t *newgen[])

{
    Matrix_t *bi;
    int i;

    MTX_VERIFY(ngen >= 0);
    if (!MatIsValid(trans))
    {
	MTX_ERROR1("trans: %E",MTX_ERR_BADARG);
	return -1;
    }

    if ((bi = MatInverse(trans)) == NULL)
    {
	MTX_ERROR("Basis transformation is singular");
	return -1;
    }

    Matrix_t *tmp = MatAlloc(trans->Field, trans->Nor, trans->Noc);
    if (!tmp) return -1;
    size_t tmpsize = FfCurrentRowSize*trans->Nor;
    for (i = 0; i < ngen; ++i)
    {
        MTX_VERIFY(gen[i]->Nor==trans->Nor);
        MTX_VERIFY(gen[i]->Noc==trans->Noc);
        memset(tmp->Data, FF_ZERO, tmpsize);
        if (!MatMulStrassen(tmp, trans, gen[i]))
        {
			MatFree(tmp);
			return -1;
		}
        if ((const Matrix_t **)newgen == gen)
            memset(newgen[i]->Data, FF_ZERO, tmpsize);
        else
        {
            newgen[i] = MatAlloc(trans->Field, trans->Nor, trans->Noc);
            if (!newgen[i])
            {
                MatFree(tmp);
                MatFree(bi);
                return -1;
            }
        }
        if (!MatMulStrassen(newgen[i], tmp, bi))
        {
            MatFree(tmp);
            MatFree(bi);
            return -1;
        }
    }
    MatFree(bi);
    MatFree(tmp);
    return 0;
}


