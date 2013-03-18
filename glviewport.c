//
//  glviewport.c
//  glance
//
//  Created by Tiago Rezende on 3/18/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include "glance.h"

static t_class *gl_viewport_class;

typedef struct _gl_viewport_obj {
    t_object x_obj;
    float x, y, width, height;
    t_outlet *out;
} t_gl_viewport_obj;


static void *gl_viewport_new(t_float x, t_float y, t_float w, t_float h) {
    t_gl_viewport_obj *obj = (t_gl_viewport_obj *)pd_new(gl_viewport_class);
    obj->x = x;
    obj->y = y;
    obj->width = w;
    obj->height = h;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    floatinlet_new(&obj->x_obj, &obj->x);
    floatinlet_new(&obj->x_obj, &obj->y);
    floatinlet_new(&obj->x_obj, &obj->width);
    floatinlet_new(&obj->x_obj, &obj->height);
    return (void *)obj;
}

static void gl_viewport_set(t_gl_viewport_obj *obj,
                            t_float x, t_float y, t_float w, t_float h) {
    obj->x = x; obj->y = y; obj->width = w; obj->height = h;
}

static void gl_viewport_render(t_gl_viewport_obj *obj,
                               t_symbol *sym, int argc, t_atom *argv) {
    glViewport(obj->x, obj->y, obj->width, obj->height);
    outlet_anything(obj->out, sym, argc, argv);
}

void gl_viewport_setup(void) {
    gl_viewport_class = class_new(gensym("gl.viewport"),
                                  (t_newmethod)gl_viewport_new,
                                  0, sizeof(t_gl_viewport_obj),
                                  CLASS_DEFAULT,
                                  A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_viewport_class,
                    (t_method)gl_viewport_set, gensym("set"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_viewport_class,
                    (t_method)gl_viewport_render, render, A_GIMME, 0);
}