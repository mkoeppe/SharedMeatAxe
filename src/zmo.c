/* ============================= C MeatAxe ==================================
   File:        $Id: zmo.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     Make orbits under permutations
   --------------------------------------------------------------------------
   (C) Copyright 1999 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */


#include "meataxe.h"
#include <stdlib.h>
#include <string.h>


#define MAXPERM 50		/* Max. number of permutations */
#define STACKSIZE 100000


/* ------------------------------------------------------------------
   Global data
   ------------------------------------------------------------------ */

MTX_DEFINE_FILE_INFO

static Perm_t *Perm[MAXPERM];			/* Permutations */
static const char *orbname;
static const char *permname;
static int nperm = 2;
static int Degree;
static int Seed = 0;
static long *OrbNo;
static long *OrbSize;
static int NOrbits;
static long Stack[STACKSIZE];
static int Sp = -1;


static MtxApplicationInfo_t AppInfo = {
"zmo", "Make Orbits",
"SYNTAX\n"
"    zmo [<Options>] [-g <#Perms>] <Perm> <Orbits>\n"
"\n"
"OPTIONS\n"
MTX_COMMON_OPTIONS_DESCRIPTION
"    -g <#Perms> ............. Set number of permutations (default: 2)\n"
"    -s <Seed> ............... Set seed point (default: 1)\n"
"\n"
"FILES\n"
"    <Perm>.{1,2...} ......... I Permutations\n"
"    <Orbits> ................ O Orbit table and sizes\n"
};
static MtxApplication_t *App = NULL;







static int ReadPermutations()
{
    int i;
    char fn[200];

    for (i = 0; i < nperm; ++i)
    {
	sprintf(fn,"%s.%d",permname,i+1);
	Perm[i] = PermLoad(fn);
	if (Perm[i] == NULL)
	    return -1;
    }
    Degree = Perm[0]->Degree;
    return 0;
}


static int Init(int argc, const char **argv)

{
    App = AppAlloc(&AppInfo,argc,argv);
    if (App == NULL)
	return -1;

    /* Command line.
       ------------- */
    nperm = AppGetIntOption(App,"-g",2,1,MAXPERM);
    Seed = AppGetIntOption(App,"-s --seed",1,1,1000000) - 1;
    if (AppGetArguments(App,2,2) < 0)
	return -1;
    permname = App->ArgV[0];
    orbname = App->ArgV[1];
    return 0;
}


static void Cleanup()

{
    if (OrbNo!= NULL)
	SysFree(OrbNo);
    AppFree(App);
}


static int AllocWorkspace()
{
    int i;

    OrbNo = NALLOC(long,Degree);
    if (OrbNo == NULL)
	return -1;
    for (i = 0; i < Degree; ++i)
	OrbNo[i] = -1;
    return 0;
}



static int MakeOrbits()
{
    int points_remaining;
    int seedpos = 0;

    MESSAGE(1,("Finding orbits\n"));
    Stack[++Sp] = Seed;
    OrbNo[Seed] = 0;
    NOrbits = 1;
    for (points_remaining = Degree; points_remaining > 0; --points_remaining)
    {
	long pt;
	long orb;
	int i;

	/* If there is somthing on the stack, take it.
	   Otherwise start a new orbit.
	   ------------------------------------------- */
	if (Sp >= 0)
	{
	    pt = Stack[Sp--];
	    orb = OrbNo[pt];
	}
	else
	{
	    for (pt = seedpos; pt < Degree && OrbNo[pt] >= 0; ++pt)
		;
	    if (pt >= Degree)
	    {
		MTX_ERROR("Internal error: no point found to continue");
		return -1;
	    }
	    seedpos = pt + 1;
	    orb = NOrbits++;
	    OrbNo[pt] = orb;
	}

	/* Apply all permutations.
	   ----------------------- */
	for (i = 0; i < nperm; ++i)
	{
	    long image = Perm[i]->Data[pt];
	    if (OrbNo[image] < 0)
	    {
		OrbNo[image] = orb;
		if (Sp >= STACKSIZE - 1)
		{
		    MTX_ERROR("Stack overflow");
		    return -1;
		}
		Stack[++Sp] = image;
	    }
	    else
	    {
		if (OrbNo[image] != orb)
		{
		    MTX_ERROR("Internal error: inconsistent orbit numbers");
	    	    return -1;
		}
	    }
	}
    }

    return 0;
}



static int CalcSizes()
{
    int i;

    MESSAGE(1,("Calculating orbit sizes\n"));
    OrbSize = NALLOC(long,NOrbits);
    if (OrbSize == NULL)
	return -1;
    memset(OrbSize,0,sizeof(long)*NOrbits);
    for (i = 0; i < Degree; ++i)
	++OrbSize[OrbNo[i]];
    return 0;
}



