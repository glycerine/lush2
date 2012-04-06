#ifndef _DYLDINFO_HPP_
#define _DYLDINFO_HPP_

/* Copyright (c) 2012 Jason E. Aten. This file
   meant to be incorporated into Lush (Lisp Universal Shell) and so is Licensed 
   under the same terms and license as Lush. */

/* This code incorporates code from Apple's source for dyldinfo.cpp. See license below. */
/* from http://www.opensource.apple.com/source/ld64/ld64-128.2/src/other/dyldinfo.cpp?txt  */


/*
  JEA notes:

  note: although lots of Bundle functions from dyld.h are depracated, the only really critical
  missing functionality from the OSX 10.6 API is the four functions:

  // broken in OSX 10.6: NSSymbolReferenceCountInObjectFileImage, use DyldInfo::num_import()
  // broken in OSX 10.6: NSSymbolReferenceNameInObjectFileImage,  use DyldInfo::kth_import()
  // broken in OSX 10.6: NSSymbolDefinitionCountInObjectFileImage, use DyldInfo::num_export()
  // broken in OSX 10.6: NSSymbolDefinitionNameInObjectFileImage,  use DyldInfo::kth_export()

  The methods on DyldInfo below, num_export(), num_import(), kth_export(), kth_import() provide replacement functionality.

  That is the point of this file, dyldi.cpp.

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

#include "dyldi.h"


#include "MachOFileAbstraction.hpp"
#include "Architectures.hpp"
#include "MachOTrie.hpp"
#include "reloc.h"

extern bool printRebase;
extern bool printBind;
extern bool printWeakBind;
extern bool printLazyBind;
extern bool printOpcodes;
extern bool printExport;
extern bool printExportGraph;
extern bool printExportNodes;
extern bool printSharedRegion;
extern bool printFunctionStarts;
extern bool printDylibs;
extern cpu_type_t sPreferredArch;
extern cpu_type_t sPreferredSubArch;



// start Exported Function helpers
// == a hash table to isolate intersection of exported symbols and function start symbols

struct ExportedFunction_t {
    const char* _name;
    uint64_t    _addr;
    bool        _exported;

    ExportedFunction_t(  const char* name,
			 uint64_t    addr,
			 bool        exported) :
	_name(name),
	_addr(addr),
	_exported(exported) {}
};



class ExportFuncEquals
{
public:
    bool operator()(const char* left, const char* right) const { 
	return (strcmp(left, right) == 0); 
    }
};

typedef __gnu_cxx::hash_map<const char*, ExportedFunction_t, __gnu_cxx::hash<const char*>, ExportFuncEquals>  ExportFuncHashTable;
typedef ExportFuncHashTable::iterator xfit;
typedef ExportFuncHashTable::value_type  xfte;

typedef std::vector<const char*> symvec;
typedef symvec::iterator           svit;



__attribute__((noreturn))
void throwf(const char* format, ...);

 
class CStringEquals
{
public:
    bool operator()(const char* left, const char* right) const { return (strcmp(left, right) == 0); }
};

typedef __gnu_cxx::hash_set<const char*, __gnu_cxx::hash<const char*>, CStringEquals>  StringSet;
typedef StringSet::iterator StringSetIt;




template <typename A>
class DyldInfoPrinter : public DyldInfo
{
public:
    static bool         validFile(const uint8_t* fileContent);
    static DyldInfoPrinter<A>*     make(const uint8_t* fileContent, uint32_t fileLength, const char* path, bool printArch, size_t mmap_len) 
    { return new DyldInfoPrinter<A>(fileContent, fileLength, path, printArch, mmap_len); }
    virtual          ~DyldInfoPrinter() {
	if (fPath) free((void*)fPath);
	if (fDylibs.size()) {
	    std::vector<const char*>::iterator e = fDylibs.end();
	    std::vector<const char*>::iterator b = fDylibs.begin();
	    for (; b != e; ++b) {
		if (*b) free((void*)*b);
	    }
	}
	if (fHeader) {
	    munmap((caddr_t)fHeader, _mmap_len);
	}
    }

    // replacement for NSSymbolReferenceCountInObjectFileImage
    int num_import() { return _imports_minus_exports_svec.size(); }

    // replacement for NSSymbolReferenceNameInObjectFileImage
    const char* kth_import(unsigned int k)  { 
	if (k < _imports_minus_exports_svec.size()) { return _imports_minus_exports_svec[k];} else return NULL; 
    }


    // replacement for NSSymbolDefinitionCountInObjectFileImage
    int num_export() { 
	// can't just be:_export_and_function_svec, as it doesn't have sufficient info.
	return _exportsvec.size();
    }
 
    // replacement for NSSymbolDefinitionNameInObjectFileImage
    const char*  kth_export(unsigned int k) { 
	if (k < _exportsvec.size()) { return _exportsvec[k];} else return NULL;
    }
 


private:
    typedef typename A::P     P;
    typedef typename A::P::E    E;
    typedef typename A::P::uint_t   pint_t;

    DyldInfoPrinter(const uint8_t* fileContent, uint32_t fileLength, const char* path, bool printArch, size_t mmap_len);
    void          printRebaseInfo();
    void          printRebaseInfoOpcodes();
    void          printBindingInfo();
    void          printWeakBindingInfo();
    void          printLazyBindingInfo();
    void          printBindingInfoOpcodes(bool weakBinding);
    void          printWeakBindingInfoOpcodes();
    void          printLazyBindingOpcodes();
    void          printExportInfo();
    void          printExportInfoGraph();
    void          printExportInfoNodes();
    void          printRelocRebaseInfo();
    void          printSymbolTableExportInfo();
    void          printClassicLazyBindingInfo();
    void          printClassicBindingInfo();
    void          printSharedRegionInfo();
    void          printFunctionStartsInfo();
    void          printDylibsInfo();
    void          printFunctionStartLine(uint64_t addr, const char** funcname);
    const uint8_t*        printSharedRegionInfoForEachULEB128Address(const uint8_t* p, uint8_t kind);
    pint_t          relocBase();
    const char*         relocTypeName(uint8_t r_type);
    uint8_t          segmentIndexForAddress(pint_t addr);
    void          processExportGraphNode(const uint8_t* const start, const uint8_t* const end,  
					 const uint8_t* parent, const uint8_t* p,
					 char* cummulativeString, int curStrOffset);
    void          gatherNodeStarts(const uint8_t* const start, const uint8_t* const end,  
				   const uint8_t* parent, const uint8_t* p,
				   std::vector<uint32_t>& nodeStarts);
    const char*         rebaseTypeName(uint8_t type);
    const char*         bindTypeName(uint8_t type);
    pint_t          segStartAddress(uint8_t segIndex);
    const char*         segmentName(uint8_t segIndex);
    const char*         sectionName(uint8_t segIndex, pint_t address);
    const char*         getSegAndSectName(uint8_t segIndex, pint_t address);
    const char*         ordinalName(int libraryOrdinal);
    const char*         classicOrdinalName(int libraryOrdinal);
    pint_t*          mappedAddressForVMAddress(pint_t vmaddress);
    const char*         symbolNameForAddress(uint64_t);

  
    const char*         fPath;
    const macho_header<P>*      fHeader;
    uint64_t         fLength;
    const char*         fStrings;
    const char*         fStringsEnd;
    const macho_nlist<P>*      fSymbols;
    uint32_t         fSymbolCount;
    const macho_dyld_info_command<P>*   fInfo;
    const macho_linkedit_data_command<P>*  fSharedRegionInfo;
    const macho_linkedit_data_command<P>*  fFunctionStartsInfo;
    uint64_t         fBaseAddress;
    const macho_dysymtab_command<P>*   fDynamicSymbolTable;
    const macho_segment_command<P>*    fFirstSegment;
    const macho_segment_command<P>*    fFirstWritableSegment;
    bool          fWriteableSegmentWithAddrOver4G;
    std::vector<const macho_segment_command<P>*>fSegments;
    std::vector<const char*>     fDylibs;
    std::vector<const macho_dylib_command<P>*> fDylibLoadCommands;

    size_t _mmap_len; // save the exact len that mmap was called with here, for correct munmap-ing.

    //ExportFuncHashTable _xftab;

    symvec  _exportsvec;
    symvec  _importsvec;
    symvec  _export_and_function_svec;
    symvec  _imports_minus_exports_svec;

};




template <>
bool DyldInfoPrinter<ppc>::validFile(const uint8_t* fileContent)
{ 
    const macho_header<P>* header = (const macho_header<P>*)fileContent;
    if ( header->magic() != MH_MAGIC )
	return false;
    if ( header->cputype() != CPU_TYPE_POWERPC )
	return false;
    switch (header->filetype()) {
    case MH_EXECUTE:
    case MH_DYLIB:
    case MH_BUNDLE:
    case MH_DYLINKER:
	return true;
    }
    return false;
}

template <>
bool DyldInfoPrinter<ppc64>::validFile(const uint8_t* fileContent)
{ 
    const macho_header<P>* header = (const macho_header<P>*)fileContent;
    if ( header->magic() != MH_MAGIC_64 )
	return false;
    if ( header->cputype() != CPU_TYPE_POWERPC64 )
	return false;
    switch (header->filetype()) {
    case MH_EXECUTE:
    case MH_DYLIB:
    case MH_BUNDLE:
    case MH_DYLINKER:
	return true;
    }
    return false;
}

template <>
bool DyldInfoPrinter<x86>::validFile(const uint8_t* fileContent)
{ 
    const macho_header<P>* header = (const macho_header<P>*)fileContent;
    if ( header->magic() != MH_MAGIC )
	return false;
    if ( header->cputype() != CPU_TYPE_I386 )
	return false;
    switch (header->filetype()) {
    case MH_EXECUTE:
    case MH_DYLIB:
    case MH_BUNDLE:
    case MH_DYLINKER:
	return true;
    }
    return false;
}

template <>
bool DyldInfoPrinter<x86_64>::validFile(const uint8_t* fileContent)
{ 
    const macho_header<P>* header = (const macho_header<P>*)fileContent;
    if ( header->magic() != MH_MAGIC_64 )
	return false;
    if ( header->cputype() != CPU_TYPE_X86_64 )
	return false;
    switch (header->filetype()) {
    case MH_EXECUTE:
    case MH_DYLIB:
    case MH_BUNDLE:
    case MH_DYLINKER:
	return true;
    }
    return false;
}

template <>
bool DyldInfoPrinter<arm>::validFile(const uint8_t* fileContent)
{ 
    const macho_header<P>* header = (const macho_header<P>*)fileContent;
    if ( header->magic() != MH_MAGIC )
	return false;
    if ( header->cputype() != CPU_TYPE_ARM )
	return false;
    switch (header->filetype()) {
    case MH_EXECUTE:
    case MH_DYLIB:
    case MH_BUNDLE:
    case MH_DYLINKER:
	return true;
    }
    return false;
}

template <typename A>
DyldInfoPrinter<A>::DyldInfoPrinter(const uint8_t* fileContent, uint32_t fileLength, const char* path, bool printArch, size_t mmap_len)
    : fHeader(NULL), fLength(fileLength), 
      fStrings(NULL), fStringsEnd(NULL), fSymbols(NULL), fSymbolCount(0), fInfo(NULL), 
      fSharedRegionInfo(NULL), fFunctionStartsInfo(NULL), 
      fBaseAddress(0), fDynamicSymbolTable(NULL), fFirstSegment(NULL), fFirstWritableSegment(NULL),
      fWriteableSegmentWithAddrOver4G(false),
      _mmap_len(mmap_len)
{
    // sanity check
    if ( ! validFile(fileContent) )
	throw "not a mach-o file that can be checked";

    fPath = strdup(path);
    fHeader = (const macho_header<P>*)fileContent;
 
    // get LC_DYLD_INFO
    const uint8_t* const endOfFile = (uint8_t*)fHeader + fLength;
    const uint8_t* const endOfLoadCommands = (uint8_t*)fHeader + sizeof(macho_header<P>) + fHeader->sizeofcmds();
    const uint32_t cmd_count = fHeader->ncmds();
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)fHeader + sizeof(macho_header<P>));
    const macho_load_command<P>* cmd = cmds;
    for (uint32_t i = 0; i < cmd_count; ++i) {
	const uint8_t* endOfCmd = ((uint8_t*)cmd)+cmd->cmdsize();
	if ( endOfCmd > endOfLoadCommands )
	    throwf("load command #%d extends beyond the end of the load commands", i);
	if ( endOfCmd > endOfFile )
	    throwf("load command #%d extends beyond the end of the file", i);
	switch ( cmd->cmd() ) {
	case LC_DYLD_INFO:
	case LC_DYLD_INFO_ONLY:
	    fInfo = (macho_dyld_info_command<P>*)cmd;
	    break;
	case macho_segment_command<P>::CMD:
	{
	    const macho_segment_command<P>* segCmd = (const macho_segment_command<P>*)cmd;
	    fSegments.push_back(segCmd);
	    if ( (segCmd->fileoff() == 0) && (segCmd->filesize() != 0) )
		fBaseAddress = segCmd->vmaddr();
	    if ( fFirstSegment == NULL )
		fFirstSegment = segCmd;
	    if ( (segCmd->initprot() & VM_PROT_WRITE) != 0 ) {
		if ( fFirstWritableSegment == NULL )
		    fFirstWritableSegment = segCmd;
		if ( segCmd->vmaddr() > 0x100000000ULL )
		    fWriteableSegmentWithAddrOver4G = true;
	    }
	}
	break;
	case LC_LOAD_DYLIB:
	case LC_LOAD_WEAK_DYLIB:
	case LC_REEXPORT_DYLIB:
	case LC_LOAD_UPWARD_DYLIB:
	case LC_LAZY_LOAD_DYLIB:
	{
	    const macho_dylib_command<P>* dylib  = (macho_dylib_command<P>*)cmd;
	    fDylibLoadCommands.push_back(dylib);
	    const char* lastSlash = strrchr(dylib->name(), '/');
	    const char* leafName = (lastSlash != NULL) ? lastSlash+1 : dylib->name();
	    const char* firstDot = strchr(leafName, '.');
	    if ( firstDot != NULL ) {
		char* t = strdup(leafName);
		t[firstDot-leafName] = '\0';
		fDylibs.push_back(t);
	    }
	    else {
		fDylibs.push_back(leafName);
	    }
	}
	break;
	case LC_DYSYMTAB:
	    fDynamicSymbolTable = (macho_dysymtab_command<P>*)cmd;
	    break;
	case LC_SYMTAB:
	{
	    const macho_symtab_command<P>* symtab = (macho_symtab_command<P>*)cmd;
	    fSymbolCount = symtab->nsyms();
	    fSymbols = (const macho_nlist<P>*)((char*)fHeader + symtab->symoff());
	    fStrings = (char*)fHeader + symtab->stroff();
	    fStringsEnd = fStrings + symtab->strsize();
	}
	break;
	case LC_SEGMENT_SPLIT_INFO:
	    fSharedRegionInfo = (macho_linkedit_data_command<P>*)cmd;
	    break;
	case LC_FUNCTION_STARTS:
	    fFunctionStartsInfo = (macho_linkedit_data_command<P>*)cmd;
	    break;
	}
	cmd = (const macho_load_command<P>*)endOfCmd;
    }


    _exportsvec.clear();
    printExportInfo();
    // printf("exported symbols:\n");
    //show_svec(_exportsvec);

    // printf("function starts in our hash table:\n");
    //show_xf(_xftab);

    //delete_if_not_expfunc(_xftab);

    //printf("after intersection:\n");
    //show_xf(_xftab);

    /* 
       _export_and_function_svec.clear();
       {
       xfit b = _xftab.begin();
       xfit e = _xftab.end();
       for(; b !=e; ++b) {
       _export_and_function_svec.push_back(b->first);
       }
       }
    */

    //printf("\n\n both functions and exported:\n");
    //show_svec(_export_and_function_svec);


    _importsvec.clear();
    if ( fInfo != NULL )
	printLazyBindingInfo();
    else
	printClassicLazyBindingInfo();

    //printf("\n\n lazy bindings (i.e. imports):\n");
    //show_svec(_importsvec);

    /*
    // don't worry about these for now, but we may have to add them later
    printf("\n\n bindings (also imports?):\n");
    if ( fInfo != NULL )
    printBindingInfo();
    else
    printClassicBindingInfo();
    */

    // subtract any exports from the imports list
    StringSet im_minus_ex;

    {  
	svit b = _importsvec.begin();
	svit e = _importsvec.end();
	for(; b !=e; ++b) {
	    im_minus_ex.insert(*b);
	}
   
	b = _exportsvec.begin();
	e = _exportsvec.end();
	for(; b !=e; ++b) {
	    im_minus_ex.erase(*b);
	}
    }

    _imports_minus_exports_svec.clear();
    StringSetIt b = im_minus_ex.begin();
    StringSetIt e = im_minus_ex.end();
    {
	for(; b !=e; ++b) {
	    _imports_minus_exports_svec.push_back(*b);
	}
    }

    // printf("\n\n imports without exports:\n");
    // show_svec(_imports_minus_exports_svec);


