/* ============================= C MeatAxe ==================================
   File:        $Id: zad.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     Sum of matrices.
   --------------------------------------------------------------------------
   (C) Copyright 1999 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */


#include "meataxe.h"
#include <string.h>


/* ------------------------------------------------------------------
   Variables
   ------------------------------------------------------------------ */

static MtxApplicationInfo_t AppInfo = {
"zad", "Add or Subtract Matrices",
"SYNTAX\n"
"    zad " MTX_COMMON_OPTIONS_SYNTAX " [-]<Mat> [-]<Mat> ... <Result>\n"
"\n"
"ARGUMENTS\n"
"    <Mat> ................... Input file: Matrix to add (-<Mat> subtracts)\n"
"    <Result> ................ Output file: Sum\n"
"\n"
"OPTIONS\n"
MTX_COMMON_OPTIONS_DESCRIPTION
"\n"
"FILES\n"
"    <Mat> ................... I Input matrix\n"
"    <Result> ................ O Sum of the input matrices\n"
};

static MtxApplication_t *App = NULL;

MTX_DEFINE_FILE_INFO

#define MAX_INPUT 20
int NInput;			    /* Number of input matrices */
FILE *Input[MAX_INPUT] = {0};	    /* Input matrices */
int Subtract[MAX_INPUT];
int Field = -1, Nor = -1, Noc = -1;
PTR Buf1, Buf2;			    /* Working buffer */
FILE *Output = NULL;		    /* Output file */


static int CheckHeader(const char *fn, int fl, int nor, int noc)

{
    static const char *fn0 = NULL;

    if (fl < 2)
    {
	MTX_ERROR2("%s: %E",fn,MTX_ERR_NOTMATRIX);
	return -1;
    }
    if (Nor == -1)
    {
	Nor = nor;
	Noc = noc;
	Field = fl;
	fn0 = fn;
    }
    else if (fl != Field || nor != Nor || noc != Noc)
    {
	MTX_ERROR3("%s and %s: %E",fn0,fn,MTX_ERR_INCOMPAT);
	return -1;
    }
    return 0;
}



/* ------------------------------------------------------------------
   Init()
   ------------------------------------------------------------------ */

static int Init(int argc, const char **argv)

{
    int i;

    if ((App = AppAlloc(&AppInfo,argc,argv)) == NULL)
	return -1;

    /* Parse command line.
       ------------------- */
    if (AppGetArguments(App,3,MAX_INPUT + 1) < 0)
	return -1;
    NInput = App->ArgC - 1;

    /* Open the input files
       -------------------- */
    for (i = 0; i < NInput; ++i)
    {
	int nor2, noc2, fl2;
	const char *file_name = App->ArgV[i];
	Subtract[i] = *file_name == '-';
	if (*file_name == '-' || *file_name == '+')
	    ++file_name;
	Input[i] = FfReadHeader(file_name,&fl2,&nor2,&noc2);
	if (Input[i] == NULL)
	    return -1;
	if (CheckHeader(file_name,fl2,nor2,noc2) != 0)
	    return -1;
    }

    /* Open the output file.
       --------------------- */
    Output = FfWriteHeader(App->ArgV[App->ArgC - 1],Field,Nor,Noc);
    if (Output == NULL)
	return -1;

    /* Allocate workspace
       ------------------ */
    FfSetField(Field);
    FfSetNoc(Noc);
    Buf1 = FfAlloc(1);
    Buf2 = FfAlloc(1);
    if (Buf1 == NULL || Buf2 == NULL)
	return -1;

    return 0;
}




/* ------------------------------------------------------------------
   CleanUp()
   ------------------------------------------------------------------ */

static void CleanUp()

{
    int i;

    /* Close all files.
       ---------------- */
    for (i = 0; i < NInput; ++i)
    {
	if (Input[i] != NULL)
	    fclose(Input[i]);
    }
    if (Output != NULL)
	fclose(Output);

    /* Free workspace.
       --------------- */
    if (Buf1 != NULL) SysFree(Buf1);
    if (Buf2 != NULL) SysFree(Buf2);
    if (App != NULL) AppFree(App);
}



static int AddMatrices()

{
    int i;
    FEL MinusOne = FfNeg(FF_ONE);

    /* Process the matrices row by row.
       -------------------------------- */
    for (i = Nor; i > 0; --i)
    {
	int k;

	/* Read a row from the first matrix.
	   --------------------------------- */
	FfReadRows(Input[0],Buf1,1);
	if (Subtract[0])
	    FfMulRow(Buf1,MinusOne);

	/* Add or subtract rows from the other matrices.
	   --------------------------------------------- */
	for (k = 1; k < NInput; ++k)
	{
	    FfReadRows(Input[k],Buf2,1);
	    if (Subtract[k])
		FfAddMulRow(Buf1,Buf2,MinusOne);
	    else
		FfAddRow(Buf1,Buf2);
	}

	/* Write the result to the output file.
	   ------------------------------------ */
	FfWriteRows(Output,Buf1,1);
    }
    return 0;
}



int main(int argc, const char **argv)

{
    int rc;
    if (Init(argc, argv) != 0)
    {
	MTX_ERROR("Initialization failed");
	return 1;
    }
    rc = AddMatrices();
    CleanUp();
    return rc;
}




/**
@page prog_zad zad - Add Matrices

@section zad-syntax Command Line
<pre>
zad [@em Options] [-]@em Mat [-]@em Mat ... @em Result
</pre>

@par @em Options
  Standard options, see @ref prog_stdopts
@par @em Mat
  Input matrix
@par @em Result
  Result matrix

@section zad-inp Input Files
@par @em Mat
  Input matrix

@section zad-out Output Files
@par @em Result
  Result matrix

@section zad-desc Description

This program reads two or more input matrices, calculates their sum or difference and
writes the result to a file.  The input matrices must be compatible, i.e., they must be
over the same field and have the same dimensions. @b zad is designed to work with very
large matrices without running out of memory. Only two rows are allocated as working
memory.

By default, all input matrices are added. For example,
<pre>
zad A B C
</pre>
calculates the sum of A and B, and writes the result to C.
If a file name is preceeded by a minus sign, this matrix is subtracted.
For example,
<pre>
zad A -B -C D
</pre>
calculates $D=A-B-C$. To subtract the first matrix, you must insert an
extra "--" before the file names. Otherwise the first argument
would be interpreted as a program option. For example, to calculate $C=-A-B$, use
<pre>
zad -- -A -B C
</pre>
If a file name starts with "-", preceed the file name by "+" to add, or
by "-" to subtract the matrix. If, for example, the second input file
is "-B", use the following syntax:
<pre>
zad A +-B C
zad A --B C
</pre>

*/
