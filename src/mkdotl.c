/* ============================= C MeatAxe ==================================
   File:        $Id: mkdotl.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     This program calculates the dotted lines.
   --------------------------------------------------------------------------
   (C) Copyright 1998 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */


#include "meataxe.h"
#include <string.h>
#include <stdlib.h>


/* --------------------------------------------------------------------------
   Global data
   -------------------------------------------------------------------------- */

MTX_DEFINE_FILE_INFO

MatRep_t *Rep;			/* Generators of the current constituent */
Matrix_t *cycl = NULL;		/* List of cyclic submodules */
long *class[MAXCYCL];		/* Classes of vectors */
long nmountains = 0;		/* Number of mountains */
Matrix_t *mountlist[MAXCYCL];	/* Mountains */
BitString_t *subof[MAXCYCL];	/* Incidence matrix */
int cfstart[LAT_MAXCF+1];	/* First mountain of each c.f. */
char lck[MAXCYCL];
char lck2[MAXCYCL];
BitString_t *dotl[MAXDOTL];	    /* Dotted lines */
BitString_t *MaxMountains[MAXDOTL]; /* Maximal mountains in dotted lines */
int ndotl = 0;			/* Number of dotted-lines in <dotl> */
int firstdotl = 0;		/* Used for locking */
int firstm, nextm;		/* First and last+1 mountain for the */
				/* current constituent */

long *sumdim[MAXCYCL];
    /* sumdim[i][j] contains the dimension of mountain[i] + mountain[j].
       Most of the program's work constists in comparing sums of pairs of
       mountains and looking if they are equal. Thus, a given sum may be
       needed many times. Since we don't keep the sums in memory we have
       to recalculate them each time. But comparing the dimensions we can
       avoid many unnecessary calculations. */

int dotlen;
    /* This is the length of a dotted line for the current constituent.
       For theoretical reasions the is always Q+1, where GF(Q) is the
       splitting field for the constituent. */

int opt_G = 0;			    /* GAP output */
static int Opt_FindDuplicates = 0;  /* Find 'duplicate' dotted-lines */
static Lat_Info LI;		    /* Data from .cfinfo file */



static MtxApplicationInfo_t AppInfo = {
"mkdotl", "Find Dotted-Lines",
"\n"
"SYNTAX\n"
"    mkdotl [<Options>] <Name>\n"
"\n"
"ARGUMENTS\n"
"    <Name> .................. Name of the representation\n"
"\n"
"OPTIONS\n"
MTX_COMMON_OPTIONS_DESCRIPTION
"    -G ...................... GAP output (implies -Q)\n"
"    --nodup ................. Find, and discard, duplicate dotted lines\n"
"\n"
"FILES\n"
"    <Name>.cfinfo ........... IO Constituent info file\n"
"    <Name><Cf>.v ............ I  Cyclic submodules, generated by MKCYCL\n"
"    <Name>.inc .............. I  Incidence matrix generated by MKINC\n"
"    <Name>.mnt .............. I  Mountain data (from MKINC)\n"
"    <Name>.dot .............. O  Dotted-lines\n"
};

static MtxApplication_t *App = NULL;



/* -----------------------------------------------------------------
   ReadFiles()
   ----------------------------------------------------------------- */

static void ReadFiles(const char *basename)

