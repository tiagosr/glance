//
//  pdcpputils.cpp
//  glance
//
//  Created by Tiago Rezende on 7/25/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#include "pdcpputils.hh"


t_out_msg::t_out_msg(const char *msg) {
    t_out_msg(gensym(msg));
}

void t_args::to_outlet(t_outlet *outlet, t_symbol *sym) {
    outlet_anything(outlet, sym, size(), data());
}

t_args& t_args::add_bang() {
    t_atom a; SETSYMBOL(&a, &s_bang); push_back(a); return *this;
}

t_args& t_args::add_float(t_float f) {
    t_atom a; SETFLOAT(&a, f); push_back(a); return *this;
}

t_args& t_args::add_symbol(t_symbol *sym) {
    t_atom a; SETSYMBOL(&a, sym); push_back(a); return *this;
}

t_args& t_args::add_symbol(const char *sym) {
    return add_symbol(gensym(sym));
}

t_args& t_args::add_gpointer(t_gpointer *pointer) {
    t_atom a; SETPOINTER(&a, pointer); push_back(a); return *this;
}

t_args& t_args::operator>>(t_float f) { return add_float(f); }
t_args& t_args::operator>>(t_symbol *s) { return add_symbol(s); }
t_args& t_args::operator>>(const char *c) { return add_symbol(c); }

t_args_with_send& t_args_with_send::operator>>(t_float f) { add_float(f); return *this; }
t_args_with_send& t_args_with_send::operator>>(t_symbol *s) { add_symbol(s); return *this; }
t_args_with_send& t_args_with_send::operator>>(const char *c) { add_symbol(c); return *this; }
t_args_with_send& t_args_with_send::operator>>(t_outlet *outlet) {
    to_outlet(outlet, msg.msg);
    return *this;
}