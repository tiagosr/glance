//
//  gluniform.c
//  glance
//
//  Created by Tiago Rezende on 3/16/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include "glance.h"

static t_class
    *gl_uniform1f_class = NULL,
    *gl_uniform2f_class = NULL,
    *gl_uniform3f_class = NULL,
    *gl_uniform4f_class = NULL;

/*
 * pd object types
 */
typedef struct _gl_uniform1f {
    t_object x_obj;
    GLint location; // location index for program
    t_symbol *name; // symbol to query location for
    t_float value; // only one float
    t_inlet *in_val; // only one extra input
    t_outlet *out; // outlet to continue render chain
} t_gl_uniform1f;

typedef struct _gl_uniform2f {
    t_object x_obj;
    GLint location;
    t_symbol *name;
    t_float value[2]; // two floats for uniform
    t_inlet *in_val[2]; // two extra inputs, no need to send data through lists
    t_outlet *out;
} t_gl_uniform2f;

typedef struct _gl_uniform3f {
    t_object x_obj;
    GLint location;
    t_symbol *name;
    t_float value[3];
    t_inlet *in_val[3];
    t_outlet *out;
} t_gl_uniform3f;

typedef struct _gl_uniform4f {
    t_object x_obj;
    GLint location;
    t_symbol *name;
    t_float value[4];
    t_inlet *in_val[4];
    t_outlet *out;
} t_gl_uniform4f;



static void *gl_uniform1f_new(t_symbol *unifname) {
    t_gl_uniform1f *obj = (t_gl_uniform1f *)pd_new(gl_uniform1f_class);
    obj->location = -1;
    obj->value = 0.0;
    obj->name = unifname;
    obj->in_val = floatinlet_new(&obj->x_obj, &obj->value);
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}
static void gl_uniform1f_destroy(t_gl_uniform1f *obj) {
    inlet_free(obj->in_val);
}

static void *gl_uniform2f_new(t_symbol *unifname) {
    t_gl_uniform2f *obj = (t_gl_uniform2f *)pd_new(gl_uniform2f_class);
    obj->location = -1;
    obj->value[0] = 0.0;
    obj->value[1] = 0.0;
    obj->name = unifname;
    obj->in_val[0] = floatinlet_new(&obj->x_obj, &obj->value[0]);
    obj->in_val[1] = floatinlet_new(&obj->x_obj, &obj->value[1]);
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}
static void gl_uniform2f_destroy(t_gl_uniform2f *obj) {
    inlet_free(obj->in_val[0]);
    inlet_free(obj->in_val[1]);
}

