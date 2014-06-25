//
//  glerror.c
//  glance
//
//  Created by Tiago Rezende on 6/24/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"

static t_class *c_glerror = NULL;
typedef struct _t_glerror t_glerror;

struct _t_glerror {
    t_object x_obj;
    t_outlet *outlet, *error_outlet;
};

static t_glerror *glerror_new(void) {
    t_glerror *obj = (t_glerror *)pd_new(c_glerror);
    obj->outlet = outlet_new(&obj->x_obj, gensym("out"));
    obj->error_outlet = outlet_new(&obj->x_obj, gensym("errors"));
    return obj;
}

static void glerror_free(t_glerror *err) {
    outlet_free(err->outlet);
}

static void glerror_bang(t_glerror *obj) {
    GLenum err;
    t_symbol *sym;
    while ((err = glGetError()) != GL_NO_ERROR) {
        switch (err) {
            case GL_INVALID_ENUM:
                sym = gensym("INVALID_ENUM");
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                sym = gensym("INVALID_FRAMEBUFFER_OPERATION");
                break;
            case GL_INVALID_INDEX:
                sym = gensym("INVALID_INDEX");
                break;
            case GL_INVALID_OPERATION:
                sym = gensym("INVALID_OPERATION");
                break;
            case GL_INVALID_VALUE:
                sym = gensym("INVALID_VALUE");
                break;
            case GL_OUT_OF_MEMORY:
                sym = gensym("OUT_OF_MEMORY");
                break;
            default:
                break;
        }
        outlet_symbol(obj->error_outlet, sym);
    }
}

static void glerror_render(t_glerror* err, t_symbol *sym, int argc, t_atom *argv) {
    glerror_bang(err);
    outlet_anything(err->outlet, sym, argc, argv);
}

void gl_error_setup(void) {
    c_glerror = class_new(gensym("gl.error"),
                          (t_newmethod)glerror_new,
                          (t_method)glerror_free,
                          sizeof(t_glerror),
                          CLASS_DEFAULT, A_NULL, 0);
    class_addmethod(c_glerror, (t_method) glerror_render, render, A_GIMME, 0);
    class_addbang(c_glerror, glerror_bang);
}
