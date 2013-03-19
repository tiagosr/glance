//
//  glclear.c
//  glance
//
//  Created by Tiago Rezende on 3/19/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include "glance.h"

static t_class *gl_clear_class = NULL;

typedef struct _gl_clear_obj {
    t_object x_obj;
    t_float r, g, b, a;
    t_float depth;
    int stencil;
    GLuint mode;
    t_outlet *out;
} t_gl_clear_obj;

static void *gl_clear_new(t_symbol *sym, int argc, t_atom *argv) {
    t_gl_clear_obj *obj = (t_gl_clear_obj *)pd_new(gl_clear_class);
    
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void gl_clear_destroy(t_gl_clear_obj *obj) {
    outlet_free(obj->out);
}

void gl_clear_setup(void) {
    gl_clear_class = class_new(gensym("gl.clear"),
                               (t_newmethod)gl_clear_new,
                               (t_method)gl_clear_destroy,
                               sizeof(t_gl_clear_obj), CLASS_DEFAULT,
                               A_GIMME, 0);
    
}