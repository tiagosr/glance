//
//  pdcpputils.h
//  glance
//
//  Created by Tiago Rezende on 7/25/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#ifndef __glance__pdcpputils__
#define __glance__pdcpputils__

#include "m_pd.h"
#include <vector>


class t_args_with_send;

struct t_out_msg {
    t_symbol *msg;
    explicit t_out_msg(t_symbol *sym): msg(sym) {}
    explicit t_out_msg(const char *sym);
    
    t_args_with_send& operator>>(t_float f);
    t_args_with_send& operator>>(t_symbol *sym);
    t_args_with_send& operator>>(const char *sym);
    
};

class t_args: std::vector<t_atom> {
public:
    void to_outlet(t_outlet *outlet, t_symbol *sym);
    
    t_args& add_symbol(t_symbol *sym);
    t_args& add_symbol(const char *sym);
    t_args& add_gpointer(t_gpointer *pointer);
    t_args& add_float(t_float f);
    t_args& add_bang();

    t_args& operator>>(t_float f);
    t_args& operator>>(t_symbol *sym);
    t_args& operator>>(const char *sym);
};

class t_args_with_send: public t_args {
    t_out_msg msg;
public:
    t_args_with_send(t_out_msg msg): msg(msg) {}
    t_args_with_send& operator>>(t_float f);
    t_args_with_send& operator>>(t_symbol *sym);
    t_args_with_send& operator>>(const char *sym);
    
    t_args_with_send& operator>>(t_outlet *outlet);
};


template <typename T>
class pd_msg_option_parser {
    
public:
    
};


#endif /* defined(__glance__pdcpputils__) */
