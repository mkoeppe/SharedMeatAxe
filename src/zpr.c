/* ============================= C MeatAxe ==================================
   File:        $Id: zpr.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     Print a matrix or permutaion.
   --------------------------------------------------------------------------
   (C) Copyright 1997 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */

#include "meataxe.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


/* ------------------------------------------------------------------
   Global data
   ------------------------------------------------------------------ */

MTX_DEFINE_FILE_INFO

static long HdrPos;
static long hdr[3];
static FILE *dest = NULL;   /* Output file */
static const char *inpname = NULL;
static FILE *inpfile = NULL;
static int Gap = 0;		/* -g (GAP mode) */
static int Summary = 0;		/* -s (summary) */



static MtxApplicationInfo_t AppInfo = {
"zpr", "Print Permutations Or Matrices",
"\n"
"SYNTAX\n"
"    zpr [-G] [-s] <Binfile> [<Textfile>]\n"
"\n"
"OPTIONS\n"
"    -G   GAP output\n"
"    -s   Print summary only\n\n"
"\n"
"FILES\n"
"    <Binfile>   i  A matrix or permutation in binary format\n"
"    <Textfile>  i  The output in text format (default: stdout)\n"
};

static MtxApplication_t *App = NULL;




/* ------------------------------------------------------------------
   PrString(), PrLong() - Prettty printer
   ------------------------------------------------------------------ */

static void PrString(char *c)
{
    static int pos = 0;
    int l = strlen(c);

    if (l + pos >= 78)
    {
	fprintf(dest,"\n");
	pos = 0;
    }
    fprintf(dest,"%s",c);
    for (; *c != 0; ++c)
	if (*c == '\n') pos = 0; else ++pos;
}


static void PrLong(long l)

{
    char tmp[20];
    sprintf(tmp,"%ld",l);
    PrString(tmp);
}


/* ------------------------------------------------------------------
   prmatrix() - Print a matrix in standard format.
   ------------------------------------------------------------------ */

static void prmatrix()

{
    PTR m1;
    FEL f1;
    long loop1, j1;
    int md, mx, iv;

    FfSetField(hdr[0]);
    FfSetNoc(hdr[2]);
    m1 = FfAlloc(1);

    if (hdr[0] < 10) {md = 1; mx = 80;}
    else if (hdr[0] < 100) {md = 3; mx = 25;}
    else if (hdr[0] < 1000) {md = 4; mx = 20;}
    else if (hdr[0] < 10000) {md = 5; mx = 15;}
    else {md = 6; mx = 12;}

    fprintf(dest,"matrix field=%ld rows=%ld cols=%ld\n",hdr[0],hdr[1],hdr[2]);
    for (loop1 = 1; loop1 <= hdr[1]; ++loop1)
    {
	if (FfReadRows(inpfile,m1,1) != 1)
	{
	    MTX_ERROR1("Cannot read %s",inpname);
	    return;
	}
	iv = 1;
	for (j1 = 0; j1 < hdr[2]; ++j1)
	{
	    f1 = FfExtract(m1,j1);
	    switch (md)
	    {	case 1: fprintf(dest,"%1d",FfToInt(f1)); break;
		case 2: fprintf(dest,"%2d",FfToInt(f1)); break;
		case 3: fprintf(dest,"%3d",FfToInt(f1)); break;
		case 4: fprintf(dest,"%4d",FfToInt(f1)); break;
		case 5: fprintf(dest,"%5d",FfToInt(f1)); break;
		case 6: fprintf(dest,"%6d",FfToInt(f1)); break;
	    }
	    if (iv++ >= mx)
	    {	fprintf(dest,"\n");
		iv = 1;
	    }
	}
	if (iv > 1)
		fprintf(dest,"\n");
    }
}


/* ------------------------------------------------------------------
   prgapmat() - Print a matrix in GAP format.
   ------------------------------------------------------------------ */

static void prgapmat()

