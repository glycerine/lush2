/* Copyright (c) 2012 Jason E. Aten. This file
   meant to be incorporated into Lush (Lisp Universal Shell) and so is Licensed 
   under the same terms and license as Lush. */

/* This code incorporates code from Apple's source for dyldinfo.cpp. See license below. */
/* from http://www.opensource.apple.com/source/ld64/ld64-128.2/src/other/dyldinfo.cpp?txt  */


/*
JEA notes:

 note: although lots of Bundle functions from dyld.h are deprecated in 10.6, the only really critical
   missing functionality from the OSX 10.6 API is the four functions:

  // broken in OSX 10.6: NSSymbolReferenceCountInObjectFileImage, use DyldInfo::num_import()
  // broken in OSX 10.6: NSSymbolReferenceNameInObjectFileImage,  use DyldInfo::kth_import()
  // broken in OSX 10.6: NSSymbolDefinitionCountInObjectFileImage, use DyldInfo::num_export()
  // broken in OSX 10.6: NSSymbolDefinitionNameInObjectFileImage,  use DyldInfo::kth_export()

  The methods on DyldInfo below, num_export(), num_import(), kth_export(), kth_import() provide replacement functionality.

  That is the point of this file, dyldinfo.cpp.

  - J.E.A. 27 March 2012

 */


/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include "header.h"
#include "dh.h"

// this is an Apple OSX only file...
#if HAVE_NSLINKMODULE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <vector>
#include <set>
#include <ext/hash_set>
#include <ext/hash_map>

#include "dyldinfo.hpp"

#include "MachOFileAbstraction.hpp"
#include "Architectures.hpp"
#include "MachOTrie.hpp"

bool printRebase = false;
bool printBind = false;
bool printWeakBind = false;
bool printLazyBind = false;
bool printOpcodes = false;
bool printExport = false;
bool printExportGraph = false;
bool printExportNodes = false;
bool printSharedRegion = false;
bool printFunctionStarts = false;
bool printDylibs = false;
cpu_type_t	sPreferredArch = 0;
cpu_type_t	sPreferredSubArch = 0;


void show_xf(ExportFuncHashTable& t) {
  xfit b = t.begin();
  xfit e = t.end();
  for(; b !=e; ++b) {
    printf("keyname:'%s'  value {  _name:'%s'  _addr:0x%0llX   _exported:%d }\n",
	   b->first,
	   (b->second)._name,
	   (b->second)._addr,
	   (b->second)._exported);
  }
}


void show_svec(symvec& t) {
  svit b = t.begin();
  svit e = t.end();
  for(; b !=e; ++b) {
    printf("name:'%s'\n",*b);
  }
}


void checkIfIsFunctionToo(ExportFuncHashTable& t, const char* exported_name, uint64_t addr) {
  xfit f = t.find(exported_name);
  if (f != t.end()) {

    // sanity check
    if (addr) { 
      assert(f->second._addr == addr); 
    }

    // mark it as exported
    f->second._exported=true;
  }
}

void delete_if_not_expfunc(ExportFuncHashTable& t) {
  xfit b = t.begin();
  xfit e = t.end();
  for(; b !=e; ++b) {
    if (!(b->second._exported)) {
      t.erase(b->first);
      b = t.begin();
      e = t.end();
    }
  }
}

// end Exported Function helpers


__attribute__((noreturn))
void throwf(const char* format, ...) 
{
	va_list	list;
	char*	p;
	va_start(list, format);
	vasprintf(&p, format, list);
	va_end(list);
	
	const char*	t = p;
	fprintf(stderr,"dyldinfo.cpp throwf() sees error: %s\n",t);
	throw t;
}





