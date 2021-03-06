/* ============================= C MeatAxe ==================================
   File:        $Id: bsread.c,v 1.1.1.1 2007/09/02 11:06:16 mringe Exp $
   Comment:     Read a bit string from a file.
   --------------------------------------------------------------------------
   (C) Copyright 1998 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */

#include "meataxe.h"

   
/* --------------------------------------------------------------------------
   Local data
   -------------------------------------------------------------------------- */

MTX_DEFINE_FILE_INFO


/**
 ** @addtogroup bs
 ** @{
 **/

/**
 ** Read a bit string from a file.
 ** @param f File to read from. Must be open for reading.
 ** @return The bit string, 0 on error.
 **/

BitString_t *BsRead(FILE *f)
{
    BitString_t *b;
    long hdr[3];	    /* File header */
    int size;
    
    if (SysReadLong32(f,hdr,3) != 3)
    {
	MTX_ERROR("Cannot read bit string header");
	return NULL;
    }
    if (hdr[0] != -3 || hdr[2] != 0)
    {
	MTX_ERROR3("Invalid bit string header (%d,%d,%d)",(int)hdr[0],
	    (int)hdr[1],(int)hdr[2]);
	return NULL;
    }
    size = (int) hdr[1];
    if (size < 0)
    {
	MTX_ERROR1("Invalid bit string size %d in file)",size);
	return NULL;
    }
    b = BsAlloc(size);
    if (b == NULL)
    {
	MTX_ERROR("Cannot allocate bit string");
	return NULL;
    }
    if (SysReadLongX(f,b->Data,(b->Size + 7) / 8) != (b->Size + 7) / 8)
    {
	MTX_ERROR("Cannot read bit string data");
	BsFree(b);
	return NULL;
    }
    return b;
}


/**
 ** @}
 **/