{   PTR m1;
    FEL f1;
    FEL gen;
    long loop1, j1;
    int cnt, isprimefield;


    FfSetField(hdr[0]);
    FfSetNoc(hdr[2]);
    gen = FfGen;		/* Generator */
    isprimefield = (FfChar == FfOrder);

    m1 = FfAlloc((long)1);
    PrString("MeatAxe.Matrix := [\n");
    for (loop1 = 1; loop1 <= hdr[1]; ++loop1)
    {
	if (FfReadRows(inpfile,m1,1) != 1)
	    MTX_ERROR1("Cannot read %s",inpname);
	cnt = 0;
	fprintf(dest,"[");
	for (j1 = 0; j1 < hdr[2]; ++j1)
	{   if (cnt > 75)
	    {	fprintf(dest,"\n ");
		cnt = 0;
	    }
	    f1 = FfExtract(m1,j1);
	    if (isprimefield)
	    {   FEL f2=FF_ZERO;
	   	long k=0;
	    	while (f2 != f1)
	    	{   f2 = FfAdd(f2,gen);
		    ++k;
		}
		fprintf(dest,"%ld",k);
		cnt += k>9999?5:k>999?4:k>99?3:k>9?2:1;
	    }
	    else
	    {   if (f1 == FF_ZERO)
	    	{   fprintf(dest,"0*Z(%ld)",hdr[0]);
		    cnt += 5;
		    cnt += hdr[0]>9999?5:hdr[0]>999?4:hdr[0]>99?3:hdr[0]>9?2:1;
		}
		else
		{   FEL f2 = gen;
		    long k = 1;
		    while (f2 != f1)
		    {   f2 = FfMul(f2,gen);
			++k;
		    }
		    fprintf(dest,"Z(%ld)^%ld",hdr[0],k);
		    cnt += 4;
		    cnt += hdr[0]>9999?5:hdr[0]>999?4:hdr[0]>99?3:hdr[0]>9?2:1;
		    cnt += k>9999?5:k>999?4:k>99?3:k>9?2:1;
		}
	    }
	    if (j1 < hdr[2]-1)
	    {	fprintf(dest,",");
		++cnt;
	    }
	}
	fprintf(dest,"]");
	if (loop1 < hdr[1])
	    fprintf(dest,",");
	fprintf(dest,"\n");
    }
    fprintf(dest,"]");
    if (isprimefield)
	fprintf(dest,"*Z(%ld)",hdr[0]);
    fprintf(dest,";\n");
}



/* ------------------------------------------------------------------
   prgapimat() - Print an integer matrix in GAP format.
   ------------------------------------------------------------------ */

static void prgapimat()

{
    long *row;
    long loop1, j1;
    int cnt;

    row = NALLOC(long,hdr[2]);
    PrString("MeatAxe.Matrix := [\n");
    for (loop1 = 1; loop1 <= hdr[1]; ++loop1)
    {
	if (SysReadLong(inpfile,row,hdr[2]) != hdr[2])
	    MTX_ERROR1("Cannot read %s",inpname);
	cnt = 0;
	fprintf(dest,"[");
	for (j1 = 0; j1 < hdr[2]; ++j1)
	{
	    long k = row[j1];
	    if (cnt > 75)
	    {	fprintf(dest,"\n ");
		cnt = 0;
	    }
	    fprintf(dest,"%ld",k);
	    cnt += k>9999?5:k>999?4:k>99?3:k>9?2:1;
	    if (j1 < hdr[2]-1)
	    {	fprintf(dest,",");
		++cnt;
	    }
	}
	fprintf(dest,"]");
	if (loop1 < hdr[1])
	    fprintf(dest,",");
	fprintf(dest,"\n");
    }
    fprintf(dest,"];\n");
}


/* ------------------------------------------------------------------
   prgapperm() - Print a permutation in GAP format.
   ------------------------------------------------------------------ */

static void prgapperm()

{
    long i, pos;
    long *perm;

    /* Allocate memory for one permutation
      ------------------------------------ */
    perm = NALLOC(long,hdr[1]);
    if (perm == NULL)
	MTX_ERROR("Cannot allocate work space");

    PrString("MeatAxe.Perms := [\n");
    for (pos = 1; pos <= hdr[2]; ++pos)
    {
    	/* Read the next permutation
	   ------------------------- */
	if (SysReadLong(inpfile,perm,hdr[1]) != (int) hdr[1])
	    MTX_ERROR1("Cannot read %s",inpname);

	/* Print it
	   -------- */
	PrString("    PermList([");
	for (i = 0; i < hdr[1]; ++i)
	{
	    if (i > 0)
	    	PrString(",");
	    PrLong(perm[i] + 1);
	}
	PrString("])");
	if (pos < hdr[2]) PrString(",");
	PrString("\n");
    }
    PrString("];\n");
}



/* ------------------------------------------------------------------
   prgap() - Print a matrix or permutation in GAP format.
   ------------------------------------------------------------------ */

static void prgap()