{
    FILE *f;
    char fn[200];
    int i;

    if (Lat_ReadInfo(&LI,basename) != 0)
    {
	MTX_ERROR1("Error reading %s.cfinfo",basename);
	return;
    }

    cfstart[0] = 0;
    for (i = 1; i <= LI.NCf; ++i)
	cfstart[i] = cfstart[i-1] + (int)(LI.Cf[i-1].nmount);

    /* Read the incidence matrix
       ------------------------- */
    sprintf(fn,"%s.inc",LI.BaseName);
    f = SysFopen(fn,FM_READ);
    if (f == NULL)
    {
	MTX_ERROR1("Cannot open %s",fn);
	return;
    }
    SysReadLong(f,&nmountains,1);
    if (nmountains != cfstart[LI.NCf])
    {
	MTX_ERROR("Bad number of mountains in .inc file");
	return;
    }
    MESSAGE(1,("Reading incidence matrix (%ld mountains)\n",
	nmountains));
    fflush(stdout);
    for (i = 0; i < (int) nmountains; ++i)
	subof[i] = BsRead(f);
    fclose(f);

    for (i = 0; i < (int) nmountains; ++i)
    {
	sumdim[i] = NALLOC(long,nmountains);
	memset(sumdim[i],0,(size_t)nmountains * sizeof(long));
    }

    /* Read classes
       ------------ */
    sprintf(fn,"%s.mnt",LI.BaseName);
    MESSAGE(1,("Reading classes (%s)\n",fn));
    f = SysFopen(fn,FM_READ);
    if (f == NULL)
    {
	MTX_ERROR1("Cannot open %s",fn);
	return;
    }
    for (i = 0; i < nmountains; ++i)
    {
	long mno, mdim, nvec, *p;
	int k;
	if (fscanf(f,"%ld%ld%ld",&mno,&mdim,&nvec) != 3 ||
	    mno != i || nvec < 1  || mdim < 1)
	{
	    MTX_ERROR("Invalid .mnt file");
	    return;
	}
	p = class[i] = NALLOC(long,nvec+2);
	*p++ = nvec;
	for (k = 0; k < nvec; ++k, ++p)
	{
	    if (fscanf(f,"%ld",p) != 1 || *p < 1)
	    {
		MTX_ERROR("Invalid .mnt file");
		return;
	    }
	}
	if (fscanf(f,"%ld",p) != 1 || *p != -1)
	{
	    MTX_ERROR("Invalid .mnt file");
	    return;
	}
    }
}




/* -----------------------------------------------------------------
   mkmount() - Make mountain
   ----------------------------------------------------------------- */

static void mkmount(int i)

{
    Matrix_t *seed;
    PTR x, y;
    long *p;

    seed = MatAlloc(cycl->Field,class[i][0],cycl->Noc);
    x = seed->Data;
    FfSetNoc(cycl->Noc);
    for (p = class[i] + 1; *p > 0; ++p)
    {
	if (*p < 1 || *p > cycl->Nor)
	{
	    MTX_ERROR("BAD VECTOR IN CLASS");
	    return;
	}
	y = MatGetPtr(cycl,*p - 1);
	FfCopyRow(x,y);
	FfStepPtr(&x);
    }

    mountlist[i] = SpinUp(seed,Rep,SF_EACH|SF_COMBINE,NULL,NULL);
    if (mountlist[i] == NULL)
    {
	MTX_ERROR("Cannot spin up mountain");
	return;
    }
    MatFree(seed);
}



/* -----------------------------------------------------------------
   nextcf() - Initialize everything for the next composition
	factor: Read generators and vectors, calculate mountains...
   ----------------------------------------------------------------- */

static void nextcf(int cf)

{
    char fn[200];
    int j;

    /* Read the generators of the condensed module
       ------------------------------------------- */
    sprintf(fn,"%s%s.%%dk",LI.BaseName,Lat_CfName(&LI,cf));
    Rep = MrLoad(fn,LI.NGen);

    /* Read generating vectors for the cyclic submodules
       ------------------------------------------------- */
    sprintf(fn,"%s%s.v",LI.BaseName,Lat_CfName(&LI,cf));
    cycl = MatLoad(fn);

    /* Calculate the length of dotted-lines. This is always
       Q + 1 where Q is the splitting field order.
       ---------------------------------------------------- */
    dotlen = FfOrder;
    for (j = LI.Cf[cf].spl - 1; j > 0; --j)
    	dotlen *= FfOrder;
    ++dotlen;
    MESSAGE(1,("Length of dotted-lines is %d\n",dotlen));

    /* Calculate the mountains
       ----------------------- */
    for (j = cfstart[cf]; j < cfstart[cf+1]; ++j)
	mkmount(j);
}