static void *gl_uniform3f_new(t_symbol *unifname) {
    t_gl_uniform3f *obj = (t_gl_uniform3f *)pd_new(gl_uniform3f_class);
    obj->location = -1;
    obj->value[0] = 0.0;
    obj->value[1] = 0.0;
    obj->value[2] = 0.0;
    obj->name = unifname;
    obj->in_val[0] = floatinlet_new(&obj->x_obj, &obj->value[0]);
    obj->in_val[1] = floatinlet_new(&obj->x_obj, &obj->value[1]);
    obj->in_val[2] = floatinlet_new(&obj->x_obj, &obj->value[2]);
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void gl_uniform3f_destroy(t_gl_uniform3f *obj) {
    inlet_free(obj->in_val[0]);
    inlet_free(obj->in_val[1]);
    inlet_free(obj->in_val[2]);
}

static void *gl_uniform4f_new(t_symbol *unifname) {
    t_gl_uniform4f *obj = (t_gl_uniform4f *)pd_new(gl_uniform4f_class);
    obj->location = -1;
    obj->value[0] = 0.0;
    obj->value[1] = 0.0;
    obj->value[2] = 0.0;
    obj->value[3] = 0.0;
    obj->name = unifname;
    obj->in_val[0] = floatinlet_new(&obj->x_obj, &obj->value[0]);
    obj->in_val[1] = floatinlet_new(&obj->x_obj, &obj->value[1]);
    obj->in_val[2] = floatinlet_new(&obj->x_obj, &obj->value[2]);
    obj->in_val[3] = floatinlet_new(&obj->x_obj, &obj->value[3]);
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void gl_uniform4f_destroy(t_gl_uniform4f *obj) {
    inlet_free(obj->in_val[0]);
    inlet_free(obj->in_val[1]);
    inlet_free(obj->in_val[2]);
    inlet_free(obj->in_val[3]);
}



static void gl_uniform1f_float(t_gl_uniform1f *obj, t_float v) {
    obj->value = v;
}
static void gl_uniform1f_setname(t_gl_uniform1f *obj, t_symbol *sym) {
    obj->location = -1; obj->name = sym;
}

static void gl_uniform2f_vec(t_gl_uniform2f *obj, t_float v0, t_float v1) {
    obj->value[0] = v0; obj->value[1] = v1;
}
static void gl_uniform2f_setname(t_gl_uniform2f *obj, t_symbol *sym) {
    obj->location = -1; obj->name = sym;
}

static void gl_uniform3f_vec(t_gl_uniform3f *obj,
                             t_float v0, t_float v1, t_float v2) {
    obj->value[0] = v0; obj->value[1] = v1; obj->value[2] = v2;
}
static void gl_uniform3f_setname(t_gl_uniform3f *obj, t_symbol *sym) {
    obj->location = -1; obj->name = sym;
}

static void gl_uniform4f_vec(t_gl_uniform4f *obj,
                             t_float v0, t_float v1, t_float v2, t_float v3) {
    obj->value[0] = v0; obj->value[1] = v1; obj->value[2] = v2; obj->value[3] = v3;
}
static void gl_uniform4f_setname(t_gl_uniform4f *obj, t_symbol *sym) {
    obj->location = -1; obj->name = sym;
}



static void gl_uniform1f_render(t_gl_uniform1f *obj,
                                t_symbol *s, int argc, t_atom *argv) {
    GLint program;
    if (obj->location == -1) {
        glGetIntegeri_v(GL_CURRENT_PROGRAM, 0, &program);
        obj->location = glGetUniformLocation(program, obj->name->s_name);
    }
    glUniform1f(obj->location, obj->value);
    outlet_anything(obj->out, s, argc, argv);
}

static void gl_uniform2f_render(t_gl_uniform2f *obj,
                                t_symbol *s, int argc, t_atom *argv) {
    GLint program;
    if (obj->location == -1) {
        glGetIntegeri_v(GL_CURRENT_PROGRAM, 0, &program);
        obj->location = glGetUniformLocation(program, obj->name->s_name);
    }
    glUniform2f(obj->location, obj->value[0], obj->value[1]);
    outlet_anything(obj->out, s, argc, argv);
}
static void gl_uniform3f_render(t_gl_uniform3f *obj,
                                t_symbol *s, int argc, t_atom *argv) {
    GLint program;
    if (obj->location == -1) {
        glGetIntegeri_v(GL_CURRENT_PROGRAM, 0, &program);
        obj->location = glGetUniformLocation(program, obj->name->s_name);
    }
    glUniform3f(obj->location, obj->value[0], obj->value[1], obj->value[2]);
    outlet_anything(obj->out, s, argc, argv);
}

static void gl_uniform4f_render(t_gl_uniform4f *obj,
                                t_symbol *s, int argc, t_atom *argv) {
    GLint program;
    if (obj->location == -1) {
        glGetIntegeri_v(GL_CURRENT_PROGRAM, 0, &program);
        obj->location = glGetUniformLocation(program, obj->name->s_name);
    }
    glUniform4f(obj->location,
                obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
    outlet_anything(obj->out, s, argc, argv);
}



void gl_uniform_setup(void) {
    gl_uniform1f_class = class_new(gensym("gl.uniform1f"),
                                   (t_newmethod)gl_uniform1f_new,
                                   (t_method)gl_uniform1f_destroy,
                                   sizeof(t_gl_uniform1f), CLASS_DEFAULT,
                                   A_SYMBOL, 0);
    class_addfloat(gl_uniform1f_class, gl_uniform1f_float);
    class_addmethod(gl_uniform1f_class, (t_method)gl_uniform1f_setname,
                    gensym("name"), A_SYMBOL, 0);
    class_addmethod(gl_uniform1f_class, (t_method)gl_uniform1f_render,
                    render, A_GIMME, 0);
    
    gl_uniform2f_class = class_new(gensym("gl.uniform2f"),
                                   (t_newmethod)gl_uniform2f_new,
                                   (t_method)gl_uniform2f_destroy,
                                   sizeof(t_gl_uniform2f), CLASS_DEFAULT,
                                   A_SYMBOL, 0);
    class_addmethod(gl_uniform2f_class, (t_method)gl_uniform2f_vec,
                    gensym("vec"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform2f_class, (t_method)gl_uniform2f_setname,
                    gensym("name"), A_SYMBOL, 0);
    class_addmethod(gl_uniform2f_class, (t_method)gl_uniform2f_render,
                    render, A_GIMME, 0);

    gl_uniform3f_class = class_new(gensym("gl.uniform3f"),
                                   (t_newmethod)gl_uniform3f_new,
                                   (t_method)gl_uniform3f_destroy,
                                   sizeof(t_gl_uniform3f), CLASS_DEFAULT,
                                   A_SYMBOL, 0);
    class_addmethod(gl_uniform3f_class, (t_method)gl_uniform3f_vec,
                    gensym("vec"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform3f_class, (t_method)gl_uniform3f_setname,
                    gensym("name"), A_SYMBOL, 0);
    class_addmethod(gl_uniform3f_class, (t_method)gl_uniform3f_render,
                    render, A_GIMME, 0);

    gl_uniform4f_class = class_new(gensym("gl.uniform4f"),
                                   (t_newmethod)gl_uniform4f_new,
                                   (t_method)gl_uniform4f_destroy,
                                   sizeof(t_gl_uniform4f), CLASS_DEFAULT,
                                   A_SYMBOL, 0);
    class_addmethod(gl_uniform4f_class, (t_method)gl_uniform4f_vec,
                    gensym("vec"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform4f_class, (t_method)gl_uniform4f_setname,
                    gensym("name"), A_SYMBOL, 0);
    class_addmethod(gl_uniform4f_class, (t_method)gl_uniform4f_render,
                    render, A_GIMME, 0);
}

void gl_uniform1f_setup(void) { gl_uniform_setup(); }
void gl_uniform2f_setup(void) { gl_uniform_setup(); }
void gl_uniform3f_setup(void) { gl_uniform_setup(); }
void gl_uniform4f_setup(void) { gl_uniform_setup(); }