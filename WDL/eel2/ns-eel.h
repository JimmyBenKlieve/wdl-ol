/*
  Nullsoft Expression Evaluator Library (NS-EEL)
  Copyright (C) 1999-2003 Nullsoft, Inc.
  
  ns-eel.h: main application interface header

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/


#ifndef __NS_EEL_H__
#define __NS_EEL_H__

// put standard includes here
#include <stdlib.h>
#include <stdio.h>

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#ifndef EEL_F_SIZE
#define EEL_F_SIZE 8
#endif

#include "../wdltypes.h"

#if EEL_F_SIZE == 4
typedef float EEL_F;
#else
typedef double EEL_F WDL_FIXALIGN;
#endif


#ifdef __cplusplus
extern "C" {
#endif

// host should implement these (can be empty stub functions if no VM will execute code in multiple threads at once)

  // implement if you will be running the code in same VM from multiple threads, 
  // or VMs that have the same GRAM pointer from different threads, or multiple
  // VMs that have a NULL GRAM pointer from multiple threads.
  // if you give each VM it's own unique GRAM and only run each VM in one thread, then you can leave it blank.

  // or if you're daring....

void NSEEL_HOSTSTUB_EnterMutex();
void NSEEL_HOSTSTUB_LeaveMutex();


int NSEEL_init(); // returns 0 on success. clears any added functions as well



// adds a function that returns a value (EEL_F)
#define NSEEL_addfunc_retval(name,np,pproc,fptr) \
    NSEEL_addfunctionex(name,np,(char *)_asm_generic##np##parm_retd,(char *)_asm_generic##np##parm_retd##_end-(char *)_asm_generic##np##parm_retd,(void*)(pproc),(void*)(fptr))

// adds a function that returns a pointer (EEL_F*)
#define NSEEL_addfunc_retptr(name,np,pproc,fptr) \
    NSEEL_addfunctionex(name,np,(char *)_asm_generic##np##parm,(char *)_asm_generic##np##parm##_end-(char *)_asm_generic##np##parm,(void*)(pproc),(void*)(fptr))

// adds a void or bool function
#define NSEEL_addfunc_retbool(name,np,pproc,fptr) \
  NSEEL_addfunctionex(name,(np)|(/*BIF_RETURNSBOOL*/0x00400),(char *)_asm_generic##np##parm_retd,(char *)_asm_generic##np##parm_retd##_end-(char *)_asm_generic##np##parm_retd,(void*)(pproc),(void*)(fptr))


#define NSEEL_addfunction(name,nparms,code,len) NSEEL_addfunctionex((name),(nparms),(code),(len),0,0)
#define NSEEL_addfunctionex(name,nparms,code,len,pproc,fptr) NSEEL_addfunctionex2((name),(nparms),(code),(len),(pproc),(fptr),0)

typedef void *(*NSEEL_PPPROC)(void *data, int data_size, struct _compileContext *userfunc_data);

void NSEEL_addfunctionex2(const char *name, int nparms, char *code_startaddr, int code_len, NSEEL_PPPROC pproc, void *fptr, void *fptr2);

void NSEEL_quit();

int *NSEEL_getstats(); // returns a pointer to 5 ints... source bytes, static code bytes, call code bytes, data bytes, number of code handles

typedef void *NSEEL_VMCTX;
typedef void *NSEEL_CODEHANDLE;

NSEEL_VMCTX NSEEL_VM_alloc(); // return a handle
void NSEEL_VM_free(NSEEL_VMCTX ctx); // free when done with a VM and ALL of its code have been freed, as well

void NSEEL_VM_remove_unused_vars(NSEEL_VMCTX _ctx);
void NSEEL_VM_clear_var_refcnts(NSEEL_VMCTX _ctx);
void NSEEL_VM_remove_all_nonreg_vars(NSEEL_VMCTX _ctx);
void NSEEL_VM_enumallvars(NSEEL_VMCTX ctx, int (*func)(const char *name, EEL_F *val, void *ctx), void *userctx); // return false from func to stop

EEL_F *NSEEL_VM_regvar(NSEEL_VMCTX ctx, const char *name); // register a variable (before compilation)
int  NSEEL_VM_get_var_refcnt(NSEEL_VMCTX _ctx, const char *name); // returns -1 if not registered, or >=0

void NSEEL_VM_freeRAM(NSEEL_VMCTX ctx); // clears and frees all (VM) RAM used
void NSEEL_VM_freeRAMIfCodeRequested(NSEEL_VMCTX); // call after code to free the script-requested memory
int NSEEL_VM_wantfreeRAM(NSEEL_VMCTX ctx); // want NSEEL_VM_freeRAMIfCodeRequested?

