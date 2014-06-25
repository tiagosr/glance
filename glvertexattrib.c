//
//  glvertexattrib.c
//  glance
//
//  Created by Tiago Rezende on 6/25/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <stdbool.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"

static t_class *gl_vertexattrib_class = NULL;
typedef struct _t_glvertexattrib t_glvertexattrib;
struct _t_glvertexattrib {
    t_object x_obj;
    bool reset_attribs;
    GLuint currentattribindex;
    GLuint attribindex;
    GLsizei attribsize;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    void* offset;
    t_outlet *out_msg;
};

static t_sym_uint_list gl_types[] = {
    {"FLOAT", GL_FLOAT},
    {"DOUBLE", GL_DOUBLE},
    {"BYTE", GL_BYTE},
    {"UNSIGNED_BYTE", GL_UNSIGNED_BYTE},
    {"SHORT", GL_SHORT},
    {"UNSIGNED_SHORT", GL_UNSIGNED_SHORT},
    {"INT", GL_INT},
    {"UNSIGNED_INT", GL_UNSIGNED_INT},
    {0,0}
};


static t_glvertexattrib *gl_vertexattrib_new(t_symbol *sym, int argc, t_atom *argv) {
    if (argc >= 3) {
        t_glvertexattrib *va = (t_glvertexattrib *)pd_new(gl_vertexattrib_class);
        va->out_msg = outlet_new(&va->x_obj, gensym("out"));
        va->reset_attribs = true;
        va->normalized = GL_FALSE;
        va->stride = 0;
        va->offset = 0;
        va->attribindex = atom_getfloat(argv);
        va->attribsize = atom_getfloat(argv+1);
        find_uint_for_sym(gl_types, atom_getsymbol(argv+2), &va->type);
        if (argc >= 4) {
            va->normalized = atom_getfloat(argv+3);
            if (argc >= 5) {
                va->stride = atom_getfloat(argv+4);
                if (argc >= 6) {
                    va->offset = (void *)((int)atom_getfloat(argv+5));
                }
            }
        }
        return va;
    } else {
        error("not enough arguments for gl.vertexattrib");
        return NULL;
    }
}

static void gl_vertexattrib_free(t_glvertexattrib *va) {
    outlet_free(va->out_msg);
}

static void gl_vertexattrib_reset_attribs(t_glvertexattrib *va) {
    if (va->currentattribindex != va->attribindex) {
        glDisableVertexAttribArray(va->currentattribindex);
        va->currentattribindex = va->attribindex;
    }
    glEnableVertexAttribArray(va->currentattribindex);
    glVertexAttribPointer(va->attribindex, va->attribsize,
                          va->type, va->normalized, va->stride, va->offset);
    va->reset_attribs = false;
}

static void gl_vertexattrib_render(t_glvertexattrib *va, t_symbol *sym, int argc, t_atom *argv) {
    if (va->reset_attribs) {
        gl_vertexattrib_reset_attribs(va);
    }
    glEnableVertexAttribArray(va->currentattribindex);
    outlet_anything(va->out_msg, render, argc, argv);
}


void gl_vertexattrib_setup(void) {
    gl_vertexattrib_class = class_new(gensym("gl.vertexattrib"),
                                      (t_newmethod)gl_vertexattrib_new,
                                      (t_method)gl_vertexattrib_free,
                                      sizeof(t_glvertexattrib),
                                      CLASS_DEFAULT, A_GIMME, 0);
    
    
}