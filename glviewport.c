//
//  glviewport.c
//  glance
//
//  Created by Tiago Rezende on 3/18/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <stdbool.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"

static t_class *gl_viewport_class;

typedef struct _gl_viewport_obj {
    t_object x_obj;
    float x, y, width, height;
    t_outlet *out;
} t_gl_viewport_obj;

/**
 * creates a [gl.viewport] object.
 * mandatory arguments are x and y position
 * and width and height of the new viewport
 */
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

/**
 * [set x y w h[
 * sets the viewport dimensions
 */
static void gl_viewport_set(t_gl_viewport_obj *obj,
                            t_float x, t_float y, t_float w, t_float h) {
    obj->x = x; obj->y = y; obj->width = w; obj->height = h;
}

/**
 * the render call
 * stores previous viewport dimensions, sets up it's own dimensions, forwards
 * the render call to other objects then resets to the previous viewport
 */
static void gl_viewport_render(t_gl_viewport_obj *obj,
                               t_symbol *sym, int argc, t_atom *argv) {
    int oldviewport[4];
    glGetIntegerv(GL_VIEWPORT, oldviewport);
    glViewport(obj->x, obj->y, obj->width, obj->height);
    outlet_anything(obj->out, sym, argc, argv);
    glViewport(oldviewport[0], oldviewport[1], oldviewport[2], oldviewport[3]);
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