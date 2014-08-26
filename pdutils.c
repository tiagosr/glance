//
//  pdutils.c
//  glance
//
//  Created by Tiago Rezende on 3/11/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "m_pd.h"
#include "pdutils.h"



char *list_to_string(int argc, t_atom *argv) {
    char str[MAXPDSTRING];
    char buf[MAXPDSTRING];
    str[0] = 0;
    if (argc > 0) {
        atom_string(argv, buf, MAXPDSTRING);
        if (argc > 1) {
            sprintf(str, "%s %s", buf, list_to_string(argc-1, argv+1));
        } else {
            sprintf(str, "%s", buf);
        }
    }
    return strdup(str);
}

int find_uint_for_sym(t_sym_uint_list *start, t_symbol *sym, unsigned *found) {
    while (start->sym != NULL) {
        if (gensym(start->sym) == sym) {
            *found = start->val;
            return 1;
        }
        start++;
    }
    return 0;
}
int find_int_for_sym(t_sym_uint_list *start, t_symbol *sym, int *found) {
    while (start->sym != NULL) {
        if (gensym(start->sym) == sym) {
            *found = start->val;
            return 1;
        }
        start++;
    }
    return 0;
}

int pd_msg_option_type(pd_msg_arg_option *option) {
    return option?option->desc->index:0;
}
int pd_msg_option_index(pd_msg_arg_option *option) {
    return option?option->desc->type:-1;
}
size_t pd_msg_option_count(pd_msg_arg_option *option) {
    if (!option) {
        return 0;
    }
    size_t count = 1;
    while (option->prev) {
        option = option->prev;
    }
    while (option->next) {
        option = option->next;
        count++;
    }
    return count;
}
pd_msg_arg_option *pd_msg_option_next(pd_msg_arg_option *option) {
    return option->next;
}

pd_msg_arg_option *pd_msg_option_prev(pd_msg_arg_option *option) {
    return option->prev;
}

pd_msg_arg_option *pd_msg_option_parse(void *data,
                                       int argc,
                                       t_atom *argv,
                                       pd_msg_arg_descriptor *descriptors) {
    pd_msg_arg_option *first = NULL;
    pd_msg_arg_option *option = NULL;
    pd_msg_arg_descriptor *current_descr = NULL;
    for (int i = 0; i < argc; i++) {
        if (current_descr == NULL) {
            current_descr = descriptors;
            pd_msg_arg_status status = ARG_ILLEGAL;
            t_symbol *found = atom_getsymbolarg(i, argc, argv);
            pd_msg_arg_option *current = malloc(sizeof(pd_msg_arg_option));
            current->arg = NULL;
            current->desc = NULL;
            current->prev = option;
            current->next = NULL;
            if (first == NULL) {
                first = current;
            }
            while (current_descr &&
                   current_descr->short_opt &&
                   current_descr->long_opt &&
                   (status == ARG_ILLEGAL)) {
                if ((strcmp(found->s_name, current_descr->short_opt) == 0) ||
                    (strcmp(found->s_name, current_descr->long_opt) == 0)) {
                    option->name = found;
                    option->desc = current_descr;
                    option->arg = i < argc-1 ? argv+(i+1):NULL;
                    if ((status = current_descr->check_arg(data, option, false)) == ARG_NONE) {
                        option->arg = NULL;
                    } else if (status != ARG_ILLEGAL) {
                        i++;
                    }
                }
            }
            if (status != ARG_ILLEGAL) {
                option = malloc(sizeof(pd_msg_arg_option));
                option->desc = current_descr;
                
            } else {
                current_descr = NULL;
            }
        } else {
            
        }
        
    }
    return first;
}
pd_msg_arg_option *pd_msg_option_get(pd_msg_arg_option *option, int index) {
    if (!option) return NULL;
    while (option->prev) {
        option = option->prev;
    }
    do {
        if (option->desc) {
            if (option->desc->index == index) {
                return option;
            }
        }
        option = option->next;
    } while (option);
    return NULL;
}
void pd_msg_option_cleanup(pd_msg_arg_option *option) {
    if (!option) return;
    while (option->prev) {
        option = option->prev;
    }
    while (option) {
        pd_msg_arg_option *current = option;
        option = option->next;
        free(current);
    }
}

