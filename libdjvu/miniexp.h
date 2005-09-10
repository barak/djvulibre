/* -*- C -*-
// -------------------------------------------------------------------
// MiniExp - Library for handling lisp expressions
// Copyright (c) 2005  Leon Bottou
//
// This software is subject to, and may be distributed under, the
// GNU General Public License, Version 2. The license should have
// accompanied the software or you may obtain a copy of the license
// from the Free Software Foundation at http://www.fsf.org .
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// -------------------------------------------------------------------
*/
/* $Id$ */

#ifndef MINIEXP_H
#define MINIEXP_H

#ifdef __cplusplus
extern "C" { 
# ifndef __cplusplus
}
# endif
#endif

#ifndef MINILISPAPI
# define MINILISPAPI /**/
#endif
  

/* -------------------------------------------------- */
/* LISP EXPRESSIONS                                   */
/* -------------------------------------------------- */

/* miniexp_t -- 
   Opaque pointer type representing a lisp expression. */

typedef struct miniexp_s* miniexp_t;


/* There are four basic types of lisp expressions,
   numbers, symbols, pairs, and objects.
   The latter category can represent any c++ object
   that inherits class <miniobj_t> defined later in this file.
   The only such objects defined in this file are strings. */


/* -------- NUMBERS -------- */

/* Minilisp numbers can represent any integer 
   in range [-2^29...2^29-1] */


/* miniexp_numberp --
   Tests if an expression is a number. */

static inline int miniexp_numberp(miniexp_t p) {
  return (((size_t)(p)&3)==3);
}

/* miniexp_to_int --
   Returns the integer corresponding to a lisp expression.
   Assume that the expression is indeed a number. */

static inline int miniexp_to_int(miniexp_t p) {
  return (((int)(size_t)(p))>>2);
}

/* miniexp_number --
   Constructs the expression corresponding to an integer. */

static inline miniexp_t miniexp_number(int x) {
  return (miniexp_t) (size_t) ((x<<2)|3);
}
   


/* -------- SYMBOLS -------- */

/* The textual representation of a minilisp symbol is a 
   sequence of non blank characters forming an indentifier. 
   Each symbol has a unique representation and remain 
   permanently allocated. To compare two symbols, 
   simply compare the <miniexp_t> pointers. */


/* miniexp_symbolp --
   Tests if an expression is a symbol. */

static inline int miniexp_symbolp(miniexp_t p) {
  return ((((size_t)p)&3)==2);
}

/* miniexp_to_name --
   Returns the symbol name as a string.
   Returns NULL if the expression is not a symbol. */
   
MINILISPAPI const char* miniexp_to_name(miniexp_t p);

/* miniexp_symbol --
   Returns the unique symbol expression with the specified name. */

MINILISPAPI miniexp_t miniexp_symbol(const char *name);



/* -------- PAIRS -------- */

/* Pairs are the basic building blocks for minilisp lists.
   Each pair contains two expression:
   - the <car> represents the first element of a list.
   - the <cdr> usually is a pair representing the rest of the list.
   The empty list is represented by a null pointer. */


/* miniexp_nil --
   The empty list. */

const miniexp_t miniexp_nil = (miniexp_t)(size_t)0;

/* miniexp_dummy --
   An invalid expression used to represent
   various exceptional conditions. */

const miniexp_t miniexp_dummy = (miniexp_t)(size_t)2;

/* miniexp_listp --
   Tests if an expression is a list, 
   that is a pair or the empty list. */

static inline int miniexp_listp(miniexp_t p) {
  return ((((size_t)p)&3)==0);
}

/* miniexp_consp --
   Tests if an expression is a non empty list. */

static inline int miniexp_consp(miniexp_t p) {
  return p && miniexp_listp(p);
}

/* miniexp_length --
   Returns the length of a list.
   Returns 0 for non lists, -1 for circular lists. */

MINILISPAPI int miniexp_length(miniexp_t p);

/* miniexp_car --
   miniexp_cdr --
   Returns the car or cdr of a pair. */

