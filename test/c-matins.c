/* ============================= C MeatAxe ==================================
   File:        $Id: c-matins.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     Check MatInsert()
   --------------------------------------------------------------------------
   (C) Copyright 1999 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */


#include "meataxe.h"
#include "check.h"
#include "c-matins.h"

#include <stdlib.h>
#include <stdio.h>




void TestMatInsert1(unsigned flags)

{
    Matrix_t *mat, *a, *e, *z;
    Poly_t *pol;

    mat = MatAlloc(FfOrder,10,10);
    e = MatId(FfOrder,10);
    z = MatAlloc(FfOrder,10,10);

    /* Check p(x) = 0 */
    pol = PolAlloc(FfOrder,-1);
    a = MatInsert(mat,pol);
    if (MatCompare(a,z) != 0)
	Error("p(A) != 0 for p(x)=0");
    MatFree(a);
    a = MatInsert_(MatDup(mat),pol);
    if (MatCompare(a,z) != 0)
	Error("p(A) != 0 for p(x)=0");
    MatFree(a);
    PolFree(pol);

    /* Check p(x) = 1 */
    pol = PolAlloc(FfOrder,0);
    a = MatInsert(mat,pol);
    if (MatCompare(a,e) != 0)
	Error("p(A) != 1 for p(x)=1");
    MatFree(a);
    a = MatInsert_(MatDup(mat),pol);
    if (MatCompare(a,e) != 0)
	Error("p(A) != 0 for p(x)=0");
    MatFree(a);
    PolFree(pol);

    /* Check p(x) = x */
    pol = PolAlloc(FfOrder,1);
    a = MatInsert(mat,pol);
    if (MatCompare(a,mat) != 0)
	Error("p(A) != A for p(x)=x");
    MatFree(a);
    a = MatInsert_(MatDup(mat),pol);
    if (MatCompare(a,mat) != 0)
	Error("p(A) != 0 for p(x)=0");
    MatFree(a);
    PolFree(pol);

    MatFree(mat);
    MatFree(e);
    MatFree(z);
    flags = 0;
}



void TestMatInsert(unsigned flags)

{
    while (NextField() > 0)
    {
	TestMatInsert1(FfOrder);
    }
    flags = 0;
}
