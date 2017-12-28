/* ============================= C MeatAxe ==================================
   File:        $Id: error.c,v 1.1.1.1 2007/09/02 11:06:17 mringe Exp $
   Comment:     Error handling
   --------------------------------------------------------------------------
   (C) Copyright 1999 Michael Ringe, Lehrstuhl D fuer Mathematik,
   RWTH Aachen, Germany  <mringe@math.rwth-aachen.de>
   This program is free software; see the file COPYING for details.
   ========================================================================== */

#include "meataxe.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/** @defgroup err Error handling and messages
 ** @details
 ** Errors can occur inside the MeatAxe library for many reasons:
 ** * insufficient system resources (memory, disk space),
 ** * device or operating system failure,
 ** * invalid parameters passed to a MeatAxe library function,
 ** * invalid usage of MeatAxe library functions, for example missing
 **   initialization, etc.
 **
 ** Note that invalid parameters are not always detected by the MeatAxe library.
 ** For example, most kernel functions such as FfAdd() do not check their
 ** arguments for the sake of performance. Thus, calling these function with
 ** invalid arguments may produce random results or even crash the program.
 ** However, most higher-level functions do some plausibility checks on their
 ** arguments before processing them.
 **
 ** When an error is detected, some variant of MtxError() is called, which
 ** extracts information on where and why the error occurred, and calls an
 ** error handler with that information. The default error handler just
 ** terminates the program with an appropriate error message. But code linked
 ** against libmtx.so can of course define a custom error handler.
 **
 ** In SharedMeatAxe 1.0, as opposed to MeatAxe 2.4.24, most higher level
 ** functions return a specific value in the case of an error, and they
 ** propagate these error values. It thus makes sense for a MeatAxe
 ** application to define a custom error handler that stores the
 ** information provided by MtxError(), and then check whether a called
 ** library function returns an error value; if it does, the application
 ** can deal with the error based on the data stored by the error handler.
 **
 ** @section err-usage How to use the error handling framework
 ** Each source file which uses the MeatAxe error mechanism, i.e.,
 ** MtxError() or any of the MTX_ERROR() macros, must also define one
 ** MtxFileInfo_t structure at file scope. This is most conveniently done
 ** by putting the MTX_DEFINE_FILE_INFO macro into the file.
 **
 ** Here is a short example:
 ** @code
 ** #include "meataxe.h"
 **
 ** MTX_DEFINE_FILE_INFO
 **
 ** int divide(int a, int b)
 ** {
 **     if (b == 0)
 **     {
 **       MTX_ERROR("Division by 0");
 **       return 0;
 **     }
 **     return a / b;
 ** }
 ** @endcode
 ** Note that you must not assume that MTX_ERROR terminates the program.
 ** Thus, if another function calls divide(a,b), which returns 0, then it
 ** is possible that an error has occurred. Hence, the callee needs to
 ** check whether there is an error or not, and eventually return its own
 ** error value.
 **
 ** @see MtxError MtxErrorRecord_t MTX_DEFINE_FILE_INFO
 ** @{
 **/

static void (*ErrorHandler)(const MtxErrorRecord_t *err) = NULL;
static FILE *LogFile = NULL;

/**
 ** The default error handler.
 ** It just prints the error message to the logfile, which
 ** typically is stderr, and exits with value 255.
 ** @param e an error record.
 **/

static void DefaultHandler(const MtxErrorRecord_t *e)
{
    static int count = 0;
    if (LogFile == NULL)
	LogFile = stderr;

    if (e->FileInfo != NULL)
	fprintf(LogFile,"%s(%d):",e->FileInfo->BaseName,e->LineNo);
    fprintf(LogFile,"%s\n",e->Text);
    if (--count <= 0)
	exit(255);
}


/**
 ** Define an error handler.
 ** This function defines an error handler that is called every
 ** time an error occurs inside libmtx.so.
 ** @param h
 ** Pointer to the new error handler or NULL to restore the default handler.
 ** @return Returns the previous error handler.
 ** @see MtxErrorRecord_t MtxErrorHandler_t
 **/

MtxErrorHandler_t *MtxSetErrorHandler(MtxErrorHandler_t *h)

{
    MtxErrorHandler_t *old = ErrorHandler;
    ErrorHandler = h;
    return old;
}



/**
 ** Signal an error.
 ** This function is used to report errors. The current error handler
 ** is executed- The default handler just writes an error message
 ** to the error log file (stderr by default) and terminates the program.
 **
 ** The error message, @a text, is formatted with MtxFormatMessage().
 ** @param fi Pointer to a file information structure.
 ** @param line Line number where the error occurred.
 ** @param text Error description.
 ** @param ... Optional arguments to be inserted into the error description.
 ** @return The return value is always 0.
 **/

int MtxError(MtxFileInfo_t *fi, int line, const char *text, ...)

{
    va_list al;
    MtxErrorRecord_t err;
    char txtbuf[2000];

    /* Remove directory names from file name
       ------------------------------------- */
    if (fi != NULL && fi->BaseName == NULL)
    {
	const char *fn;
	for (fn = fi->Name; *fn != 0; ++fn);
	while (fn != fi->Name && fn[-1] != '/' && fn[-1] != '\\')
	    --fn;
	fi->BaseName = fn;
    }

    /* Create error record
       ------------------- */
    err.FileInfo = fi;
    err.LineNo = line;
    err.Text = txtbuf;
    va_start(al,text);
    MtxFormatMessage(txtbuf,sizeof(txtbuf),text,al);
    va_end(al);

    /* Call the error handler
       ---------------------- */
    if (ErrorHandler != NULL)
	ErrorHandler(&err);
    else
	DefaultHandler(&err);

    return 0;
}

/**
 ** @fn MTX_DEFINE_FILE_INFO
 ** This macro must be included in each source file that uses the
 ** MeatAxe error reporting mechanism. It defines a data structure
 ** of type MtxFileInfo_t that is used by the error reporting macros
 ** @see MtxError
 **/

/** @} **/