/* 
   if ( printArch ) {
   switch ( fHeader->cputype() ) {
   case CPU_TYPE_I386:
   printf("for arch i386:\n");
   break;
   case CPU_TYPE_X86_64:
   printf("for arch x86_64:\n");
   break;
   case CPU_TYPE_POWERPC:   
   printf("for arch ppc:\n");
   break;
   case CPU_TYPE_ARM:
   for (const ARMSubType* t=ARMSubTypes; t->subTypeName != NULL; ++t) {
   if ( (cpu_subtype_t)fHeader->cpusubtype() == t->subType) {
   printf("for arch %s:\n", t->subTypeName);
   break;
   }
   }
   }
   }
 
 
   if ( printRebase ) {
   if ( fInfo != NULL )
   printRebaseInfo();
   else
   printRelocRebaseInfo();
   }
   if ( printBind ) {
   if ( fInfo != NULL )
   printBindingInfo();
   else
   printClassicBindingInfo();
   }
   if ( printWeakBind ) 
   printWeakBindingInfo();
   if ( printLazyBind ) {
   if ( fInfo != NULL )
   printLazyBindingInfo();
   else
   printClassicLazyBindingInfo();
   }
   if ( printExport ) {
   if ( fInfo != NULL )
   printExportInfo();
   else
   printSymbolTableExportInfo();
   }
   if ( printOpcodes ) {
   printRebaseInfoOpcodes();
   printBindingInfoOpcodes(false);
   printBindingInfoOpcodes(true);
   printLazyBindingOpcodes();
   }
   if ( printExportGraph )
   printExportInfoGraph();
   if ( printExportNodes ) 
   printExportInfoNodes();
   if ( printSharedRegion )
   printSharedRegionInfo();
   if ( printFunctionStarts ) 
   printFunctionStartsInfo();
   if ( printDylibs )
   printDylibsInfo();
*/

}

static uint64_t read_uleb128(const uint8_t*& p, const uint8_t* end)
{
    uint64_t result = 0;
    int   bit = 0;
    do {
	if (p == end)
	    throwf("malformed uleb128");

	uint64_t slice = *p & 0x7f;

	if (bit >= 64 || slice << bit >> bit != slice)
	    throwf("uleb128 too big");
	else {
	    result |= (slice << bit);
	    bit += 7;
	}
    } 
    while (*p++ & 0x80);
    return result;
}

static int64_t read_sleb128(const uint8_t*& p, const uint8_t* end)
{
    int64_t result = 0;
    int bit = 0;
    uint8_t byte;
    do {
	if (p == end)
	    throwf("malformed sleb128");
	byte = *p++;
	result |= ((byte & 0x7f) << bit);
	bit += 7;
    } while (byte & 0x80);
    // sign extend negative numbers
    if ( (byte & 0x40) != 0 )
	result |= (-1LL) << bit;
    return result;
}


