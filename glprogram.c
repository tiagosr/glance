//
//  glprogram.c
//  glance
//
//  Created by Tiago Rezende on 3/6/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>

static t_class *glance_program_class;
struct _glance_program {
    t_object x_obj;
    GLuint progid;
    GLint compile_status;
    t_outlet *data_out;
};

typedef struct _glance_program t_glance_program;
static void * glance_program_new(void) {
    t_glance_program *obj = (t_glance_program *)pd_new(glance_program_class);
    inlet_new(&obj->x_obj, &obj->x_obj.ob_pd, &s_anything, &s_anything);
    obj->progid = glCreateProgram();
    obj->compile_status = 0;
    obj->data_out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void glance_program_compile(t_glance_program *obj) {
    glLinkProgram(obj->progid);
    glGetProgramiv(obj->progid, GL_LINK_STATUS, &(obj->compile_status));
    outlet_float(obj->data_out, obj->compile_status);
}

static void glance_program_bang(t_glance_program *obj) {
    if (obj->compile_status) {
        glUseProgram(obj->progid);
    } else {
        error("shaders were not compiled or not specified");
    }
}

void gl_program_setup(void) {
    glance_program_class = class_new(gensym("gl.program"),
                                     (t_newmethod)glance_program_new, 0,
                                     sizeof(t_glance_program), CLASS_DEFAULT, 0);
    class_addmethod(glance_program_class, (t_method)glance_program_compile,
                    gensym("compile"), 0);
}