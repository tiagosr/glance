//
//  glshader.c
//  glance
//
//  Created by Tiago Rezende on 3/7/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"
#include "window.hh"

static t_class *glprogram_class;
struct glprogram_obj {
    t_object x_obj;
    t_canvas *x_canvas;
    GLuint shader;
    std::shared_ptr<glwindow_program> program;
    GLint compiled, linked;
    t_outlet *data_out;
    t_outlet *info_out;
};

glwindow_program::glwindow_program() {
    program = glCreateProgram();
}

glwindow_program::~glwindow_program() {
    glDeleteProgram(program);
}



static t_symbol *vtx, *frag, *geom;
static t_symbol *compiled, *linked;
static void *glprogram_new(t_symbol *name) {
    glprogram_obj *obj = (glprogram_obj *)pd_new(glprogram_class);
    obj->x_canvas = canvas_getcurrent();
    obj->shader = 0;
    obj->program = glwindow_program::programs.get_create(name);
    obj->compiled = false;
    obj->linked = false;
    obj->data_out = outlet_new(&obj->x_obj, &s_list);
    obj->info_out = outlet_new(&obj->x_obj, &s_list);
    return (void *)obj;
}

static void glprogram_load(glprogram_obj *obj, t_symbol *sym, int argc, t_atom *argv) {
    if (argc > 0) {
        char **txts = (char **)alloca(sizeof(char *)*argc);
        int *lens = (int *)alloca(sizeof(int)*argc);
        for (int c = 0; c < argc; c++) {
            t_symbol *filename_s = atom_getsymbolarg(c, argc, argv);
            char *fname = NULL;
            asprintf(&fname, "%s/%s",canvas_getdir(obj->x_canvas)->s_name, filename_s->s_name);
            FILE *f = fopen(fname, "r");
            free(fname);
            size_t len = 0;
            char *txt = NULL;
            if (f) {
                len = ftell(f);
                txt = (char *)alloca(len);
                fread(txt, len, 1, f);
                fclose(f);
            } else {
                error("file %s could not be opened", filename_s->s_name);
            }
            txts[c] = txt;
            lens[c] = len;
        }
        GLenum t = (sym==vtx?GL_VERTEX_SHADER:
                    (sym==frag?GL_FRAGMENT_SHADER:
                     (sym==geom)?GL_GEOMETRY_SHADER:-1));
        GLuint shader = 0;
        if(t!=-1) {
            shader = glCreateShader(t);
        }
        obj->shader = shader;
        glShaderSource(shader, 1, (const GLchar **)txts, lens);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &obj->compiled);
        if (obj->compiled) {
            glAttachShader(obj->program->program, shader);
            t_atom compiled_atoms[2];
            SETSYMBOL(compiled_atoms+0, compiled);
            SETFLOAT(compiled_atoms+1, shader);
            outlet_list(obj->info_out, &s_list, 2, compiled_atoms);
        } else {
            error("shaders were not compiled");
            
        }
    }
}
static void glprogram_link(glprogram_obj *obj) {
    glLinkProgram(obj->program->program);
    glGetProgramiv(obj->program->program, GL_COMPILE_STATUS, &obj->linked);
    if (obj->linked) {
        t_atom linked_atoms[2];
        SETSYMBOL(linked_atoms+0, linked);
        SETFLOAT(linked_atoms+1, obj->program->program);
        outlet_list(obj->data_out, &s_list, 2, linked_atoms);
    } else {
        error("fail on shader linkage");
    }
}

static void glprogram_use(glprogram_obj *obj) {
    if (obj->linked) {
        glUseProgram(obj->program->program);
    }
}

static void glprogram_glance_render(glprogram_obj *obj, t_symbol *s, int argc, t_atom *argv) {
    glprogram_use(obj);
    outlet_anything(obj->data_out, s, argc, argv);
}

void gl_program_setup(void) {
    vtx = gensym("vertex");
    frag = gensym("fragment");
    geom = gensym("geometry");
    glprogram_class = class_new(gensym("gl.program"),
                               (t_newmethod)glprogram_new,
                               0,
                               sizeof(glprogram_obj),
                               CLASS_DEFAULT,
                               A_SYMBOL, 0);
    class_addmethod(glprogram_class, (t_method)glprogram_load,
                    vtx, A_GIMME, 0);
    class_addmethod(glprogram_class, (t_method)glprogram_load,
                    frag, A_GIMME, 0);
    class_addmethod(glprogram_class, (t_method)glprogram_load,
                    geom, A_GIMME, 0);
    
    class_addmethod(glprogram_class, (t_method)glprogram_link,
                    gensym("link"), A_NULL, 0);
    
    
    class_addmethod(glprogram_class, (t_method)glprogram_glance_render,
                    render, A_GIMME, 0);
}