/* -----------------------------------------------------------------
   ----------------------------------------------------------------- */

static void CleanupCf()

{
    MatFree(cycl);
    MrFree(Rep);
}


/* -----------------------------------------------------------------
   sum() - Sum of two mountains
   ----------------------------------------------------------------- */

static Matrix_t *sum(int i, int k)

{
    Matrix_t *s;
    int dim_i, dim_k;

    dim_i = mountlist[i]->Nor;
    dim_k = mountlist[k]->Nor;

    s = MatAlloc(FfOrder,dim_i + dim_k,mountlist[i]->Noc);

    MatCopyRegion(s,0,0,mountlist[i],0,0,dim_i,-1);
    MatCopyRegion(s,dim_i,0,mountlist[k],0,0,dim_k,-1);

    /* OLD:
    memcpy(s->Data,mountlist[i]->Data,FfCurrentRowSize * mountlist[i]->Nor);
    x = FfGetPtr(s->Data,mountlist[i]->Nor,FfNoc);
    memcpy(x,mountlist[k]->Data,FfCurrentRowSize * mountlist[k]->Nor);
    */

    MatEchelonize(s);

    /* Remember the dimension of the sum. We use this information
       later to avoid unnecessary recalculations.
       ----------------------------------------------------------- */
    sumdim[i][k] = sumdim[k][i] = s->Nor;
    return s;
}



/* -----------------------------------------------------------------
   lock()
   ----------------------------------------------------------------- */

static void lock(int i, char *c)

{
    int l, m;
    BitString_t *b;

    memset(c,0,sizeof(lck));
    for (m = firstm; m < nextm; ++m)
    {
	if (BsTest(subof[i],m) || BsTest(subof[m],i))
	    c[m] = 1;
    }
    for (l = firstdotl; l < ndotl; ++l)
    {
	b = dotl[l];
	if (!BsTest(b,i))
	    continue;
	for (m = firstm; m < nextm; ++m)
	{
	    if (BsTest(b,m))
		c[m] = 1;
	}
    }
}



static void FindMaxMountains(Matrix_t *span, BitString_t *bs)

{
    int m;

    BsClearAll(bs);
    for (m = firstm; m < nextm; ++m)
    {
	Matrix_t *tmp = MatDup(mountlist[m]);
	MatClean(tmp,span);
	if (tmp->Nor == 0)	    /* Mountain is countained in <span> */
	    BsSet(bs,m);
	MatFree(tmp);
    }

    /* Remove non-maximal mountains */
    for (m = firstm; m < nextm; ++m)
    {
	int k;
	if (!BsTest(bs,m))
	    continue;
	for (k = firstm; k < nextm; ++k)
	{
	    if (k != m && BsTest(subof[k],m))
		BsClear(bs,k);
	}
    }
}




/* -----------------------------------------------------------------
   trydot() - Find out if mountains #i and #k generate a dotted
	line.
   ----------------------------------------------------------------- */

static void trydot(int i, int k, int beg, int next)