extern "C" DyldInfo*  get_bundle_info(const char* path)
{
	struct stat stat_buf;

    int fd = -1;	
	try {
		fd = ::open(path, O_RDONLY, 0);
		if ( fd == -1 )
			throw "cannot open file";
		if ( ::fstat(fd, &stat_buf) != 0 ) 
			throwf("fstat(%s) failed, errno=%d\n", path, errno);
		uint32_t length = stat_buf.st_size;
		uint8_t* p = (uint8_t*)::mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
		if ( p == ((uint8_t*)(-1)) ) {
			::close(fd);
			fd=-1;
			throw "cannot map file";
		}
		::close(fd);
		const mach_header* mh = (mach_header*)p;
		if ( mh->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
			const struct fat_header* fh = (struct fat_header*)p;
			const struct fat_arch* archs = (struct fat_arch*)(p + sizeof(struct fat_header));
			for (unsigned long i=0; i < OSSwapBigToHostInt32(fh->nfat_arch); ++i) {
				size_t offset = OSSwapBigToHostInt32(archs[i].offset);
				size_t size = OSSwapBigToHostInt32(archs[i].size);
				cpu_type_t cputype = OSSwapBigToHostInt32(archs[i].cputype);
				cpu_type_t cpusubtype = OSSwapBigToHostInt32(archs[i].cpusubtype);
				if ( ((cputype == sPreferredArch) 
					&& ((sPreferredSubArch==0) || (sPreferredSubArch==cpusubtype)))
					|| (sPreferredArch == 0) ) {	
					switch(cputype) {
					case CPU_TYPE_POWERPC:
						if ( DyldInfoPrinter<ppc>::validFile(p + offset) )
							return DyldInfoPrinter<ppc>::make(p + offset, size, path, (sPreferredArch == 0), stat_buf.st_size);
						else
							throw "in universal file, ppc slice does not contain ppc mach-o";
						break;
					case CPU_TYPE_I386:
						if ( DyldInfoPrinter<x86>::validFile(p + offset) )
							return DyldInfoPrinter<x86>::make(p + offset, size, path, (sPreferredArch == 0), stat_buf.st_size);
						else
							throw "in universal file, i386 slice does not contain i386 mach-o";
						break;
					case CPU_TYPE_POWERPC64:
						if ( DyldInfoPrinter<ppc64>::validFile(p + offset) )
							return DyldInfoPrinter<ppc64>::make(p + offset, size, path, (sPreferredArch == 0), stat_buf.st_size);
						else
							throw "in universal file, ppc64 slice does not contain ppc64 mach-o";
						break;
					case CPU_TYPE_X86_64:
						if ( DyldInfoPrinter<x86_64>::validFile(p + offset) )
							return DyldInfoPrinter<x86_64>::make(p + offset, size, path, (sPreferredArch == 0), stat_buf.st_size);
						else
							throw "in universal file, x86_64 slice does not contain x86_64 mach-o";
						break;
					case CPU_TYPE_ARM:
						if ( DyldInfoPrinter<arm>::validFile(p + offset) ) 
							return DyldInfoPrinter<arm>::make(p + offset, size, path, (sPreferredArch == 0), stat_buf.st_size);
						else
							throw "in universal file, arm slice does not contain arm mach-o";
						break;
					default:
							throwf("in universal file, unknown architecture slice 0x%x\n", cputype);
					}
				}
			}
		}
		else if ( DyldInfoPrinter<x86>::validFile(p) ) {
			return DyldInfoPrinter<x86>::make(p, length, path, false, stat_buf.st_size);
		}
		else if ( DyldInfoPrinter<ppc>::validFile(p) ) {
			return DyldInfoPrinter<ppc>::make(p, length, path, false, stat_buf.st_size);
		}
		else if ( DyldInfoPrinter<ppc64>::validFile(p) ) {
			return DyldInfoPrinter<ppc64>::make(p, length, path, false, stat_buf.st_size);
		}
		else if ( DyldInfoPrinter<x86_64>::validFile(p) ) {
			return DyldInfoPrinter<x86_64>::make(p, length, path, false, stat_buf.st_size);
		}
		else if ( DyldInfoPrinter<arm>::validFile(p) ) {
			return DyldInfoPrinter<arm>::make(p, length, path, false, stat_buf.st_size);
		}
		else {
			throw "not a known file type";
		}
	}
	catch (const char* msg) {
		throwf("%s in %s", msg, path);
	}

  return NULL;
}


#endif /* HAVE_NSLINKMODULE */