static int WriteOutput()
{
    long hdr[3];
    FILE *f;

    if ((f = SysFopen(orbname,FM_CREATE)) == NULL)
	return -1;

    /* Write the orbit number table.
       ----------------------------- */
    hdr[0] = -8;
    hdr[1] = 1;
    hdr[2] = Degree;
    if (SysWriteLong(f,hdr,3) != 3)
    {
	MTX_ERROR("Error writing file header");
	return -1;
    }
    if (SysWriteLong(f,OrbNo,Degree) != Degree)
    {
	MTX_ERROR("Error writing file orbit number table");
	return -1;
    }

    /* Write the orbit sizes table.
       ---------------------------- */
    hdr[0] = -8;
    hdr[1] = 1;
    hdr[2] = NOrbits;
    if (SysWriteLong(f,hdr,3) != 3)
    {
	MTX_ERROR("Error writing file header");
	return -1;
    }
    if (SysWriteLong(f,OrbSize,NOrbits) != NOrbits)
    {
	MTX_ERROR("Error writing file orbit sizes table");
	return -1;
    }
    fclose(f);

    /* Print orbit sizes
       ----------------- */
    if (MtxMessageLevel >= 0)
    {
	int size[20];
	int count[20];
	int dis = 0;
	int i;
	for (i = 0; i <	NOrbits; ++i)
	{
	    int k;
	    for (k = 0; k < dis && size[k] != OrbSize[i]; ++k);
	    if (k < dis)
		++count[k];
	    else if (dis < 20)
	    {
		size[dis] = OrbSize[i];
		count[dis] = 1;
		++dis;
	    }
	}
	for (i = 0; i < dis; ++i)
	{
	    printf("%6d ORBIT%c OF SIZE %6d\n",
		 count[i],count[i] > 1 ? 'S':' ',size[i]);
	}
    }

    return 0;
}



/* ------------------------------------------------------------------
   main()
   ------------------------------------------------------------------ */

int main(int argc, const char **argv)

{
    if (Init(argc,argv) != 0)
	return 1;
    if (ReadPermutations() != 0)
    {
	MTX_ERROR("Error reading input files");
	return 1;
    }
    if (AllocWorkspace() != 0)
	return 1;
    if (MakeOrbits() != 0)
	return 1;
    if (CalcSizes() != 0)
	return 1;
    if (WriteOutput() != 0)
	return 1;

    Cleanup();
    return (EXIT_OK);
}





/**
@page prog_zmo zmo - Make Orbits

@section zmo-syntax Command Line
<pre>
zmo @em Options [-g @em NPerms] [-s @em Seed] @em Perm @em Orbits
</pre>

@par @em Options
Standard options, see @ref prog_stdopts

@par [-g @em NPerms]
Set the number of permutatins (default is 2).

@par [-s @em Seed]
Set the seed point (default is 1).

@par @em Perm
Permutation base name.

@par @em Orbits
Output file name.

@section zmo-inp Input Files
@par @em Perm.1, @em Perm.2, ...
Permutations.

@section zmo-out Output Files
@par @em Orbits
Orbit number and sizes.


@section zmo-desc Description

This program calculates the orbits under a set of permutations.
By default, the program works with two permutations which are read
from @em Perm.1 and @em Perm.2.  Using the `-g' option you
can specify a different number of permutations (using the same file
name convention). For example,
<pre>
zmo -g 3 p orbs
</pre>
reads three permutations from p.1, p.2, and p.3.
All permutations must have the same degree.

The result is written to @em Orbits and consists of two parts:
- The orbit number table. This is an integer matrix with one
  row and N columns, where N is the degree of the permutations.
  The i-th entry in this table contains the orbit number of
  point i. Note: Orbit numbers start with 0.
- The orbit sizes table. This is an integer matrix with one
  row and K columns, where K is the number of orbits. The
  i-th entry contains the size of the orbit number i.
This file can be fed into @ref prog_zkd "zkd" or @ref prog_zuk "zuk".
At the end, the program prints a message containing the orbit sizes.
Note that at most 20 different orbit sizes are shown here.


@section zmo-impl Implementation Details
The algorithm uses a fixed size stack to store points. At the beginning,
a seed point is searched which has not yes assigned an orbit number.
This point is assigned the next orbit number (beginning with 0) and put on the stack.
The main part consists of taking a point from the stack and applying all generators.
Any new points obtained in this way are assigned the same orbit
number and put on the stack. This step is repeated until the stack is
empty, i.e., the orbit is exhausted. Then, the next seed point is
seached, and its orbit is calculated, and so on, until all orbits
are found.
By default, the first seed point is 1. A different seed point may
be selected with the "-s" option.

The number of permutations must be less than 50. All permutations must
fit into memory at the same time. Also the stack size is limited to
100000 positions.
*/
