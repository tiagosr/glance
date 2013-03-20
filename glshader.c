//
//  glshader.c
//  glance
//
//  Created by Tiago Rezende on 3/7/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include "glance.h"

static t_class *glshader_class;
struct _glshader_obj {
    t_object x_obj;
    GLuint shader;
    GLuint program;
    GLint compiled, linked;
    t_outlet *data_out;
    t_outlet *info_out;
};
typedef struct _glshader_obj glshader_obj;

static t_symbol *vtx, *frag, *geom;
static t_symbol *compiled, *linked;
static void *glshader_new(void) {
    glshader_obj *obj = (glshader_obj *)pd_new(glshader_class);
    obj->shader = 0;
    obj->program = glCreateProgram();
    obj->compiled = false;
    obj->linked = false;
    obj->data_out = outlet_new(&obj->x_obj, &s_list);
    obj->info_out = outlet_new(&obj->x_obj, &s_list);
    return (void *)obj;
}

static void glshader_load(glshader_obj *obj, t_symbol *sym, int argc, t_atom *argv) {
    if (argc > 0) {
        char **txts = alloca(sizeof(char *)*argc);
        int *lens = alloca(sizeof(int)*argc);
        for (int c = 0; c < argc; c++) {
            t_symbol *filename_s = atom_getsymbolarg(c, argc, argv);
            FILE *f = fopen(filename_s->s_name, "r");
            size_t len = ftell(f);
            char *txt = alloca(len);
            fread(txt, len, 1, f);
            fclose(f);
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
            glAttachShader(obj->program, shader);
            t_atom compiled_atoms[2];
            SETSYMBOL(compiled_atoms+0, compiled);
            SETFLOAT(compiled_atoms+1, shader);
            outlet_list(obj->data_out, &s_list, 2, compiled_atoms);
        } else {
            
        }
    }
}
static void glshader_link(glshader_obj *obj) {
    glLinkProgram(obj->program);
    glGetProgramiv(obj->program, GL_COMPILE_STATUS, &obj->linked);
    if (obj->linked) {
        t_atom linked_atoms[2];
        SETSYMBOL(linked_atoms+0, linked);
        SETFLOAT(linked_atoms+1, obj->program);
        outlet_list(obj->data_out, &s_list, 2, linked_atoms);
    } else {
        
    }
}

static void glshader_use(glshader_obj *obj) {
    if (obj->linked) {
        glUseProgram(obj->program);
    }
}

static void glshader_glance_render(glshader_obj *obj, t_symbol *s, int argc, t_atom *argv) {
    glshader_use(obj);
    outlet_anything(obj->data_out, s, argc, argv);
}

void gl_shader_setup(void) {
    vtx = gensym("VERTEX");
    frag = gensym("FRAGMENT");
    geom = gensym("GEOMETRY");
    glshader_class = class_new(gensym("gl.shader"),
                               (t_newmethod)glshader_new,
                               0,
                               sizeof(glshader_obj),
                               CLASS_DEFAULT,
                               0);
    class_addmethod(glshader_class, (t_method)glshader_load,
                    vtx, A_GIMME, 0);
    class_addmethod(glshader_class, (t_method)glshader_load,
                    frag, A_GIMME, 0);
    class_addmethod(glshader_class, (t_method)glshader_load,
                    geom, A_GIMME, 0);
    
    class_addmethod(glshader_class, (t_method)glshader_link,
                    gensym("link"),0);
    
    class_addmethod(glshader_class, (t_method)glshader_glance_render,
                    render, A_GIMME, 0);
}
