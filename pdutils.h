//
//  pdutils.h
//  glance
//
//  Created by Tiago Rezende on 3/11/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#ifndef glance_pdutils_h
#define glance_pdutils_h

#include "m_pd.h"

/**
 * builds a string with the elements of a given list
 */
char *list_to_string(int argc, t_atom *argv);


/**
 * structures for quick t_symbol->int/uint matching/searching
 */
typedef struct _sym_uint_list {
    const char *sym;
    unsigned val;
} t_sym_uint_list;
typedef struct _sym_int_list {
    const char *sym;
    int val;
} t_sym_int_list;

/**
 * utility functions for quick t_symbol->int/uint matching/searching
 * @param start pointer to the beginning of a t_sym_<int/uint>_list
 * @param sym the symbol to find a match for
 * @param found a pointer to an int to store the result
 * @return 1 if found, 0 otherwise
 */
int find_uint_for_sym(t_sym_uint_list *start, t_symbol *sym, unsigned *found);
int find_int_for_sym(t_sym_uint_list *start, t_symbol *sym, int *found);

typedef enum {
    ARG_NONE,
    ARG_OK,
    ARG_IGNORE,
    ARG_ILLEGAL
} pd_msg_arg_status;

typedef struct _pd_msg_arg_option pd_msg_arg_option;

typedef pd_msg_arg_status (*t_check_arg)(void *ctx,
                                         pd_msg_arg_option *option,
                                         bool output_error);

typedef struct _pd_msg_arg_descriptor {
    const int index; //
    const int type;
    const char* short_opt;
    const char* long_opt;
    t_check_arg check_arg;
    const char* help;
} pd_msg_arg_descriptor;

struct _pd_msg_arg_option {
    pd_msg_arg_option *next;
    pd_msg_arg_option *prev;
    
    const pd_msg_arg_descriptor *desc;
    t_symbol *name;
    t_atom *arg;
};

int pd_msg_option_type(pd_msg_arg_option *option);
int pd_msg_option_index(pd_msg_arg_option *option);
size_t pd_msg_option_count(pd_msg_arg_option *option);
pd_msg_arg_option *pd_msg_option_next(pd_msg_arg_option *option);
pd_msg_arg_option *pd_msg_option_prev(pd_msg_arg_option *option);

pd_msg_arg_option *pd_msg_option_parse(void *data,
                                       int argc,
                                       t_atom *argv,
                                       pd_msg_arg_descriptor *descriptors);
pd_msg_arg_option *pd_msg_option_get(pd_msg_arg_option *option, int index);
void pd_msg_option_cleanup(pd_msg_arg_option *option);

#endif