static inline miniexp_t miniexp_car(miniexp_t p) {
  if (miniexp_consp(p))
    return ((miniexp_t*)p)[0];
  return miniexp_nil;
}

static inline miniexp_t miniexp_cdr(miniexp_t p) {
  if (miniexp_consp(p))
    return ((miniexp_t*)p)[1];
  return miniexp_nil;
}

/* miniexp_cXXr --
   Represent common combinations of car and cdr. */

MINILISPAPI miniexp_t miniexp_caar (miniexp_t p);
MINILISPAPI miniexp_t miniexp_cadr (miniexp_t p);
MINILISPAPI miniexp_t miniexp_cdar (miniexp_t p);
MINILISPAPI miniexp_t miniexp_cddr (miniexp_t p);
MINILISPAPI miniexp_t miniexp_caddr(miniexp_t p);
MINILISPAPI miniexp_t miniexp_cdddr(miniexp_t p);

/* miniexp_nth --
   Returns the n-th element of a list. */

MINILISPAPI miniexp_t miniexp_nth(int n, miniexp_t l);

/* miniexp_cons --
   Constructs a pair. */

MINILISPAPI miniexp_t miniexp_cons(miniexp_t car, miniexp_t cdr);

/* miniexp_list --
   Constructs a list with all arguments.
   The last argument must be <miniexp_dummy>. */

MINILISPAPI miniexp_t miniexp_list(miniexp_t p, ...);

/* miniexp_rplaca --
   miniexp_rplacd --
   Changes the car or the cdr of a pair. */

MINILISPAPI miniexp_t miniexp_rplaca(miniexp_t pair, miniexp_t newcar);
MINILISPAPI miniexp_t miniexp_rplacd(miniexp_t pair, miniexp_t newcdr);

/* miniexp_reverse --
   Reverses a list in place. */

MINILISPAPI miniexp_t miniexp_reverse(miniexp_t p);


/* -------- OBJECTS (GENERIC) -------- */

/* Object expressions represent a c++ object
   that inherits class <miniobj_t> defined later.
   Each object expression has a symbolic class name
   and a pointer to the c++ object. */

/* miniexp_objectp --
   Tests if an expression is an object. */

static inline int miniexp_objectp(miniexp_t p) {
  return ((((size_t)p)&3)==1);
}

/* miniexp_classof --
   Returns the symbolic class of an expression.
   Returns nil if the expression is not an object. */

MINILISPAPI miniexp_t miniexp_classof(miniexp_t p);

/* miniexp_isa --
   If <p> is an instance of class named <c> or one of
   its subclasses, returns the actual class name.
   Otherwise returns miniexp_nil. */

MINILISPAPI miniexp_t miniexp_isa(miniexp_t p, miniexp_t c);


/* -------- OBJECTS (STRINGS) -------- */

/* The most common object objects are strings. */

/* miniexp_stringp --
   Tests that an expression is a string. */

MINILISPAPI int miniexp_stringp(miniexp_t p);

/* miniexp_to_str --
   Returns the c string represented by the expression.
   Returns NULL if the expression is not a string.
   The c string remains valid as long as the
   corresponding lisp object exists. */

MINILISPAPI const char *miniexp_to_str(miniexp_t p);

/* miniexp_string --
   Constructs a string expression by copying string s. */

MINILISPAPI miniexp_t miniexp_string(const char *s);

/* miniexp_substring --
   Constructs a string expression by copying 
   at most n character from string s. */

MINILISPAPI miniexp_t miniexp_substring(const char *s, int n);

/* miniexp_concat --
   Concat all the string expressions in list <l>. */

MINILISPAPI miniexp_t miniexp_concat(miniexp_t l);





/* -------------------------------------------------- */
/* GARBAGE COLLECTION                                 */
/* -------------------------------------------------- */


