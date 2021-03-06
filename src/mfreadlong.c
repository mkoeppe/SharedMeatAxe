/* ============================= C MeatAxe ==================================
   File:        $Id: mfreadlong.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     Read long integers.
   --------------------------------------------------------------------------
   (C) Copyright 1999 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */

#include "meataxe.h"
#include <string.h>

   
/* --------------------------------------------------------------------------
   Local data
   -------------------------------------------------------------------------- */

MTX_DEFINE_FILE_INFO





/**
!section obj.file
 ** Read long integers from a file.
 ** @param f
    Pointer to the file.
 ** @param buf
    Data buffer.
 ** @param count
    Number of integers to read.
 ** @return
    Number of integers that were actually read. Any value other than |count|
    indicates an error.
!description
    This function reads |count| long integers from a data file into a buffer.
    If necessary, the data is converted from machine independent format into
    the format needed by the platform. See |SysReadLong()| for details.
 ** @see SysReadLong MfWriteLong
 **/

int MfReadLong(MtxFile_t *f, long *buf, int count)

{
    int rc;
    if (!MfIsValid(f))
	return -1;
    rc = SysReadLong(f->File,buf,count);
    if (rc < 0)
	MTX_ERROR1("%s: read failed",f->Name);
    return rc;
}