template <typename A>
const char* DyldInfoPrinter<A>::rebaseTypeName(uint8_t type)
{
    switch (type ){
    case REBASE_TYPE_POINTER:
	return "pointer";
    case REBASE_TYPE_TEXT_ABSOLUTE32:
	return "text abs32";
    case REBASE_TYPE_TEXT_PCREL32:
	return "text rel32";
    }
    return "!!unknown!!";
}


template <typename A>
const char* DyldInfoPrinter<A>::bindTypeName(uint8_t type)
{
    switch (type ){
    case BIND_TYPE_POINTER:
	return "pointer";
    case BIND_TYPE_TEXT_ABSOLUTE32:
	return "text abs32";
    case BIND_TYPE_TEXT_PCREL32:
	return "text rel32";
    }
    return "!!unknown!!";
}


template <typename A>
typename A::P::uint_t DyldInfoPrinter<A>::segStartAddress(uint8_t segIndex)
{
    if ( segIndex > fSegments.size() )
	throw "segment index out of range";
    return fSegments[segIndex]->vmaddr();
}

template <typename A>
const char* DyldInfoPrinter<A>::segmentName(uint8_t segIndex)
{
    if ( segIndex > fSegments.size() )
	throw "segment index out of range";
    return fSegments[segIndex]->segname();
}

template <typename A>
const char* DyldInfoPrinter<A>::sectionName(uint8_t segIndex, pint_t address)
{
    if ( segIndex > fSegments.size() )
	throw "segment index out of range";
    const macho_segment_command<P>* segCmd = fSegments[segIndex];
    macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
    macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
    for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
	if ( (sect->addr() <= address) && (address < (sect->addr()+sect->size())) ) {
	    if ( strlen(sect->sectname()) > 15 ) {
		static char temp[18];
		strlcpy(temp, sect->sectname(), 17);
		return temp;
	    }
	    else {
		return sect->sectname();
	    }
	}
    }
    return "??";
}

template <typename A>
const char* DyldInfoPrinter<A>::getSegAndSectName(uint8_t segIndex, pint_t address)
{
    static char buffer[64];
    strcpy(buffer, segmentName(segIndex));
    strcat(buffer, "/");
    const macho_segment_command<P>* segCmd = fSegments[segIndex];
    macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
    macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
    for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
	if ( (sect->addr() <= address) && (address < (sect->addr()+sect->size())) ) {
	    // section name may not be zero terminated
	    char* end = &buffer[strlen(buffer)];
	    strlcpy(end, sect->sectname(), 16);
	    return buffer;    
	}
    }
    return "??";
}

template <typename A>
uint8_t DyldInfoPrinter<A>::segmentIndexForAddress(pint_t address)
{
    for(unsigned int i=0; i < fSegments.size(); ++i) {
	if ( (fSegments[i]->vmaddr() <= address) && (address < (fSegments[i]->vmaddr()+fSegments[i]->vmsize())) ) {
	    return i;
	}
    }
    throwf("address 0x%llX is not in any segment", (uint64_t)address);
}

template <typename A>
typename A::P::uint_t* DyldInfoPrinter<A>::mappedAddressForVMAddress(pint_t vmaddress)
{
    for(unsigned int i=0; i < fSegments.size(); ++i) {
	if ( (fSegments[i]->vmaddr() <= vmaddress) && (vmaddress < (fSegments[i]->vmaddr()+fSegments[i]->vmsize())) ) {
	    unsigned long offsetInMappedFile = fSegments[i]->fileoff()+vmaddress-fSegments[i]->vmaddr();
	    return (pint_t*)((uint8_t*)fHeader + offsetInMappedFile);
	}
    }
    throwf("address 0x%llX is not in any segment", (uint64_t)vmaddress);
}

template <typename A>
const char* DyldInfoPrinter<A>::ordinalName(int libraryOrdinal)
{
    switch ( libraryOrdinal) {
    case BIND_SPECIAL_DYLIB_SELF:
	return "this-image";
    case BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE:
	return "main-executable";
    case BIND_SPECIAL_DYLIB_FLAT_LOOKUP:
	return "flat-namespace";
    }
    if ( libraryOrdinal < BIND_SPECIAL_DYLIB_FLAT_LOOKUP )
	throw "unknown special ordinal";
    if ( libraryOrdinal > (int)fDylibs.size() )
	throw "libraryOrdinal out of range";
    return fDylibs[libraryOrdinal-1];
}

template <typename A>
const char* DyldInfoPrinter<A>::classicOrdinalName(int libraryOrdinal)
{
    if ( (fHeader->flags() & MH_TWOLEVEL) ==  0 )
	return "flat-namespace";
    switch ( libraryOrdinal) {
    case SELF_LIBRARY_ORDINAL:
	return "this-image";
    case EXECUTABLE_ORDINAL:
	return "main-executable";
    case DYNAMIC_LOOKUP_ORDINAL:
	return "flat-namespace";
    }
    if ( libraryOrdinal > (int)fDylibs.size() )
	throw "libraryOrdinal out of range";
    return fDylibs[libraryOrdinal-1];
}