/* The garbage collector reclaims the memory 
   allocated for lisp expressions no longer in use.
   The trick is to determine which lisp expressions
   are in use at a given moment.

   Minilisp takes a somehow simplistic approach:
   Type <minivar_t> represents a lisp expression *variable*.
   The expressions in use are defined as 
   - all lisp expressions referenced by a minivar 
   - all lisp expressions referenced by a lisp expression
     that has already been determined to be in use.
     
   C++ program can directly use instances of <minivar_t> 
   as if they were normal <miniexp_t> variables.
   C programs must use wrappers to allocate minivars 
   and to access their value. The garbage collector is 
   automatically invoked when the available memory runs low. 
   It is however possible to temporarily disable it. */


/* minilisp_gc --
   Invoke the garbage collector now. */

MINILISPAPI void minilisp_gc(void);

/* minilisp_info --
   Prints garbage collector statistics. */

MINILISPAPI void minilisp_info(void);

/* minilisp_acquire_gc_lock --
   minilisp_release_gc_lock --
   Temporarily disables automatic garbage collection.
   Acquire/release pairs may be nested. */

MINILISPAPI void minilisp_acquire_gc_lock(void);
MINILISPAPI void minilisp_release_gc_lock(void);

/* minivar_t --
   The minivar type. */
#ifdef __cplusplus
class minivar_t;
#else
typedef struct minivar_s minivar_t;
#endif

/* minivar_alloc --
   minivar_free --
   Wrappers for allocating and deallocating minivar objects. */

MINILISPAPI minivar_t *minivar_alloc(void);
MINILISPAPI void minivar_free(minivar_t *v);

/* minivar_pointer --
   Get a pointer to the miniexp_t reference 
   managed by a minivar. */

MINILISPAPI miniexp_t *minivar_pointer(minivar_t *v);

/* minilisp_finish --
   Deallocate everything.
   Nothing should be reused after calling this. */

MINILISPAPI void minilisp_finish(void);


/* -------------------------------------------------- */
/* INPUT/OUTPUT                                       */
/* -------------------------------------------------- */

/* Notes about the textual represenation of miniexps.

   - Special characters are:
     * the parenthesis <(> and <)>,
     * the double quote <">,
     * the vertical bar <|>,
     * any ascii character with a non zero entry 
       in array <minilisp_macrochar_parser>.

   - Symbols are represented by their name.
     Vertical bars <|> can be used to delimit names that
     contain blanks, special characters, non printable
     ascii characters, non ascii characters, or 
     can be confused as a number.
     
   - Numbers follow the syntax specified by the C
     function strtol() with base=0.

   - Strings are delimited by double quotes.
     All C string escapes are recognized.

   - List are represented by an open parenthesis <(>
     followed by the space separated list elements,
     followed by a closed parenthesis <)>.
     When the cdr of the last pair is non zero,
     the closed parenthesis is preceded by 
     a space, a dot <.>, a space, and the textual 
     representation of the cdr.

   - When the parser encounters an ascii character corresponding
     to a non zero function pointer in <minilisp_macrochar_parser>,
     the function is invoked and must return a possibly empty
     list of miniexps to be returned by the parser. */


/* minilisp_puts/getc/ungetc --
   All minilisp i/o is performed by invoking 
   these functions pointers. */

extern MINILISPAPI int (*minilisp_puts)(const char *s);
extern MINILISPAPI int (*minilisp_getc)(void);
extern MINILISPAPI int (*minilisp_ungetc)(int c);

/* minilisp_set_output --
   minilisp_set_input --
   Sets the above function to read/write from/to file f. 
   Only defined when <stdio.h> has been included. */

#if defined(stdin)
MINILISPAPI void minilisp_set_output(FILE *f);
MINILISPAPI void minilisp_set_input(FILE *f);
#endif

/* miniexp_read --
   Reads an expression by repeatedly
   invoking <minilisp_getc> and <minilisp_ungetc>.
   Returns <miniexp_dummy> when an error occurs. */

MINILISPAPI miniexp_t miniexp_read(void);

/* miniexp_prin --
   miniexp_print --
   Prints a minilisp expression by repeatedly invoking <minilisp_puts>.
   Only <minilisp_print> outputs a final newline character. */

MINILISPAPI miniexp_t miniexp_prin(miniexp_t p);
MINILISPAPI miniexp_t miniexp_print(miniexp_t p);