{
    if (hdr[0] == -1)
	prgapperm();
    else if (hdr[0] == -8)
	prgapimat();
    else if (hdr[0] >= 2)
	prgapmat();
    else
	MTX_ERROR1("Cannot print type %d in GAP format",(int)hdr[0]);
}



/* ------------------------------------------------------------------
   prpol() - Print a polynomial
   ------------------------------------------------------------------ */

static void prpol()

{
    int i;
    Poly_t *p;

    SysFseek(inpfile,HdrPos);
    if ((p = PolRead(inpfile)) == NULL)
	MTX_ERROR1("Cannot read %s",inpname);


    fprintf(dest,"polynomial field=%ld degree=%ld\n",hdr[1],hdr[2]);
    for (i = 0; i <= p->Degree; ++i)
    {
	PrLong(FfToInt(p->Data[i]));
	PrString(" ");
    }
    PrString("\n");
}


/* ------------------------------------------------------------------
   prperm() - Print a permutation in standard format.
   ------------------------------------------------------------------ */

static int prperm()

{
    Perm_t *perm;
    long f1, i;

    SysFseekRelative(inpfile,-3 * 4);
    if ((perm = PermRead(inpfile)) == NULL)
    {
	MTX_ERROR("%s: Cannot read permutation\n");
	return -1;
    }

    fprintf(dest,"permutation degree=%d\n",perm->Degree);
    for (i = 0; i < perm->Degree; ++i)
    {
	f1 = perm->Data[i];
	PrLong(f1 + 1);
	PrString(" ");
    }
    PrString("\n");
    return 0;
}

/* ------------------------------------------------------------------
   primat() - Print an integer matrix
   ------------------------------------------------------------------ */

static void primat()

{
    long k, i;
    long *row;

    row = NALLOC(long,hdr[2]);
    if (row == NULL)
	MTX_ERROR("Cannot allocate work space");

    fprintf(dest,"integer-matrix rows=%ld cols=%ld\n",hdr[1],
	hdr[2]);
    for (i = 0; i < hdr[1]; ++i)
    {
	if (SysReadLong(inpfile,row,hdr[2]) != (int) hdr[2])
	    MTX_ERROR1("Cannot read %s: %E",inpname);
    	for (k = 0; k < hdr[2]; ++k)
    	{
    	    PrLong(row[k]);
    	    PrString(" ");
    	}
    	PrString("\n");
    }
}



/* ------------------------------------------------------------------
   prmtx() - Print a matrix or permutation in standard format.
   ------------------------------------------------------------------ */

static void prmtx()

{
    if (hdr[0] > 1)
	prmatrix();
    else if (hdr[0] == -1)
	prperm();
    else if (hdr[0] == -2)
	prpol();
    else if (hdr[0] == -8)
	primat();
    else
	MTX_ERROR1("Cannot print type %d in Mtx format",(int)hdr[0]);
}


static void PrintPermutationSummary()
{
    if (Gap)
    {
	printf("MeatAxe.PermutationCount:=%ld;\n",hdr[2]);
	printf("MeatAxe.PermutationDegree:=%ld;\n",hdr[1]);
    }
    else
    {
	printf("%ld Permutation%s of degree %ld\n",
	    hdr[2], hdr[2] == 1 ? "" : "s", hdr[1]);
    }
}

static void PrintMatrixSummary()
{
    if (Gap)
    {
	printf("MeatAxe.MatrixRows:=%ld;\n",hdr[1]);
	printf("MeatAxe.MatrixCols:=%ld;\n",hdr[2]);
	printf("MeatAxe.MatrixField:=%ld;\n",hdr[0]);
    }
    else
    {
	printf("%ld x %ld matrix over GF(%ld)\n",hdr[1],hdr[2],hdr[0]);
    }
}


static void PrintPolySummary()
{
    if (Gap)
    {
	printf("MeatAxe.PolynomialField=%ld\n",hdr[1]);
	printf("MeatAxe.PolynomialDegree=%ld\n",hdr[2]);
    }
    else
	printf("Polynomial of degree %ld over GF(%ld)\n",hdr[2],hdr[1]);
}


static void PrintImatSummary()
{
    if (Gap)
    {
	printf("MeatAxe.IntegerMatrixRows:=%ld;\n",hdr[1]);
	printf("MeatAxe.IntegerMatrixCols:=%ld;\n",hdr[2]);
    }
    else
	printf("%ld x %ld integer matrix\n",hdr[1],hdr[2]);
}