{
    Matrix_t *span, *sp;
    BitString_t *dot;
    int count, l, m;

/*
     OPTIMIERUNG, FUNKTIONIERT NICHT!

    if (i == 1 && k == 13)
	i = 1;

    for (l = 0; l < ndotl; ++l)
    {
	if (BsTest(MaxMountains[l],i) && BsTest(MaxMountains[l],k))
	    return;
    }
*/
    lock(k,lck2);
    dot = BsAlloc(nmountains);
    BsSet(dot,i);
    BsSet(dot,k);
    span = sum(i,k);
    count = 2;
    for (l = beg; l < next && count < dotlen; ++l)
    {
	int abort = 0;
	if (lck[l] || lck2[l])
	    continue;
	for (m = i; !abort && m < l; ++m)
	{
	    if (!BsTest(dot,m))
		continue;
	    if (sumdim[l][m] != 0 && sumdim[l][m] != span->Nor)
	    	abort = 1;
	    else
	    {
		sp = sum(l,m);
		abort = (sp->Nor != span->Nor) || !IsSubspace(span,sp,0);
		MatFree(sp);
	    }
	}
	if (!abort)
	{
	    BsSet(dot,l);
	    ++count;
	    lck[l] = 1;
	}
    }
    if (count == dotlen)	/* We have found a dotted line */
    {
	int d;

	MESSAGE(1,("New dotted line: %d+%d\n",i,k));
	if (ndotl >= MAXDOTL)
	{
	    MTX_ERROR1("Too many dotted lines (max %d)",MAXDOTL);
	    return;
	}
	dotl[ndotl] = dot;
	if (Opt_FindDuplicates)
	{
	    MaxMountains[ndotl] = BsDup(dot);
	    FindMaxMountains(span,MaxMountains[ndotl]);
	    for (d = 0; d < ndotl; ++d)
	    {
		if (BsCompare(MaxMountains[ndotl],MaxMountains[d]) == 0)
		    break;
	    }
	    if (d < ndotl)
	    {
		BsFree(dot);
		BsFree(MaxMountains[ndotl]);
		MESSAGE(2,("Discarding %d+%d (= dl %d)\n",i,k,d));
	    }
	    else
		ndotl++;
	}
	else
	    ndotl++;

    }
    else
	free(dot);
    MatFree(span);
}


/* --------------------------------------------------------------------------
   mkdot() - Find dotted-lines in one constituent

   Arguments:
     <cf>: Index of the constituent.

   Description:
     This function finds all dotted-lines corresponding to a given
     constituent.
   -------------------------------------------------------------------------- */

static void mkdot(int cf)

{
    int i, k;

    firstm = cfstart[cf];
    nextm = cfstart[cf+1];
    firstdotl = ndotl;
    for (i = firstm; i < nextm; ++i)
    {
	MESSAGE(2,("Trying mountain %d\n",i));
	lock(i,lck);
    	for (k = i+1; k < nextm; ++k)
    	{
	    if (lck[k])
		continue;
    	    trydot(i,k,k+1,nextm);
    	}
    }
}



/* -----------------------------------------------------------------
   WriteResult() - Write dotted lines.
   ----------------------------------------------------------------- */

static void WriteResult()

{
    FILE *f;
    char fn[200];
    int i;
    long l;

    strcat(strcpy(fn,LI.BaseName),".dot");
    MESSAGE(1,("Writing %s (%d dotted line%s)\n",
	fn,ndotl,ndotl!=1 ? "s" : ""));
    f = SysFopen(fn,FM_CREATE);
    if (f == NULL)
    {
	MTX_ERROR1("Cannot open %s",fn);
	return;
    }
    l = (long) ndotl;
    SysWriteLong(f,&l,1);
    for (i = 0; i < ndotl; ++i)
	BsWrite(dotl[i],f);
    fclose(f);
    Lat_WriteInfo(&LI);
}


/* -----------------------------------------------------------------
   WriteResultGAP() - Write dotted lines in GAP format.
   ----------------------------------------------------------------- */

static void WriteResultGAP()

{
    int i,j ;

    printf ("MeatAxe.DottedLines := [\n") ;
    for (i = 0; i < ndotl; ++i)
    {
	printf( "BlistList([" );
	for (j = 0 ; j < nmountains ; j++)
        printf( j < (nmountains - 1) ? "%s," : "%s], [1])" ,
	       BsTest(dotl[i],j) ? "1" : "0" );
        printf( ",\n" );
    }
    printf( "];\n" );
}




/* -------------------------------------------------------------------------
   Init() - Program initialization
   ------------------------------------------------------------------------- */

static int Init(int argc, const char **argv)