/* miniexp_pprin --
   miniexp_pprint --
   Prints a minilisp expression with reasonably pretty line breaks. 
   Argument <width> is the intended number of columns. 
   Only <minilisp_pprint> outputs a final newline character. */

MINILISPAPI miniexp_t miniexp_pprin(miniexp_t p, int width);
MINILISPAPI miniexp_t miniexp_pprint(miniexp_t p, int width);

/* minilisp_print_7bits --
   When this flag is set, all non 7bit characters 
   in strings are escaped in octal. */

extern MINILISPAPI int minilisp_print_7bits;

/* minilisp_macrochar_parser --
   A non zero entry in this array defines a special parsing
   function that runs when the corresponding character is
   encountered. */

extern MINILISPAPI miniexp_t (*minilisp_macrochar_parser[128])(void);



/* -------------------------------------------------- */
/* STUFF FOR C++ ONLY                                 */
/* -------------------------------------------------- */

#ifdef __cplusplus
# ifndef __cplusplus
{
# endif
} // extern "C"

typedef void minilisp_mark_t(miniexp_t *pp);

/* -------- MINIVARS -------- */

/* minivar_t --
   A class for protected garbage collector variables. */

MINILISPAPI 
class minivar_t 
{
  miniexp_t data;
  minivar_t *next;
  minivar_t **pprev;
public:
  ~minivar_t();
  minivar_t();
  minivar_t(miniexp_t p);
  minivar_t(const minivar_t &v);
  operator miniexp_t&() { return data; }
  operator miniexp_t() const { return data; }
  miniexp_t* operator&() { return &data; }
  minivar_t& operator=(miniexp_t p) { data = p; return *this; }
  minivar_t& operator=(const minivar_t &v) { data = v.data; return *this; }
#ifdef MINIEXP_IMPLEMENTATION
  static minivar_t *vars;
  static void mark(minilisp_mark_t*);
#endif
};


/* -------- MINIOBJ -------- */


/* miniobj_t --
   The base class for c++ objects 
   represented by object expressions. */

MINILISPAPI 
class miniobj_t {
 public:
  virtual ~miniobj_t();

  /* --- stuff defined by MINIOBJ_DECLARE --- */
  /* classname: a symbol characterizing this class. */
  static miniexp_t classname;
  /* classof: class name symbol for this object. */
  virtual miniexp_t classof() const = 0;
  /* isa -- tests if this is an instance of <classname>. */
  virtual bool isa(miniexp_t classname) const;
  
  /* --- optional stuff --- */
  /* mark: iterate over miniexps contained by this c++ object
     for garbage collecting purposes. */
  virtual void mark(minilisp_mark_t action);
  /* pname: returns a printable name for this object.
     The caller must deallocate the result with delete[]. */
  virtual char *pname() const;
};

/* MINIOBJ_DECLARE --
   MINIOBJ_IMPLEMENT --
   Useful code fragments for implementing 
   miniobj subclasses. */

#define MINIOBJ_DECLARE(cls, supercls, name) \
  public: static miniexp_t classname; \
          virtual miniexp_t classof() const; \
          virtual bool isa(miniexp_t) const; 

#define MINIOBJ_IMPLEMENT(cls, supercls, name)\
  miniexp_t cls::classname = miniexp_symbol(name);\
  miniexp_t cls::classof() const {\
    return cls::classname; }\
  bool cls::isa(miniexp_t n) const {\
  return (cls::classname==n) || (supercls::isa(n)); }


/* miniexp_to_obj --
   Returns a pointer to the object represented
   by an object expression. */

static inline miniobj_t *miniexp_to_obj(miniexp_t p) {
  if (miniexp_objectp(p))
    return ((miniobj_t**)(((size_t)p)&~((size_t)3)))[0];
  return 0;
}

/* miniexp_object --
   Create an object expression. */

MINILISPAPI miniexp_t miniexp_object(miniobj_t *obj);


#endif /* __cplusplus */





/* -------------------------------------------------- */
/* THE END                                            */
/* -------------------------------------------------- */

#endif /* MINIEXP_H */