/* ------------------------------------------------------------------
   PrintSummary() - Print header information
   ------------------------------------------------------------------ */

static void PrintSummary()
{
    if (!Gap)
    	printf("%s: ",inpname);
    if (hdr[0] == -1)
    {
	PrintPermutationSummary();
	SysFseek(inpfile,ftell(inpfile) + hdr[1]*hdr[2]*4);
    }
    else if (hdr[0] >= 2)
    {
	PTR x;
	long l;
	PrintMatrixSummary();
	FfSetField(hdr[0]);
	FfSetNoc(hdr[2]);
	x = FfAlloc(1);
	for (l = hdr[1]; l > 0; --l)
	    FfReadRows(inpfile,x,1);
	free(x);
    }
    else if (hdr[0] == -2)
    {
	PrintPolySummary();
	FfSetField(hdr[1]);
	FfSetNoc(hdr[2]+1);
	SysFseek(inpfile,ftell(inpfile) + FfCurrentRowSize);
    }
    else if (hdr[0] == -8)
    {
	PrintImatSummary();
	SysFseek(inpfile,ftell(inpfile)+hdr[1]*hdr[2]*4);
    }
    else
	printf("Unknown file format (%ld,%ld,%ld).\n",hdr[0],hdr[1],hdr[2]);
}





/* ------------------------------------------------------------------
   ReadHeader() - Read file header. Returns 1 on success, 0 on error
   or EOF.
   ------------------------------------------------------------------ */

static int ReadHeader(void)

{
    HdrPos = ftell(inpfile);
    if (feof(inpfile)) return 0;
    if (SysReadLong(inpfile,hdr,3) != 3)
	return 0;
    /* Check the header */
    if (hdr[0] > 65536 || hdr[0] < -20 || hdr[1] < 0 || hdr[2] < 0)
    {
	MTX_ERROR5("%s: %E (%d %d %d)\n",inpname,MTX_ERR_FILEFMT,
	    (int)hdr[0],(int)hdr[1],(int)hdr[2]);
    }
    return 1;
}



static int Init(int argc, const char **argv)
{
    if ((App = AppAlloc(&AppInfo,argc,argv)) == NULL)
	return -1;

    /* Process command line options
       ---------------------------- */
    Gap = AppGetOption(App,"-G --gap");
    Summary = AppGetOption(App,"-s --summary");
    if (Gap)
	MtxMessageLevel = -100;	/* Suppress messages in GAP mode */

    /* Process arguments, open files
       ----------------------------- */
    if (AppGetArguments(App,1,2) < 0)
	return -1;
    inpname = App->ArgV[0];
    inpfile = SysFopen(inpname,FM_READ);
    if (inpfile == NULL)
	return -1;
    if (App->ArgC >= 2)
    {
	dest = SysFopen(App->ArgV[1],FM_CREATE|FM_TEXT);
	if (dest == NULL)
	    return -1;
    }
    else
	dest = stdout;
    return 0;
}


/* ------------------------------------------------------------------
   main()
   ------------------------------------------------------------------ */

int main(int argc, const char **argv)

{
    if (Init(argc,argv) != 0)
    {
	MTX_ERROR("Initialization failed");
	return 1;
    }

    while (ReadHeader())
    {
	if (Summary)
	    PrintSummary();
	else if (Gap)
	    prgap();
	else
	    prmtx();
    }
    fclose(inpfile);
    fclose(dest);
    MtxCleanupLibrary();
    return (EXIT_OK);
}



/**
@page prog_zpr zpr - Print Matrices and Permutations
@see @ref prog_zcv

@section zpr-syntax Command Line
<pre>
zpr [@em Options] [-Gs] @em DataFile [@em TextFile]
</pre>

@par @em Options
  Standard options, see @ref prog_stdopts
@par -G, --gap
  Output in GAP format.
@par -s, --summary
  Show headers only.
@par @em DataFile
  Input file (binary)
@par @em TextFile
  Output file (text)

@section zpr-inp Input Files
@par @em DataFile
  Input file (binary)

@section zpr-out Output Files
@par @em TextFile
  Output file (text)

@section zpr-desc Description
This program prints the contents of a MeatAxe data file in readable
format. The text produced by @b zpr can be converted into binary format by
the @ref prog_zcv "zcv" program.

If there is only one argument on the command line, @b zpr writes to stdout.
A second argument, if present, is taken as the output file name.

To find out the contents of a MeatAxe file, use the -s option. To generate
output readble by GAP, use -G. Both options can be combined.
*/