{
    if ((App = AppAlloc(&AppInfo,argc,argv)) == NULL)
	return -1;

    /* Parse command line
       ------------------ */
    opt_G = AppGetOption(App,"-G --gap");
    Opt_FindDuplicates = AppGetOption(App,"--nodup");
    if (opt_G)
	MtxMessageLevel = -100;
    if (AppGetArguments(App,1,1) != 1)
	return -1;
    MESSAGE(0,("*** DOTTED LINES ***\n\n"));

    ReadFiles(App->ArgV[0]);
    return 0;
}



/* -----------------------------------------------------------------
   main()
   ----------------------------------------------------------------- */


int main(int argc, char *argv[])

{
    int i, nn = 0;

    if (Init(argc,(const char **)argv) != 0)
	return -1;
    for (i = 0; i < LI.NCf; ++i)
    {
	nextcf(i);
	mkdot(i);
	LI.Cf[i].ndotl = ndotl - nn;
	MESSAGE(0,("%s%s: %d vectors, %ld mountains, %ld dotted line%s\n",
	    LI.BaseName,Lat_CfName(&LI,i),  cycl->Nor,LI.Cf[i].nmount,
	    LI.Cf[i].ndotl, LI.Cf[i].ndotl != 1 ? "s": ""));
	nn = ndotl;
	CleanupCf();
    }

/*
    for (i = 0; i < ndotl; ++i)
    {
	int k;
	printf("%3d:",i);
	for (k = 0; k < nmountains; ++k)
	    putc(BsTest(dotl[i],k) ? 'x' : '.',stdout);
	printf("\n");
	printf("    ");
	for (k = 0; k < nmountains; ++k)
	    putc(BsTest(MaxMountains[i],k) ? 'x' : '.',stdout);
	printf("\n");
    }
*/
    WriteResult();
    if (opt_G)
         WriteResultGAP();
    AppFree(App);
    return 0;
}


/**
@page prog_mkdotl mkdotl - Find Dotted-lines

@section mkdotl-syntax Command Line
<pre>
mkdotl [@em Options] [-G] [--nodup] @em Name
</pre>

@par @em Options
  Standard options, see @ref prog_stdopts
@par -G
  Produce output in GAP format.
@par --nodup
  Eliminate redundant dotted-lines.
@par @em Name
  Name of the representation.

@section mkdotl-inp Input Files
@par @em Name.cfinfo
  Constituent info file.
@par @em NameCf.v
  Cyclic submodules, generated by @ref prog_mkcycl "mkcycl".
@par @em Name.inc
  Incidence matrix, created by @ref prog_mkinc "mkinc".
@par @em Name.mnt
  Mountain data, created by @ref prog_mkinc "mkinc".

@section mkdotl-out Output Files
@par @em Name.dot
  Dotted-lines.

@section mkdotl-desc Description
This program calculates a set of dotted lines between the local
submodules. More precisely, it computes one dotted line for
each submodule with head isomoprphic to S⊕S, S irreducible.
It can be shown that this set of dotted lines is sufficient to
determine the complete submodule lattice as described by
Benson and Conway.

Input for this program are the incidence matrix calculated by
@ref prog_mkinc "mkinc" and the cyclic submodules from @ref prog_mkcycl "mkcycl".
Again, the whole calculation takes place in the condensed
modules, so there is no need to uncondense the cyclic submodules.

It is known that all dotted lines have length q+1, where q is the
order of the splitting field. This information is used by the program
to determine if a dotted line is complete.

A list of all dotted lines is written to @em Name.dot.

Using the option "--nodup" eliminates redundant dotted-lines from the output.
If this option is specified, the program will calculate, for each dotted-line,
the maximal mountains contained in the span of the dotted-line.
If a dotted-line has the same set of maximal mountains as an earlier
dotted-line, it is considered as redundant and dropped. Note that
"--nodup" increases both memory and CPU time usage. However, the
subsequent step, @ref prog_mkgraph "mkgraph",
will benefit from a reduction of the number of dotted-lines.

**/

