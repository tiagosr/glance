//
//  glclear.c
//  glance
//
//  Created by Tiago Rezende on 3/19/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"

static t_class *gl_clear_class = NULL;

typedef struct _gl_clear_obj {
    t_object x_obj;
    t_float r, g, b, a;
    t_float depth;
    int stencil;
    GLbitfield mask;
    t_outlet *out;
} t_gl_clear_obj;

static t_sym_uint_list gl_clear_modes[] = {
    {"COLOR", GL_COLOR_BUFFER_BIT},
    {"DEPTH", GL_DEPTH_BUFFER_BIT},
    {"STENCIL", GL_STENCIL_BUFFER_BIT},
    {0,0}
};
static void *gl_clear_new(t_symbol *sym, int argc, t_atom *argv) {
    t_gl_clear_obj *obj = (t_gl_clear_obj *)pd_new(gl_clear_class);
    obj->r = obj->g = obj->b = 0.0;
    obj->a = 1.0;
    obj->depth = 1.0;
    obj->stencil = 0;
    obj->mask = 0;
    for (int i = 0; i < argc; i++) {
        t_symbol *arg = atom_getsymbolarg(i, argc, argv);
        GLbitfield mask = 0;
        find_uint_for_sym(gl_clear_modes, arg, &mask);
        obj->mask |= mask;
    }
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void gl_clear_destroy(t_gl_clear_obj *obj) {
    outlet_free(obj->out);
}

static void gl_clear_color(t_gl_clear_obj *obj, t_float r, t_float g, t_float b) {
    obj->r = r;
    obj->g = g;
    obj->b = b;
}

static void gl_clear_depth(t_gl_clear_obj *obj, t_float depth) {
    obj->depth = depth;
}

static void gl_clear_stencil(t_gl_clear_obj *obj, t_float stencilf) {
    obj->stencil = stencilf;
}

static void gl_clear_set(t_gl_clear_obj *obj, t_symbol *sym, t_float val) {
    GLbitfield mask = 0;
    find_uint_for_sym(gl_clear_modes, sym, &mask);
    if (fabs(val) >= 1.0) {
        obj->mask |= mask;
    } else {
        obj->mask &= ~mask;
    }
}

static void gl_clear_render(t_gl_clear_obj *obj,
                            t_symbol *s, int argc, t_atom *argv) {
    if(obj->mask & GL_COLOR_BUFFER_BIT) glClearColor(obj->r, obj->g, obj->b, obj->a);
    if(obj->mask & GL_DEPTH_BUFFER_BIT) glClearDepth(obj->depth);
    if(obj->mask & GL_STENCIL_BUFFER_BIT) glClearStencil(obj->stencil);
    glClear(obj->mask);
    outlet_anything(obj->out, s, argc, argv);
}

static void gl_clear_reset(t_gl_clear_obj *obj)
{
    outlet_anything(obj->out, reset, 0, NULL);
}

void gl_clear_setup(void) {
    gl_clear_class = class_new(gensym("gl.clear"),
                               (t_newmethod)gl_clear_new,
                               (t_method)gl_clear_destroy,
                               sizeof(t_gl_clear_obj), CLASS_DEFAULT,
                               A_GIMME, 0);
    class_addmethod(gl_clear_class, (t_method)gl_clear_render, render, A_GIMME, 0);
    class_addmethod(gl_clear_class, (t_method)gl_clear_reset, reset, A_NULL, 0);
    class_addmethod(gl_clear_class, (t_method)gl_clear_color,
                    gensym("color"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_clear_class, (t_method)gl_clear_depth,
                    gensym("depth"), A_FLOAT, 0);
    class_addmethod(gl_clear_class, (t_method)gl_clear_color,
                    gensym("stencil"), A_FLOAT, 0);
    class_addmethod(gl_clear_class, (t_method)gl_clear_color,
                    gensym("set"), A_SYMBOL, A_FLOAT, 0);
    
}