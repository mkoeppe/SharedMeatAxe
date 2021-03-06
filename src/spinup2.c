/* ============================= C MeatAxe ==================================
   File:        $Id: spinup2.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     Spin-up with script.
   --------------------------------------------------------------------------
   (C) Copyright 1999 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */

#include "meataxe.h"

MTX_DEFINE_FILE_INFO

/**
 ** @addtogroup spinup
 ** @{
 **/


static int ArgsAreValid(const Matrix_t *seed, const MatRep_t *rep, 
    const IntMatrix_t *script)

{
    if (!ImatIsValid(script) || script->Noc != 2)
    {
	MTX_ERROR("Invalid script");
	return 0;
    }
    if (!MatIsValid(seed))
    {
	MTX_ERROR("Invalid seed space");
	return 0;
    }
    if (!MrIsValid(rep) || rep->NGen <= 0)
    {
	MTX_ERROR("Invalid representation");
	return 0;
    }
    if (seed->Noc != rep->Gen[0]->Noc || seed->Field != rep->Gen[0]->Field)
    {
	MTX_ERROR1("seed and rep: %E",MTX_ERR_INCOMPAT);
	return 0;
    }
    return 1;
}




/**
 ** Spin-up with script.
 ** This function repeats a previous spin-up with different seed vector and generators.
 ** Todo so, the functino needs a spin-up script, which is generated by SpinUp().
 ** The result is a matrix having as many rows as the script.
 ** @param seed Matrix with seed vectors.
 ** @param rep Pointer to a MatRep_t structure with generators.
 ** @param script Pointer to the spinup script.
 ** @return Span of the seed vector(s) under the action of the generators, or 0 on error.
 **/

Matrix_t *SpinUpWithScript(const Matrix_t *seed, const MatRep_t *rep, 
    const IntMatrix_t *script)
{
    const long *op;
    Matrix_t *basis;
    int i;

    /* Check arguments.
       ---------------- */
    if (!ArgsAreValid(seed,rep,script))
    {
	MTX_ERROR("Invalid arguments");
	return NULL;
    }

    /* Spin up
       ------- */
    FfSetField(seed->Field);
    FfSetNoc(seed->Noc);
    op = script->Data;
    basis = MatAlloc(FfOrder,script->Nor,seed->Noc);
    for (i = 0; i < script->Nor; ++i)
    {
    	int vecno = (int) op[2 * i];
	int vecgen = (int) op[2 * i + 1];
	PTR vec = MatGetPtr(basis,i);
	if (vecgen < 0)
	{
	    if (vecno < 1 || vecno > seed->Nor)
	    {
	    	MTX_ERROR2("Seed vector number (%d) out of range (%d)",
		    vecno,seed->Nor);
	    }
	    else
	    	FfCopyRow(vec,MatGetPtr(seed,vecno - 1));
	}
	else
	{
	    if (vecno < 0 || vecno >= i)
	    	MTX_ERROR2("Invalid source vector %d at position %d",vecno,i);
	    else if (vecgen < 0 || vecgen >= rep->NGen)
	    {
	    	MTX_ERROR2("Invalid generator number %d at position %d",
		    vecgen,i);
	    }
	    FfMapRow(MatGetPtr(basis,vecno),rep->Gen[vecgen]->Data,FfNoc,vec);
	}
    }
    return basis;
}
    



/* Convert a spin-up script from 2.3 format.

   The MeatAxe version 2.3 uses 1-based indexes for both generators and vectors, seed 
   vectors have gen=0 and vec is the 1-based seed vector number.
   Starting with version 2.4, generator and vector indexes are 0-based,
   seed vectors have gen = -1 and vec is the 1-based(!) seed vector number.
*/

int ConvertSpinUpScript(IntMatrix_t *script)
{
    int k;
    long *op = script->Data;

    if (script->Nor == 0 || op[1] < 0)
	return 0;

    for (k = 0; k < script->Nor; ++k)
    {
	if (op[2*k+1] == 0)
	{
	    /* Seed vector: keep vector number, set gen=-1 */
	    op[2*k+1] = -1;
	}
	else
	{
	    /* Image vector: decrement vec and gen */
	    --op[2*k];
	    --op[2*k+1];
	}
    }
    return 1;

}

/**
 ** @}
 **/

