//
//  glscissor.c
//  glance
//
//  Created by Tiago Rezende on 1/18/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <stdbool.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"

static t_class *gl_scissor_class;

typedef struct _gl_scissor_obj t_gl_scissor_obj;

struct _gl_scissor_obj {
    t_object x_obj;
    t_outlet *out;
    int x, y;
    size_t width, height;
};

static void *gl_scissor_new(t_float x, t_float y, t_float w, t_float h) {
    t_gl_scissor_obj *obj = (t_gl_scissor_obj *)pd_new(gl_scissor_class);
    obj->x = x;
    obj->y = y;
    obj->width = w;
    obj->height = h;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return obj;
}

static void gl_scissor_delete(t_gl_scissor_obj *obj) {
    // nuthin'
}

static void gl_scissor_render(t_gl_scissor_obj *obj,
                              t_symbol *sym, int argc, t_atom *argv) {
    int temp[4];
    glGetIntegerv(GL_SCISSOR_BOX, temp);
    glScissor(obj->x, obj->y, obj->width, obj->height);
    outlet_anything(obj->out, sym, argc, argv);
    glScissor(temp[0], temp[1], temp[2], temp[3]);
}

static void gl_scissor_set(t_gl_scissor_obj *obj,
                           t_float x, t_float y, t_float w, t_float h) {
    obj->x = x;
    obj->y = y;
    obj->width = w;
    obj->height = h;
}

void gl_scissor_setup(void) {
    gl_scissor_class = class_new(gensym("gl.scissor"),
                                 (t_newmethod)gl_scissor_new,
                                 (t_method)gl_scissor_delete,
                                 sizeof(t_gl_scissor_obj),
                                 CLASS_DEFAULT,
                                 A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, NULL);
    class_addmethod(gl_scissor_class,
                    (t_method)gl_scissor_set, gensym("set"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_scissor_class,
                    (t_method)gl_scissor_render, render, A_GIMME, 0);
}