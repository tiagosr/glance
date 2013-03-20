//
//  glance.c
//  glance
//
//  Created by Tiago Rezende on 3/5/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include <stdio.h>
#include "glance.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 2

static t_class *glance_class;

t_symbol *render;

typedef struct _glance_obj {
    t_object x_obj;
    //
} t_glance_obj;

static void glance_bang(t_glance_obj *obj) {
    
}
static void * glance_new(void) {
    t_glance_obj *obj = (t_glance_obj *)pd_new(glance_class);
    return (void *)obj;
}



void glance_setup(void) {
    render = gensym("glance_render");
    glance_class = class_new(gensym("glance"),
                           (t_newmethod)glance_new,
                           0, sizeof(t_glance_obj),
                           CLASS_DEFAULT,
                           0);
    class_addbang(glance_class, glance_bang);
    gl_win_setup();
    gl_shader_setup();
    gl_vertexarray_setup();
    gl_uniform_setup();
    gl_uniform_matrix_setup();
    gl_viewport_setup();
    gl_clear_setup();
    post("glance: OpenGL/windowing/input libraries for PD\n"
         "        version %d.%d\n"
         "        (c)2013 Tiago Rezende\n", VERSION_MAJOR, VERSION_MINOR);
}
