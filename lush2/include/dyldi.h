#ifndef _DYLDI_H_
#define _DYLDI_H_

/* Copyright (c) 2012 Jason E. Aten. This file
   meant to be incorporated into Lush (Lisp Universal Shell) and so is Licensed 
   under the same terms and license as Lush. */

#include "header.h"
#include "dh.h"

// this is an Apple OSX only file...
#if HAVE_NSLINKMODULE


// interface / base class for all the templated DyldInfoPrinters
//
struct  DyldInfo {

    // replacement for NSSymbolReferenceCountInObjectFileImage
	virtual int num_import() = 0;

    // replacement for NSSymbolReferenceNameInObjectFileImage
	virtual const char* kth_import(unsigned int k) = 0;

    // replacement for NSSymbolDefinitionCountInObjectFileImage
	virtual int num_export() = 0;

    // replacement for NSSymbolDefinitionNameInObjectFileImage
	virtual const char*  kth_export(unsigned int k) = 0;

    virtual ~DyldInfo() {}
};

extern "C" DyldInfo*  get_bundle_info(const char* path);

#endif /* HAVE_NSLINKMODULE */

#endif /* _DYLDI_H_ */
