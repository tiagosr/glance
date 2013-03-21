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

char *list_to_string(int argc, t_atom *argv);
typedef struct _sym_uint_list {
    const char *sym;
    unsigned val;
} t_sym_uint_list;
typedef struct _sym_int_list {
    const char *sym;
    int val;
} t_sym_int_list;

int find_uint_for_sym(t_sym_uint_list *start, t_symbol *sym, unsigned *found);
int find_int_for_sym(t_sym_uint_list *start, t_symbol *sym, int *found);


#endif