template <typename A>
void DyldInfoPrinter<A>::printRebaseInfo()
{
    if ( (fInfo == NULL) || (fInfo->rebase_off() == 0) ) {
	printf("no compressed rebase info\n");
    }
    else {
	printf("rebase information (from compressed dyld info):\n");
	printf("segment section          address     type\n");

	const uint8_t* p = (uint8_t*)fHeader + fInfo->rebase_off();
	const uint8_t* end = &p[fInfo->rebase_size()];
  
	uint8_t type = 0;
	uint64_t segOffset = 0;
	uint32_t count;
	uint32_t skip;
	int segIndex;
	pint_t segStartAddr = 0;
	const char* segName = "??";
	const char* typeName = "??";
	bool done = false;
	while ( !done && (p < end) ) {
	    uint8_t immediate = *p & REBASE_IMMEDIATE_MASK;
	    uint8_t opcode = *p & REBASE_OPCODE_MASK;
	    ++p;
	    switch (opcode) {
	    case REBASE_OPCODE_DONE:
		done = true;
		break;
	    case REBASE_OPCODE_SET_TYPE_IMM:
		type = immediate;
		typeName = rebaseTypeName(type);
		break;
	    case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
		segIndex = immediate;
		segStartAddr = segStartAddress(segIndex);
		segName = segmentName(segIndex);
		segOffset = read_uleb128(p, end);
		break;
	    case REBASE_OPCODE_ADD_ADDR_ULEB:
		segOffset += read_uleb128(p, end);
		break;
	    case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
		segOffset += immediate*sizeof(pint_t);
		break;
	    case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
		for (int i=0; i < immediate; ++i) {
		    printf("%-7s %-16s 0x%08llX  %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName);
		    segOffset += sizeof(pint_t);
		}
		break;
	    case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
		count = read_uleb128(p, end);
		for (uint32_t i=0; i < count; ++i) {
		    printf("%-7s %-16s 0x%08llX  %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName);
		    segOffset += sizeof(pint_t);
		}
		break;
	    case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
		printf("%-7s %-16s 0x%08llX  %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName);
		segOffset += read_uleb128(p, end) + sizeof(pint_t);
		break;
	    case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
		count = read_uleb128(p, end);
		skip = read_uleb128(p, end);
		for (uint32_t i=0; i < count; ++i) {
		    printf("%-7s %-16s 0x%08llX  %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName);
		    segOffset += skip + sizeof(pint_t);
		}
		break;
	    default:
		throwf("bad rebase opcode %d", *p);
	    }
	} 
    }

}



template <typename A>
void DyldInfoPrinter<A>::printRebaseInfoOpcodes()
{
    if ( (fInfo == NULL) || (fInfo->rebase_off() == 0) ) {
	printf("no compressed rebase info\n");
    }
    else {
	printf("rebase opcodes:\n");
	const uint8_t* p = (uint8_t*)fHeader + fInfo->rebase_off();
	const uint8_t* end = &p[fInfo->rebase_size()];
  
	uint8_t type = 0;
	uint64_t address = fBaseAddress;
	uint32_t count;
	uint32_t skip;
	unsigned int segmentIndex;
	bool done = false;
	while ( !done && (p < end) ) {
	    uint8_t immediate = *p & REBASE_IMMEDIATE_MASK;
	    uint8_t opcode = *p & REBASE_OPCODE_MASK;
	    ++p;
	    switch (opcode) {
	    case REBASE_OPCODE_DONE:
		done = true;
		printf("REBASE_OPCODE_DONE()\n");
		break;
	    case REBASE_OPCODE_SET_TYPE_IMM:
		type = immediate;
		printf("REBASE_OPCODE_SET_TYPE_IMM(%d)\n", type);
		break;
	    case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
		segmentIndex = immediate;
		address = read_uleb128(p, end);
		printf("REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(%d, 0x%08llX)\n", segmentIndex, address);
		break;
	    case REBASE_OPCODE_ADD_ADDR_ULEB:
		address = read_uleb128(p, end);
		printf("REBASE_OPCODE_ADD_ADDR_ULEB(0x%0llX)\n", address);
		break;
	    case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
		address = immediate*sizeof(pint_t);
		printf("REBASE_OPCODE_ADD_ADDR_IMM_SCALED(0x%0llX)\n", address);
		break;
	    case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
		printf("REBASE_OPCODE_DO_REBASE_IMM_TIMES(%d)\n", immediate);
		break;
	    case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
		count = read_uleb128(p, end);
		printf("REBASE_OPCODE_DO_REBASE_ULEB_TIMES(%d)\n", count);
		break;
	    case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
		skip = read_uleb128(p, end) + sizeof(pint_t);
		printf("REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB(%d)\n", skip);
		break;
	    case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
		count = read_uleb128(p, end);
		skip = read_uleb128(p, end);
		printf("REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB(%d, %d)\n", count, skip);
		break;
	    default:
		throwf("bad rebase opcode %d", *p);
	    }
	} 
    }

}






template <typename A>
void DyldInfoPrinter<A>::printBindingInfoOpcodes(bool weakbinding)
{
    if ( fInfo == NULL ) {
	printf("no compressed binding info\n");
    }
    else if ( !weakbinding && (fInfo->bind_off() == 0) ) {
	printf("no compressed binding info\n");
    }
    else if ( weakbinding && (fInfo->weak_bind_off() == 0) ) {
	printf("no compressed weak binding info\n");
    }
    else {
	const uint8_t* start;
	const uint8_t* end;
	if ( weakbinding ) {
	    printf("weak binding opcodes:\n");
	    start = (uint8_t*)fHeader + fInfo->weak_bind_off();
	    end = &start[fInfo->weak_bind_size()];
	}
	else {
	    printf("binding opcodes:\n");
	    start = (uint8_t*)fHeader + fInfo->bind_off();
	    end = &start[fInfo->bind_size()];
	}
	const uint8_t* p = start;
	uint8_t type = 0;
	uint8_t flags;
	uint64_t address = fBaseAddress;
	const char* symbolName = NULL;
	int libraryOrdinal = 0;
	int64_t addend = 0;
	uint32_t segmentIndex = 0;
	uint32_t count;
	uint32_t skip;
	bool done = false;
	while ( !done && (p < end) ) {
	    uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
	    uint8_t opcode = *p & BIND_OPCODE_MASK;
	    uint32_t opcodeOffset = p-start;
	    ++p;
	    switch (opcode) {
	    case BIND_OPCODE_DONE:
		done = true;
		printf("0x%04X BIND_OPCODE_DONE\n", opcodeOffset);
		break;
	    case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
		libraryOrdinal = immediate;
		printf("0x%04X BIND_OPCODE_SET_DYLIB_ORDINAL_IMM(%d)\n", opcodeOffset, libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
		libraryOrdinal = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB(%d)\n", opcodeOffset, libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
		// the special ordinals are negative numbers
		if ( immediate == 0 )
		    libraryOrdinal = 0;
		else {
		    int8_t signExtended = BIND_OPCODE_MASK | immediate;
		    libraryOrdinal = signExtended;
		}
		printf("0x%04X BIND_OPCODE_SET_DYLIB_SPECIAL_IMM(%d)\n", opcodeOffset, libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
		flags = immediate;
		symbolName = (char*)p;
		while (*p != '\0')
		    ++p;
		++p;
		printf("0x%04X BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM(0x%02X, %s)\n", opcodeOffset, flags, symbolName);
		break;
	    case BIND_OPCODE_SET_TYPE_IMM:
		type = immediate;
		printf("0x%04X BIND_OPCODE_SET_TYPE_IMM(%d)\n", opcodeOffset, type);
		break;
	    case BIND_OPCODE_SET_ADDEND_SLEB:
		addend = read_sleb128(p, end);
		printf("0x%04X BIND_OPCODE_SET_ADDEND_SLEB(%lld)\n", opcodeOffset, addend);
		break;
	    case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
		segmentIndex = immediate;
		address = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(0x%02X, 0x%08llX)\n", opcodeOffset, segmentIndex, address);
		break;
	    case BIND_OPCODE_ADD_ADDR_ULEB:
		skip = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_ADD_ADDR_ULEB(0x%08X)\n", opcodeOffset, skip);
		break;
	    case BIND_OPCODE_DO_BIND:
		printf("0x%04X BIND_OPCODE_DO_BIND()\n", opcodeOffset);
		break;
	    case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
		skip = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB(0x%08X)\n", opcodeOffset, skip);
		break;
	    case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
		skip = immediate*sizeof(pint_t) + sizeof(pint_t);
		printf("0x%04X BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED(0x%08X)\n", opcodeOffset, skip);
		break;
	    case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
		count = read_uleb128(p, end);
		skip = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB(%d, 0x%08X)\n", opcodeOffset, count, skip);
		break;
	    default:
		throwf("unknown bind opcode %d", *p);
	    }
	} 
    }

}



template <typename A>
void DyldInfoPrinter<A>::printBindingInfo()
{
    if ( (fInfo == NULL) || (fInfo->bind_off() == 0) ) {
	printf("no compressed binding info\n");
    }
    else {
	printf("bind information:\n");
	printf("segment section          address        type    addend dylib            symbol\n");
	const uint8_t* p = (uint8_t*)fHeader + fInfo->bind_off();
	const uint8_t* end = &p[fInfo->bind_size()];
  
	uint8_t type = 0;
	uint8_t segIndex = 0;
	uint64_t segOffset = 0;
	const char* symbolName = NULL;
	const char* fromDylib = "??";
	int libraryOrdinal = 0;
	int64_t addend = 0;
	uint32_t count;
	uint32_t skip;
	pint_t segStartAddr = 0;
	const char* segName = "??";
	const char* typeName = "??";
	const char* weak_import = "";
	bool done = false;
	while ( !done && (p < end) ) {
	    uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
	    uint8_t opcode = *p & BIND_OPCODE_MASK;
	    ++p;
	    switch (opcode) {
	    case BIND_OPCODE_DONE:
		done = true;
		break;
	    case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
		libraryOrdinal = immediate;
		fromDylib = ordinalName(libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
		libraryOrdinal = read_uleb128(p, end);
		fromDylib = ordinalName(libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
		// the special ordinals are negative numbers
		if ( immediate == 0 )
		    libraryOrdinal = 0;
		else {
		    int8_t signExtended = BIND_OPCODE_MASK | immediate;
		    libraryOrdinal = signExtended;
		}
		fromDylib = ordinalName(libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
		symbolName = (char*)p;
		while (*p != '\0')
		    ++p;
		++p;
		if ( (immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 )
		    weak_import = " (weak import)";
		else
		    weak_import = "";
		break;
	    case BIND_OPCODE_SET_TYPE_IMM:
		type = immediate;
		typeName = bindTypeName(type);
		break;
	    case BIND_OPCODE_SET_ADDEND_SLEB:
		addend = read_sleb128(p, end);
		break;
	    case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
		segIndex = immediate;
		segStartAddr = segStartAddress(segIndex);
		segName = segmentName(segIndex);
		segOffset = read_uleb128(p, end);
		break;
	    case BIND_OPCODE_ADD_ADDR_ULEB:
		segOffset += read_uleb128(p, end);
		break;
	    case BIND_OPCODE_DO_BIND:
		printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import );
		segOffset += sizeof(pint_t);
		break;
	    case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
		printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import );
		segOffset += read_uleb128(p, end) + sizeof(pint_t);
		break;
	    case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
		printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import );
		segOffset += immediate*sizeof(pint_t) + sizeof(pint_t);
		break;
	    case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
		count = read_uleb128(p, end);
		skip = read_uleb128(p, end);
		for (uint32_t i=0; i < count; ++i) {
		    printf("%-7s %-16s 0x%08llX %10s  %5lld %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, fromDylib, symbolName, weak_import );
		    segOffset += skip + sizeof(pint_t);
		}
		break;
	    default:
		throwf("bad bind opcode %d", *p);
	    }
	} 
    }

}

template <typename A>
void DyldInfoPrinter<A>::printWeakBindingInfo()
{
    if ( (fInfo == NULL) || (fInfo->weak_bind_off() == 0) ) {
	printf("no weak binding\n");
    }
    else {
	printf("weak binding information:\n");
	printf("segment section          address       type     addend symbol\n");
	const uint8_t* p = (uint8_t*)fHeader + fInfo->weak_bind_off();
	const uint8_t* end = &p[fInfo->weak_bind_size()];
  
	uint8_t type = 0;
	uint8_t segIndex = 0;
	uint64_t segOffset = 0;
	const char* symbolName = NULL;
	int64_t addend = 0;
	uint32_t count;
	uint32_t skip;
	pint_t segStartAddr = 0;
	const char* segName = "??";
	const char* typeName = "??";
	bool done = false;
	while ( !done && (p < end) ) {
	    uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
	    uint8_t opcode = *p & BIND_OPCODE_MASK;
	    ++p;
	    switch (opcode) {
	    case BIND_OPCODE_DONE:
		done = true;
		break;
	    case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
		symbolName = (char*)p;
		while (*p != '\0')
		    ++p;
		++p;
		if ( (immediate & BIND_SYMBOL_FLAGS_NON_WEAK_DEFINITION) != 0 )
		    printf("                                       strong          %s\n", symbolName );
		break;
	    case BIND_OPCODE_SET_TYPE_IMM:
		type = immediate;
		typeName = bindTypeName(type);
		break;
	    case BIND_OPCODE_SET_ADDEND_SLEB:
		addend = read_sleb128(p, end);
		break;
	    case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
		segIndex = immediate;
		segStartAddr = segStartAddress(segIndex);
		segName = segmentName(segIndex);
		segOffset = read_uleb128(p, end);
		break;
	    case BIND_OPCODE_ADD_ADDR_ULEB:
		segOffset += read_uleb128(p, end);
		break;
	    case BIND_OPCODE_DO_BIND:
		printf("%-7s %-16s 0x%08llX %10s   %5lld %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, symbolName );
		segOffset += sizeof(pint_t);
		break;
	    case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
		printf("%-7s %-16s 0x%08llX %10s   %5lld %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, symbolName );
		segOffset += read_uleb128(p, end) + sizeof(pint_t);
		break;
	    case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
		printf("%-7s %-16s 0x%08llX %10s   %5lld %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, symbolName );
		segOffset += immediate*sizeof(pint_t) + sizeof(pint_t);
		break;
	    case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
		count = read_uleb128(p, end);
		skip = read_uleb128(p, end);
		for (uint32_t i=0; i < count; ++i) {
		    printf("%-7s %-16s 0x%08llX %10s   %5lld %s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, typeName, addend, symbolName );
		    segOffset += skip + sizeof(pint_t);
		}
		break;
	    default:
		throwf("unknown weak bind opcode %d", *p);
	    }
	} 
    }

}


template <typename A>
void DyldInfoPrinter<A>::printLazyBindingInfo()
{
    if ( fInfo == NULL ) {
	//printf("no compressed dyld info\n");
    }
    else if ( fInfo->lazy_bind_off() == 0 ) {
	//printf("no compressed lazy binding info\n");
    }
    else {
	//printf("lazy binding information (from lazy_bind part of dyld info):\n");
	//printf("segment section          address    index  dylib            symbol\n");
	const uint8_t* const start = (uint8_t*)fHeader + fInfo->lazy_bind_off();
	const uint8_t* const end = &start[fInfo->lazy_bind_size()];

	uint8_t type = BIND_TYPE_POINTER;
	uint8_t segIndex = 0;
	uint64_t segOffset = 0;
	const char* symbolName = NULL;
	const char* fromDylib = "??";
	int libraryOrdinal = 0;
	int64_t addend = 0;
	uint32_t lazy_offset = 0;
	pint_t segStartAddr = 0;
	const char* segName = "??";
	const char* typeName = "??";
	const char* weak_import = "";
	for (const uint8_t* p=start; p < end; ) {
	    uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
	    uint8_t opcode = *p & BIND_OPCODE_MASK;
	    ++p;
	    switch (opcode) {
	    case BIND_OPCODE_DONE:
		lazy_offset = p-start;
		break;
	    case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
		libraryOrdinal = immediate;
		fromDylib = ordinalName(libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
		libraryOrdinal = read_uleb128(p, end);
		fromDylib = ordinalName(libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
		// the special ordinals are negative numbers
		if ( immediate == 0 )
		    libraryOrdinal = 0;
		else {
		    int8_t signExtended = BIND_OPCODE_MASK | immediate;
		    libraryOrdinal = signExtended;
		}
		fromDylib = ordinalName(libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
		symbolName = (char*)p;
		while (*p != '\0')
		    ++p;
		++p;
		if ( (immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 )
		    weak_import = " (weak import)";
		else
		    weak_import = "";
		break;
	    case BIND_OPCODE_SET_TYPE_IMM:
		type = immediate;
		typeName = bindTypeName(type);
		break;
	    case BIND_OPCODE_SET_ADDEND_SLEB:
		addend = read_sleb128(p, end);
		break;
	    case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
		segIndex = immediate;
		segStartAddr = segStartAddress(segIndex);
		segName = segmentName(segIndex);
		segOffset = read_uleb128(p, end);
		break;
	    case BIND_OPCODE_ADD_ADDR_ULEB:
		segOffset += read_uleb128(p, end);
		break;
	    case BIND_OPCODE_DO_BIND:
		_importsvec.push_back(symbolName);
		//printf("%-7s %-16s 0x%08llX 0x%04X %-16s %s%s\n", segName, sectionName(segIndex, segStartAddr+segOffset), segStartAddr+segOffset, lazy_offset, fromDylib, symbolName, weak_import);
		segOffset += sizeof(pint_t);
		break;
	    default:
		throwf("bad lazy bind opcode %d", *p);
	    }
	}
    }

}

template <typename A>
void DyldInfoPrinter<A>::printLazyBindingOpcodes()
{
    if ( fInfo == NULL ) {
	printf("no compressed dyld info\n");
    }
    else if ( fInfo->lazy_bind_off() == 0 ) {
	printf("no compressed lazy binding info\n");
    }
    else {
	printf("lazy binding opcodes:\n");
	const uint8_t* const start = (uint8_t*)fHeader + fInfo->lazy_bind_off();
	const uint8_t* const end = &start[fInfo->lazy_bind_size()];
	uint8_t type = BIND_TYPE_POINTER;
	uint8_t flags;
	uint64_t address = fBaseAddress;
	const char* symbolName = NULL;
	int libraryOrdinal = 0;
	int64_t addend = 0;
	uint32_t segmentIndex = 0;
	uint32_t count;
	uint32_t skip;
	for (const uint8_t* p = start; p < end; ) {
	    uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
	    uint8_t opcode = *p & BIND_OPCODE_MASK;
	    uint32_t opcodeOffset = p-start;
	    ++p;
	    switch (opcode) {
	    case BIND_OPCODE_DONE:
		printf("0x%04X BIND_OPCODE_DONE\n", opcodeOffset);
		break;
	    case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
		libraryOrdinal = immediate;
		printf("0x%04X BIND_OPCODE_SET_DYLIB_ORDINAL_IMM(%d)\n", opcodeOffset, libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
		libraryOrdinal = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB(%d)\n", opcodeOffset, libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
		// the special ordinals are negative numbers
		if ( immediate == 0 )
		    libraryOrdinal = 0;
		else {
		    int8_t signExtended = BIND_OPCODE_MASK | immediate;
		    libraryOrdinal = signExtended;
		}
		printf("0x%04X BIND_OPCODE_SET_DYLIB_SPECIAL_IMM(%d)\n", opcodeOffset, libraryOrdinal);
		break;
	    case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
		flags = immediate;
		symbolName = (char*)p;
		while (*p != '\0')
		    ++p;
		++p;
		printf("0x%04X BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM(0x%02X, %s)\n", opcodeOffset, flags, symbolName);
		break;
	    case BIND_OPCODE_SET_TYPE_IMM:
		type = immediate;
		printf("0x%04X BIND_OPCODE_SET_TYPE_IMM(%d)\n", opcodeOffset, type);
		break;
	    case BIND_OPCODE_SET_ADDEND_SLEB:
		addend = read_sleb128(p, end);
		printf("0x%04X BIND_OPCODE_SET_ADDEND_SLEB(%lld)\n", opcodeOffset, addend);
		break;
	    case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
		segmentIndex = immediate;
		address = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB(0x%02X, 0x%08llX)\n", opcodeOffset, segmentIndex, address);
		break;
	    case BIND_OPCODE_ADD_ADDR_ULEB:
		skip = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_ADD_ADDR_ULEB(0x%08X)\n", opcodeOffset, skip);
		break;
	    case BIND_OPCODE_DO_BIND:
		printf("0x%04X BIND_OPCODE_DO_BIND()\n", opcodeOffset);
		break;
	    case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
		skip = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB(0x%08X)\n", opcodeOffset, skip);
		break;
	    case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
		skip = immediate*sizeof(pint_t) + sizeof(pint_t);
		printf("0x%04X BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED(0x%08X)\n", opcodeOffset, skip);
		break;
	    case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
		count = read_uleb128(p, end);
		skip = read_uleb128(p, end);
		printf("0x%04X BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB(%d, 0x%08X)\n", opcodeOffset, count, skip);
		break;
	    default:
		throwf("unknown bind opcode %d", *p);
	    }
	} 
    }

}

struct SortExportsByAddress
{
    bool operator()(const mach_o::trie::Entry& left, const mach_o::trie::Entry& right)
    {
        return ( left.address < right.address );
    }
};

template <typename A>
void DyldInfoPrinter<A>::printExportInfo()
{
    if ( (fInfo == NULL) || (fInfo->export_off() == 0) ) {
	//printf("no compressed export info\n");
    }
    else {
	//printf("export information (from trie):\n");
	const uint8_t* start = (uint8_t*)fHeader + fInfo->export_off();
	const uint8_t* end = &start[fInfo->export_size()];
	std::vector<mach_o::trie::Entry> list;
	parseTrie(start, end, list);
	//std::sort(list.begin(), list.end(), SortExportsByAddress());
	for (std::vector<mach_o::trie::Entry>::iterator it=list.begin(); it != list.end(); ++it) {
	    if ( it->flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
		if ( it->importName[0] == '\0' )
		    ;//fprintf(stdout, "[re-export] %s from dylib=%llu\n", it->name, it->other);
		else
		    ;//fprintf(stdout, "[re-export] %s from dylib=%llu named=%s\n", it->name, it->other, it->importName);
	    }
	    else {
		const char* flags = "";
		if ( it->flags & EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION )
		    flags = "[weak_def] ";
		else if ( (it->flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL )
		    flags = "[per-thread] ";
		if ( it->flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER ) {
		    flags = "[resolver] ";
		    //fprintf(stdout, "0x%08llX  %s%s (resolver=0x%08llX)\n", fBaseAddress+it->address, flags, it->name, it->other);
		}
		else {
		    //fprintf(stdout, "0x%08llX  %s%s\n", fBaseAddress+it->address, flags, it->name);
		    _exportsvec.push_back(it->name);
		}
	    }
	}
    }
}


template <typename A>
void DyldInfoPrinter<A>::processExportGraphNode(const uint8_t* const start, const uint8_t* const end,  
						const uint8_t* parent, const uint8_t* p,
						char* cummulativeString, int curStrOffset) 
{
    const uint8_t* const me = p;
    const uint8_t terminalSize = read_uleb128(p, end);
    const uint8_t* children = p + terminalSize;
    if ( terminalSize != 0 ) {
	uint32_t flags = read_uleb128(p, end);
	if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
	    uint64_t ordinal = read_uleb128(p, end);
	    const char* importName = (const char*)p;
	    while (*p != '\0')
		++p;
	    ++p;
	    if ( *importName == '\0' ) 
		printf("\tnode%03ld [ label=%s,re-export from dylib=%llu ];\n", (long)(me-start), cummulativeString, ordinal);
	    else
		printf("\tnode%03ld [ label=%s,re-export %s from dylib=%llu ];\n", (long)(me-start), cummulativeString, importName, ordinal);
	}
	else {
	    uint64_t address = read_uleb128(p, end);
	    printf("\tnode%03ld [ label=%s,addr0x%08llX ];\n", (long)(me-start), cummulativeString, address);
	    if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
		read_uleb128(p, end);
	}
    }
    else {
	printf("\tnode%03ld;\n", (long)(me-start));
    }
    const uint8_t childrenCount = *children++;
    const uint8_t* s = children;
    for (uint8_t i=0; i < childrenCount; ++i) {
	const char* edgeName = (char*)s;
	int edgeStrLen = 0;
	while (*s != '\0') {
	    cummulativeString[curStrOffset+edgeStrLen] = *s++;
	    ++edgeStrLen;
	}
	cummulativeString[curStrOffset+edgeStrLen] = *s++;
	uint32_t childNodeOffet = read_uleb128(s, end);
	printf("\tnode%03ld -> node%03d [ label=%s ] ;\n", (long)(me-start), childNodeOffet, edgeName);
	processExportGraphNode(start, end, start, start+childNodeOffet, cummulativeString, curStrOffset+edgeStrLen); 
    }
}

template <typename A>
void DyldInfoPrinter<A>::printExportInfoGraph()
{
    if ( (fInfo == NULL) || (fInfo->export_off() == 0) ) {
	printf("no compressed export info\n");
    }
    else {
	const uint8_t* p = (uint8_t*)fHeader + fInfo->export_off();
	const uint8_t* end = &p[fInfo->export_size()];
	char cummulativeString[2000];
	printf("digraph {\n");
	processExportGraphNode(p, end, p, p, cummulativeString, 0);
	printf("}\n");
    }
}

template <typename A>
void DyldInfoPrinter<A>::gatherNodeStarts(const uint8_t* const start, const uint8_t* const end,  
					  const uint8_t* parent, const uint8_t* p,
					  std::vector<uint32_t>& nodeStarts) 
{
    nodeStarts.push_back(p-start);
    const uint8_t terminalSize = read_uleb128(p, end);
    const uint8_t* children = p + terminalSize;
 
    const uint8_t childrenCount = *children++;
    const uint8_t* s = children;
    for (uint8_t i=0; i < childrenCount; ++i) {
	// skip over edge string
	while (*s != '\0')
	    ++s;
	++s;
	uint32_t childNodeOffet = read_uleb128(s, end);
	gatherNodeStarts(start, end, start, start+childNodeOffet, nodeStarts); 
    }
}


template <typename A>
void DyldInfoPrinter<A>::printExportInfoNodes()
{
    if ( (fInfo == NULL) || (fInfo->export_off() == 0) ) {
	printf("no compressed export info\n");
    }
    else {
	const uint8_t* start = (uint8_t*)fHeader + fInfo->export_off();
	const uint8_t* end = &start[fInfo->export_size()];
	std::vector<uint32_t> nodeStarts;
	gatherNodeStarts(start, end, start, start, nodeStarts);
	std::sort(nodeStarts.begin(), nodeStarts.end());
	for (std::vector<uint32_t>::const_iterator it=nodeStarts.begin(); it != nodeStarts.end(); ++it) {
	    printf("0x%04X: ", *it);
	    const uint8_t* p = start + *it;
	    uint64_t exportInfoSize = read_uleb128(p, end);
	    if ( exportInfoSize != 0 ) {
		// print terminal info
		uint64_t flags = read_uleb128(p, end);
		if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
		    uint64_t ordinal = read_uleb128(p, end);
		    const char* importName = (const char*)p;
		    while (*p != '\0')
			++p;
		    ++p;
		    if ( strlen(importName) == 0 )
			printf("[flags=REEXPORT ordinal=%llu] ", ordinal);
		    else
			printf("[flags=REEXPORT ordinal=%llu import=%s] ", ordinal, importName);
		}
		else if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER ) {
		    uint64_t stub = read_uleb128(p, end);
		    uint64_t resolver = read_uleb128(p, end);
		    printf("[flags=STUB_AND_RESOLVER stub=0x%06llX resolver=0x%06llX] ", stub, resolver);
		}
		else {
		    uint64_t address = read_uleb128(p, end);
		    if ( (flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_REGULAR )
			printf("[addr=0x%06llX] ", address);
		    else if ( (flags & EXPORT_SYMBOL_FLAGS_KIND_MASK) == EXPORT_SYMBOL_FLAGS_KIND_THREAD_LOCAL)
			printf("[flags=THREAD_LOCAL addr=0x%06llX] ", address);
		    else
			printf("[flags=0x%llX addr=0x%06llX] ", flags, address);
		}
	    }
	    // print child edges
	    const uint8_t childrenCount = *p++;
	    for (uint8_t i=0; i < childrenCount; ++i) {
		const char* edgeName = (const char*)p;
		while (*p != '\0')
		    ++p;
		++p;
		uint32_t childNodeOffet = read_uleb128(p, end);
		printf("%s->0x%04X", edgeName, childNodeOffet);
		if ( i < (childrenCount-1) )
		    printf(", ");
	    }
	    printf("\n");
	}
    }
}



template <typename A>
const uint8_t* DyldInfoPrinter<A>::printSharedRegionInfoForEachULEB128Address(const uint8_t* p, uint8_t kind)
{
    const char* kindStr =  "??";
    switch (kind) {
    case 1:
	kindStr = "32-bit pointer";
	break;
    case 2:
	kindStr = "64-bit pointer";
	break;
    case 3:
	kindStr = "ppc hi16";
	break;
    case 4:
	kindStr = "32-bit offset to IMPORT";
	break;
    case 5:
	kindStr = "thumb2 movw";
	break;
    case 6:
	kindStr = "ARM movw";
	break;
    case 0x10:
	kindStr = "thumb2 movt low high 4 bits=0";
	break;
    case 0x11:
	kindStr = "thumb2 movt low high 4 bits=1";
	break;
    case 0x12:
	kindStr = "thumb2 movt low high 4 bits=2";
	break;
    case 0x13:
	kindStr = "thumb2 movt low high 4 bits=3";
	break;
    case 0x14:
	kindStr = "thumb2 movt low high 4 bits=4";
	break;
    case 0x15:
	kindStr = "thumb2 movt low high 4 bits=5";
	break;
    case 0x16:
	kindStr = "thumb2 movt low high 4 bits=6";
	break;
    case 0x17:
	kindStr = "thumb2 movt low high 4 bits=7";
	break;
    case 0x18:
	kindStr = "thumb2 movt low high 4 bits=8";
	break;
    case 0x19:
	kindStr = "thumb2 movt low high 4 bits=9";
	break;
    case 0x1A:
	kindStr = "thumb2 movt low high 4 bits=0xA";
	break;
    case 0x1B:
	kindStr = "thumb2 movt low high 4 bits=0xB";
	break;
    case 0x1C:
	kindStr = "thumb2 movt low high 4 bits=0xC";
	break;
    case 0x1D:
	kindStr = "thumb2 movt low high 4 bits=0xD";
	break;
    case 0x1E:
	kindStr = "thumb2 movt low high 4 bits=0xE";
	break;
    case 0x1F:
	kindStr = "thumb2 movt low high 4 bits=0xF";
	break;
    }
    uint64_t address = 0;
    uint64_t delta = 0;
    uint32_t shift = 0;
    bool more = true;
    do {
	uint8_t byte = *p++;
	delta |= ((byte & 0x7F) << shift);
	shift += 7;
	if ( byte < 0x80 ) {
	    if ( delta != 0 ) {
		address += delta;
		printf("0x%0llX   %s\n", address+fBaseAddress, kindStr); 
		delta = 0;
		shift = 0;
	    }
	    else {
		more = false;
	    }
	}
    } while (more);
    return p;
}

template <typename A>
void DyldInfoPrinter<A>::printSharedRegionInfo()
{
    if ( (fSharedRegionInfo == NULL) || (fSharedRegionInfo->datasize() == 0) ) {
	printf("no shared region info\n");
    }
    else {
	const uint8_t* infoStart = (uint8_t*)fHeader + fSharedRegionInfo->dataoff();
	const uint8_t* infoEnd = &infoStart[fSharedRegionInfo->datasize()];
	for(const uint8_t* p = infoStart; (*p != 0) && (p < infoEnd);) {
	    uint8_t kind = *p++;
	    p = this->printSharedRegionInfoForEachULEB128Address(p, kind);
	}

    }
}

template <>
void DyldInfoPrinter<arm>::printFunctionStartLine(uint64_t addr, const char** funcname)
{
    if ( addr & 1 ) {
	//printf("0x%0llX [thumb] %s\n", (addr & -2), symbolNameForAddress(addr & -2));
	*funcname = symbolNameForAddress(addr & -2);
    } else {
	//printf("0x%0llX         %s\n", addr, symbolNameForAddress(addr)); 
	*funcname = symbolNameForAddress(addr);
    }
}

template <typename A>
void DyldInfoPrinter<A>::printFunctionStartLine(uint64_t addr, const char** funcname)
{
    *funcname = symbolNameForAddress(addr);
    //printf("0x%0llX   %s\n", addr, symbolNameForAddress(addr)); 
}


template <typename A>
void DyldInfoPrinter<A>::printFunctionStartsInfo()
{
    if ( (fFunctionStartsInfo == NULL) || (fFunctionStartsInfo->datasize() == 0) ) {
	//printf("no function starts info\n");
    }
    else {
	const uint8_t* infoStart = (uint8_t*)fHeader + fFunctionStartsInfo->dataoff();
	const uint8_t* infoEnd = &infoStart[fFunctionStartsInfo->datasize()];
	uint64_t address = fBaseAddress;
	for(const uint8_t* p = infoStart; (*p != 0) && (p < infoEnd); ) {
	    uint64_t delta = 0;
	    uint32_t shift = 0;
	    bool more = true;
	    do {
		uint8_t byte = *p++;
		delta |= ((byte & 0x7F) << shift);
		shift += 7;
		if ( byte < 0x80 ) {
		    address += delta;
		    printFunctionStartLine(address);
		    more = false;
		}
	    } while (more);
	}
    }
}

template <typename A>
void DyldInfoPrinter<A>::printDylibsInfo()
{
    printf("attributes     dependent dylibs\n");
    for(typename std::vector<const macho_dylib_command<P>*>::iterator it = fDylibLoadCommands.begin(); it != fDylibLoadCommands.end(); ++it) {
	const macho_dylib_command<P>* dylib  = *it;
	const char* attribute = "";
	switch ( dylib->cmd() ) {
	case LC_LOAD_WEAK_DYLIB:
	    attribute = "weak_import";
	    break;
	case LC_REEXPORT_DYLIB:
	    attribute = "re-export";
	    break;
	case LC_LOAD_UPWARD_DYLIB:
	    attribute = "upward";
	    break;
	case LC_LAZY_LOAD_DYLIB:
	    attribute = "lazy_load";
	    break;
	case LC_LOAD_DYLIB:
	default:
	    break;
	}
	printf(" %-12s   %s\n", attribute, dylib->name());
    }
}


template <>
ppc::P::uint_t DyldInfoPrinter<ppc>::relocBase()
{
    if ( fHeader->flags() & MH_SPLIT_SEGS )
	return fFirstWritableSegment->vmaddr();
    else
	return fFirstSegment->vmaddr();
}

template <>
ppc64::P::uint_t DyldInfoPrinter<ppc64>::relocBase()
{
    if ( fWriteableSegmentWithAddrOver4G ) 
	return fFirstWritableSegment->vmaddr();
    else
	return fFirstSegment->vmaddr();
}

template <>
x86::P::uint_t DyldInfoPrinter<x86>::relocBase()
{
    if ( fHeader->flags() & MH_SPLIT_SEGS )
	return fFirstWritableSegment->vmaddr();
    else
	return fFirstSegment->vmaddr();
}

template <>
x86_64::P::uint_t DyldInfoPrinter<x86_64>::relocBase()
{
    // check for split-seg
    return fFirstWritableSegment->vmaddr();
}

template <>
arm::P::uint_t DyldInfoPrinter<arm>::relocBase()
{
    if ( fHeader->flags() & MH_SPLIT_SEGS )
	return fFirstWritableSegment->vmaddr();
    else
	return fFirstSegment->vmaddr();
}


template <>
const char* DyldInfoPrinter<ppc>::relocTypeName(uint8_t r_type)
{
    if ( r_type == GENERIC_RELOC_VANILLA )
	return "pointer";
    else
	return "??";
}
 
template <>
const char* DyldInfoPrinter<ppc64>::relocTypeName(uint8_t r_type)
{
    if ( r_type == GENERIC_RELOC_VANILLA )
	return "pointer";
    else
	return "??";
}
 
template <>
const char* DyldInfoPrinter<x86>::relocTypeName(uint8_t r_type)
{
    if ( r_type == GENERIC_RELOC_VANILLA )
	return "pointer";
    else if ( r_type == GENERIC_RELOC_PB_LA_PTR )
	return "pb pointer";
    else
	return "??";
}
 
template <>
const char* DyldInfoPrinter<x86_64>::relocTypeName(uint8_t r_type)
{
    if ( r_type == X86_64_RELOC_UNSIGNED )
	return "pointer";
    else
	return "??";
}
 
template <>
const char* DyldInfoPrinter<arm>::relocTypeName(uint8_t r_type)
{
    if ( r_type == ARM_RELOC_VANILLA )
	return "pointer";
    else if ( r_type == ARM_RELOC_PB_LA_PTR )
	return "pb pointer";
    else
	return "??";
}
 

template <typename A>
void DyldInfoPrinter<A>::printRelocRebaseInfo()
{
    if ( fDynamicSymbolTable == NULL ) {
	printf("no classic dynamic symbol table");
    }
    else {
	printf("rebase information (from local relocation records and indirect symbol table):\n");
	printf("segment  section          address     type\n");
	// walk all local relocations
	pint_t rbase = relocBase();
	const macho_relocation_info<P>* const relocsStart = (macho_relocation_info<P>*)(((uint8_t*)fHeader) + fDynamicSymbolTable->locreloff());
	const macho_relocation_info<P>* const relocsEnd = &relocsStart[fDynamicSymbolTable->nlocrel()];
	for (const macho_relocation_info<P>* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
	    if ( (reloc->r_address() & R_SCATTERED) == 0 ) {
		pint_t addr = reloc->r_address() + rbase;
		uint8_t segIndex = segmentIndexForAddress(addr);
		const char* typeName = relocTypeName(reloc->r_type());
		const char* segName  = segmentName(segIndex);
		const char* sectName = sectionName(segIndex, addr);
		printf("%-8s %-16s 0x%08llX  %s\n", segName, sectName, (uint64_t)addr, typeName);
	    } 
	    else {
		const macho_scattered_relocation_info<P>* sreloc = (macho_scattered_relocation_info<P>*)reloc;
		pint_t addr = sreloc->r_address() + rbase;
		uint8_t segIndex = segmentIndexForAddress(addr);
		const char* typeName = relocTypeName(sreloc->r_type());
		const char* segName  = segmentName(segIndex);
		const char* sectName = sectionName(segIndex, addr);
		printf("%-8s %-16s 0x%08llX  %s\n", segName, sectName, (uint64_t)addr, typeName);
	    }
	}
	// look for local non-lazy-pointers
	const uint32_t* indirectSymbolTable =  (uint32_t*)(((uint8_t*)fHeader) + fDynamicSymbolTable->indirectsymoff());
	uint8_t segIndex = 0;
	for(typename std::vector<const macho_segment_command<P>*>::iterator segit=fSegments.begin(); segit != fSegments.end(); ++segit, ++segIndex) {
	    const macho_segment_command<P>* segCmd = *segit;
	    macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
	    macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
	    for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
		uint8_t type = sect->flags() & SECTION_TYPE;
		if ( type == S_NON_LAZY_SYMBOL_POINTERS ) {
		    uint32_t indirectOffset = sect->reserved1();
		    uint32_t count = sect->size() / sizeof(pint_t);
		    for (uint32_t i=0; i < count; ++i) {
			uint32_t symbolIndex = E::get32(indirectSymbolTable[indirectOffset+i]);
			if ( symbolIndex == INDIRECT_SYMBOL_LOCAL ) {
			    pint_t addr = sect->addr() + i*sizeof(pint_t);       
			    const char* typeName = "pointer";
			    const char* segName  = segmentName(segIndex);
			    const char* sectName = sectionName(segIndex, addr);
			    printf("%-8s %-16s 0x%08llX  %s\n", segName, sectName, (uint64_t)addr, typeName);
			}
		    }
		}
	    }
	}
    }
}


template <typename A>
void DyldInfoPrinter<A>::printSymbolTableExportInfo()
{
    if ( fDynamicSymbolTable == NULL ) {
	printf("no classic dynamic symbol table");
    }
    else {
	printf("export information (from symbol table):\n");
	const macho_nlist<P>* lastExport = &fSymbols[fDynamicSymbolTable->iextdefsym()+fDynamicSymbolTable->nextdefsym()];
	for (const macho_nlist<P>* sym = &fSymbols[fDynamicSymbolTable->iextdefsym()]; sym < lastExport; ++sym) {
	    const char* flags = "";
	    if ( sym->n_desc() & N_WEAK_DEF )
		flags = "[weak_def] ";
	    pint_t thumb = 0;
	    if ( sym->n_desc() & N_ARM_THUMB_DEF )
		thumb = 1;
	    printf("0x%08llX %s%s\n", sym->n_value()+thumb, flags, &fStrings[sym->n_strx()]);
	}
    }
}

template <typename A>
const char* DyldInfoPrinter<A>::symbolNameForAddress(uint64_t addr)
{
    if ( fDynamicSymbolTable != NULL ) {
	// find exact match in globals
	const macho_nlist<P>* lastExport = &fSymbols[fDynamicSymbolTable->iextdefsym()+fDynamicSymbolTable->nextdefsym()];
	for (const macho_nlist<P>* sym = &fSymbols[fDynamicSymbolTable->iextdefsym()]; sym < lastExport; ++sym) {
	    if ( (sym->n_value() == addr) && ((sym->n_type() & N_TYPE) == N_SECT) && ((sym->n_type() & N_STAB) == 0) ) {
		return &fStrings[sym->n_strx()];
	    }
	}
	// find exact match in local symbols
	const macho_nlist<P>* lastLocal = &fSymbols[fDynamicSymbolTable->ilocalsym()+fDynamicSymbolTable->nlocalsym()];
	for (const macho_nlist<P>* sym = &fSymbols[fDynamicSymbolTable->ilocalsym()]; sym < lastLocal; ++sym) {
	    if ( (sym->n_value() == addr) && ((sym->n_type() & N_TYPE) == N_SECT) && ((sym->n_type() & N_STAB) == 0) ) {
		return &fStrings[sym->n_strx()];
	    }
	}
    }
    else {
	// find exact match in all symbols
	const macho_nlist<P>* lastSym = &fSymbols[fSymbolCount];
	for (const macho_nlist<P>* sym = &fSymbols[0]; sym < lastSym; ++sym) {
	    if ( (sym->n_value() == addr) && ((sym->n_type() & N_TYPE) == N_SECT) && ((sym->n_type() & N_STAB) == 0) ) {
		return &fStrings[sym->n_strx()];
	    }
	}
    }

    return "?";
}
 
template <typename A>
void DyldInfoPrinter<A>::printClassicBindingInfo()
{
    if ( fDynamicSymbolTable == NULL ) {
	printf("no classic dynamic symbol table");
    }
    else {
	printf("binding information (from relocations and indirect symbol table):\n");
	printf("segment  section          address        type   weak  addend dylib            symbol\n");
	// walk all external relocations
	pint_t rbase = relocBase();
	const macho_relocation_info<P>* const relocsStart = (macho_relocation_info<P>*)(((uint8_t*)fHeader) + fDynamicSymbolTable->extreloff());
	const macho_relocation_info<P>* const relocsEnd = &relocsStart[fDynamicSymbolTable->nextrel()];
	for (const macho_relocation_info<P>* reloc=relocsStart; reloc < relocsEnd; ++reloc) {
	    pint_t addr = reloc->r_address() + rbase;
	    uint32_t symbolIndex = reloc->r_symbolnum();
	    const macho_nlist<P>* sym = &fSymbols[symbolIndex];
	    const char* symbolName = &fStrings[sym->n_strx()];
	    const char* weak_import = (sym->n_desc() & N_WEAK_REF) ? "weak" : "";
	    const char* fromDylib = classicOrdinalName(GET_LIBRARY_ORDINAL(sym->n_desc()));
	    uint8_t segIndex = segmentIndexForAddress(addr);
	    const char* typeName = relocTypeName(reloc->r_type());
	    const char* segName  = segmentName(segIndex);
	    const char* sectName = sectionName(segIndex, addr);
	    const pint_t* addressMapped = mappedAddressForVMAddress(addr);
	    int64_t addend = P::getP(*addressMapped); 
	    if ( fHeader->flags() & MH_PREBOUND ) {
		// In prebound binaries the content is already pointing to the target.
		// To get the addend requires subtracting out the base address it was prebound to.
		addend -= sym->n_value();
	    }
	    printf("%-8s %-16s 0x%08llX %10s %4s  %5lld %-16s %s\n", segName, sectName, (uint64_t)addr, 
		   typeName, weak_import, addend, fromDylib, symbolName);
	}
	// look for non-lazy pointers
	const uint32_t* indirectSymbolTable =  (uint32_t*)(((uint8_t*)fHeader) + fDynamicSymbolTable->indirectsymoff());
	for(typename std::vector<const macho_segment_command<P>*>::iterator segit=fSegments.begin(); segit != fSegments.end(); ++segit) {
	    const macho_segment_command<P>* segCmd = *segit;
	    macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
	    macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
	    for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
		uint8_t type = sect->flags() & SECTION_TYPE;
		if ( type == S_NON_LAZY_SYMBOL_POINTERS ) {
		    uint32_t indirectOffset = sect->reserved1();
		    uint32_t count = sect->size() / sizeof(pint_t);
		    for (uint32_t i=0; i < count; ++i) {
			uint32_t symbolIndex = E::get32(indirectSymbolTable[indirectOffset+i]);
			if ( symbolIndex != INDIRECT_SYMBOL_LOCAL ) {
			    const macho_nlist<P>* sym = &fSymbols[symbolIndex];
			    const char* symbolName = &fStrings[sym->n_strx()];
			    const char* weak_import = (sym->n_desc() & N_WEAK_REF) ? "weak" : "";
			    const char* fromDylib = classicOrdinalName(GET_LIBRARY_ORDINAL(sym->n_desc()));
			    pint_t addr = sect->addr() + i*sizeof(pint_t);
			    uint8_t segIndex = segmentIndexForAddress(addr);
			    const char* typeName = "pointer";
			    const char* segName  = segmentName(segIndex);
			    const char* sectName = sectionName(segIndex, addr);
			    int64_t addend = 0;
			    printf("%-8s %-16s 0x%08llX %10s %4s  %5lld %-16s %s\n", segName, sectName, (uint64_t)addr, 
				   typeName, weak_import, addend, fromDylib, symbolName);
			}
		    }
		}
	    }
	}
    }
}


template <typename A>
void DyldInfoPrinter<A>::printClassicLazyBindingInfo()
{
    if ( fDynamicSymbolTable == NULL ) {
	//printf("no classic dynamic symbol table");
    }
    else {
	//printf("lazy binding information (from section records and indirect symbol table):\n");
	//printf("segment section          address    index  dylib            symbol\n");
	const uint32_t* indirectSymbolTable =  (uint32_t*)(((uint8_t*)fHeader) + fDynamicSymbolTable->indirectsymoff());
	for(typename std::vector<const macho_segment_command<P>*>::iterator segit=fSegments.begin(); segit != fSegments.end(); ++segit) {
	    const macho_segment_command<P>* segCmd = *segit;
	    macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
	    macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
	    for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
		uint8_t type = sect->flags() & SECTION_TYPE;
		if ( type == S_LAZY_SYMBOL_POINTERS ) {
		    uint32_t indirectOffset = sect->reserved1();
		    uint32_t count = sect->size() / sizeof(pint_t);
		    for (uint32_t i=0; i < count; ++i) {
			uint32_t symbolIndex = E::get32(indirectSymbolTable[indirectOffset+i]);
			const macho_nlist<P>* sym = &fSymbols[symbolIndex];
			const char* symbolName = &fStrings[sym->n_strx()];
			const char* fromDylib = classicOrdinalName(GET_LIBRARY_ORDINAL(sym->n_desc()));
			pint_t addr = sect->addr() + i*sizeof(pint_t);
			uint8_t segIndex = segmentIndexForAddress(addr);
			const char* segName  = segmentName(segIndex);
			const char* sectName = sectionName(segIndex, addr);
			if (0) { printf("%-7s %-16s 0x%08llX 0x%04X %-16s %s\n", segName, sectName, (uint64_t)addr, symbolIndex, fromDylib, symbolName);}
			_importsvec.push_back(symbolName);      
		    }
		}
		else if ( (type == S_SYMBOL_STUBS) && (((sect->flags() & S_ATTR_SELF_MODIFYING_CODE) != 0)) && (sect->reserved2() == 5) ) {
		    // i386 self-modifying stubs
		    uint32_t indirectOffset = sect->reserved1();
		    uint32_t count = sect->size() / 5;
		    for (uint32_t i=0; i < count; ++i) {
			uint32_t symbolIndex = E::get32(indirectSymbolTable[indirectOffset+i]);
			if ( symbolIndex != INDIRECT_SYMBOL_ABS ) {
			    const macho_nlist<P>* sym = &fSymbols[symbolIndex];
			    const char* symbolName = &fStrings[sym->n_strx()];
			    const char* fromDylib = classicOrdinalName(GET_LIBRARY_ORDINAL(sym->n_desc()));
			    pint_t addr = sect->addr() + i*5;
			    uint8_t segIndex = segmentIndexForAddress(addr);
			    const char* segName  = segmentName(segIndex);
			    const char* sectName = sectionName(segIndex, addr);
			    if (0) { printf("%-7s %-16s 0x%08llX 0x%04X %-16s %s\n", segName, sectName, (uint64_t)addr, symbolIndex, fromDylib, symbolName); }
			    _importsvec.push_back(symbolName);
			}
		    }
		}
	    }
	}
    }
}



#if 0
static void usage()
{
    fprintf(stderr, "Usage: dyldinfo [-arch <arch>] <options> <mach-o file>\n"
	    "\t-dylibs           print dependent dylibs\n"
	    "\t-rebase           print addresses dyld will adjust if file not loaded at preferred address\n"
	    "\t-bind             print addresses dyld will set based on symbolic lookups\n"
	    "\t-weak_bind        print symbols which dyld must coalesce\n"
	    "\t-lazy_bind        print addresses dyld will lazily set on first use\n"
	    "\t-export           print addresses of all symbols this file exports\n"
	    "\t-opcodes          print opcodes used to generate the rebase and binding information\n"
	    "\t-function_starts  print table of function start addresses\n"
	    "\t-export_dot       print a GraphViz .dot file of the exported symbols trie\n"
	);
}



int main(int argc, const char* argv[])
{
    if ( argc == 1 ) {
	usage();
	return 0;
    }

    try {
	std::vector<const char*> files;
	for(int i=1; i < argc; ++i) {
	    const char* arg = argv[i];
	    if ( arg[0] == '-' ) {
		if ( strcmp(arg, "-arch") == 0 ) {
		    const char* arch = ++i<argc? argv[i]: "";
		    if ( strcmp(arch, "ppc64") == 0 )
			sPreferredArch = CPU_TYPE_POWERPC64;
		    else if ( strcmp(arch, "ppc") == 0 )
			sPreferredArch = CPU_TYPE_POWERPC;
		    else if ( strcmp(arch, "i386") == 0 )
			sPreferredArch = CPU_TYPE_I386;
		    else if ( strcmp(arch, "x86_64") == 0 )
			sPreferredArch = CPU_TYPE_X86_64;
		    else {
			bool found = false;
			for (const ARMSubType* t=ARMSubTypes; t->subTypeName != NULL; ++t) {
			    if ( strcmp(t->subTypeName,arch) == 0 ) {
				sPreferredArch = CPU_TYPE_ARM;
				sPreferredSubArch = t->subType;
				found = true;
				break;
			    }
			}
			if ( !found )
			    throwf("unknown architecture %s", arch);
		    }
		}
		else if ( strcmp(arg, "-rebase") == 0 ) {
		    printRebase = true;
		}
		else if ( strcmp(arg, "-bind") == 0 ) {
		    printBind = true;
		}
		else if ( strcmp(arg, "-weak_bind") == 0 ) {
		    printWeakBind = true;
		}
		else if ( strcmp(arg, "-lazy_bind") == 0 ) {
		    printLazyBind = true;
		}
		else if ( strcmp(arg, "-export") == 0 ) {
		    printExport = true;
		}
		else if ( strcmp(arg, "-opcodes") == 0 ) {
		    printOpcodes = true;
		}
		else if ( strcmp(arg, "-export_dot") == 0 ) {
		    printExportGraph = true;
		}
		else if ( strcmp(arg, "-export_trie_nodes") == 0 ) {
		    printExportNodes = true;
		}
		else if ( strcmp(arg, "-shared_region") == 0 ) {
		    printSharedRegion = true;
		}
		else if ( strcmp(arg, "-function_starts") == 0 ) {
		    printFunctionStarts = true;
		}
		else if ( strcmp(arg, "-dylibs") == 0 ) {
		    printDylibs = true;
		}
		else {
		    throwf("unknown option: %s\n", arg);
		}
	    }
	    else {
		files.push_back(arg);
	    }
	}
	if ( files.size() == 0 )
	    usage();
	if ( files.size() == 1 ) {
	    dump(files[0]);
	}
	else {
	    for(std::vector<const char*>::iterator it=files.begin(); it != files.end(); ++it) {
		printf("\n%s:\n", *it);
		dump(*it);
	    }
	}
    }
    catch (const char* msg) {
	fprintf(stderr, "dyldinfo failed: %s\n", msg);
	return 1;
    }
 
    return 0;
}
#endif



#endif /* HAVE_NSLINKMODULE */

#endif /* _DYLDINFO_HPP_ */