// if you set this, it uses a local GMEM context. 
// Must be set before compilation. 
// void *p=NULL; 
// NSEEL_VM_SetGRAM(ctx,&p);
// .. do stuff
// NSEEL_VM_FreeGRAM(&p);
void NSEEL_VM_SetGRAM(NSEEL_VMCTX ctx, void **gram); 
void NSEEL_VM_FreeGRAM(void **ufd); // frees a gmem context.
void NSEEL_VM_SetCustomFuncThis(NSEEL_VMCTX ctx, void *thisptr);


NSEEL_CODEHANDLE NSEEL_code_compile(NSEEL_VMCTX ctx, const char *code, int lineoffs);
#define NSEEL_CODE_COMPILE_FLAG_COMMONFUNCS 1 // allows that code's functions to be used in other code (note you shouldn't destroy that codehandle without destroying others first if used)
#define NSEEL_CODE_COMPILE_FLAG_COMMONFUNCS_RESET 2 // resets common code functions

NSEEL_CODEHANDLE NSEEL_code_compile_ex(NSEEL_VMCTX ctx, const char *code, int lineoffs, int flags);

char *NSEEL_code_getcodeerror(NSEEL_VMCTX ctx);
void NSEEL_code_execute(NSEEL_CODEHANDLE code);
void NSEEL_code_free(NSEEL_CODEHANDLE code);
int *NSEEL_code_getstats(NSEEL_CODEHANDLE code); // 4 ints...source bytes, static code bytes, call code bytes, data bytes
  

// global memory control/view
extern unsigned int NSEEL_RAM_limitmem; // if nonzero, memory limit for user data, in bytes
extern unsigned int NSEEL_RAM_memused;
extern int NSEEL_RAM_memused_errors;



// configuration:

  // the old parser may have more quirks. 
  // Changes in the new parser:
  //   1) expressions such as a = (1+5;3); now work as expected (a is set to 3, rather than 4).
  //   2) 0xHEXNUMBER is now allowed (old parser required $xHEXNUMBER
  //   3) error notices (unsure which is more accurate)
  //   4) new parser allows more than 3 parameter eel-functions (up to NSEEL_MAX_EELFUNC_PARAMETERS)

  //#define NSEEL_USE_OLD_PARSER
#define NSEEL_SUPER_MINIMAL_LEXER // smaller code that uses far less ram, but the flex version we'll keep around too in case we want to do fancier things someday

 // #define NSEEL_EEL1_COMPAT_MODE // supports old behaviors (continue after failed compile), old functions _bnot etc.

#define NSEEL_MAX_VARIABLE_NAMELEN 128  // define this to override the max variable length
#define NSEEL_MAX_EELFUNC_PARAMETERS 40
#define NSEEL_MAX_FUNCSIG_NAME 2048 // longer than variable maxlen, due to multiple namespaces

// maximum loop length
#define NSEEL_LOOPFUNC_SUPPORT_MAXLEN 1048576 // scary, we can do a million entries. probably will never want to, though.
#define NSEEL_LOOPFUNC_SUPPORT_MAXLEN_STR "1048576"


#define NSEEL_MAX_FUNCTION_SIZE_FOR_INLINE 2048

// when a VM ctx doesn't have a GRAM context set, make the global one this big
#define NSEEL_SHARED_GRAM_SIZE (1<<20)




// note: if you wish to change NSEEL_RAM_*, and your target is x86-64, you will need to regenerate things.

// on osx:
//  php a2x64.php win64x
//  php a2x64.php macho64

// or on win32:
//  php a2x64.php
//  php a2x64.php macho64x
// this will regenerate the .asm files and object files

// 128*65536 = ~8million entries. (64MB RAM used)


#define NSEEL_RAM_BLOCKS_LOG2 7
#define NSEEL_RAM_ITEMSPERBLOCK_LOG2 16

#define NSEEL_RAM_BLOCKS (1 << NSEEL_RAM_BLOCKS_LOG2)
#define NSEEL_RAM_ITEMSPERBLOCK (1<<NSEEL_RAM_ITEMSPERBLOCK_LOG2)

#define NSEEL_STACK_SIZE 4096 // about 64k overhead if the stack functions are used in a given code handle

// arch neutral mode, runs about 1/8th speed or so
//#define EEL_TARGET_PORTABLE

#ifdef EEL_TARGET_PORTABLE
#define EEL_BC_TYPE int
#endif

#ifdef __cplusplus
}
#endif

#endif//__NS_EEL_H__
