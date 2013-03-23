//
//  glvertexarray.c
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

static t_class *gl_vertexarray_class;

typedef struct _gl_vertexarray_obj {
    t_object x_obj;
    GLuint arraybuffer;
    GLuint attribindex;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    float * pointer;
    t_outlet *out;
} t_gl_vertexarray_obj;

static t_sym_uint_list gl_arrayread_modes[] = {
    {"STATIC_COPY", GL_STATIC_COPY},
    {"STATIC_DRAW", GL_STATIC_DRAW},
    {"STATIC_READ", GL_STATIC_READ},
    {"DYNAMIC_COPY", GL_DYNAMIC_COPY},
    {"DYNAMIC_DRAW", GL_DYNAMIC_DRAW},
    {"DYNAMIC_READ", GL_DYNAMIC_READ},
    {"STREAM_COPY", GL_STREAM_COPY},
    {"STREAM_DRAW", GL_STREAM_DRAW},
    {"STREAM_READ", GL_STREAM_READ},
    {0,0}
};

static void *gl_vertexarray_new(t_symbol *readmode, t_float components, t_float flength) {
    t_gl_vertexarray_obj *obj = NULL;
    obj = (t_gl_vertexarray_obj *)pd_new(gl_vertexarray_class);
    glGenBuffers(1, &obj->arraybuffer);
    GLenum glreadmode = GL_STATIC_COPY;
    find_uint_for_sym(gl_arrayread_modes, readmode, &glreadmode);
    obj->attribindex = 0;
    obj->normalized = false;
    obj->size = sizeof(float)*(unsigned)components*(unsigned)flength;
    obj->stride = components;
    obj->pointer = getbytes(obj->size);
    int prevboundbuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevboundbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, obj->arraybuffer);
    glBufferData(GL_ARRAY_BUFFER, obj->size, obj->pointer, glreadmode);
    glBindBuffer(GL_ARRAY_BUFFER, prevboundbuffer);
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void gl_vertexarray_destroy(t_gl_vertexarray_obj *obj) {
    glDeleteBuffers(1, &obj->arraybuffer);
    outlet_free(obj->out);
    freebytes(obj->pointer, obj->size);
}

static void gl_vertexarray_render(t_gl_vertexarray_obj *obj,
                                  t_symbol *s, int argc, t_atom *argv) {
    glBindBuffer(GL_ARRAY_BUFFER, obj->arraybuffer);
    glEnableVertexAttribArray(obj->attribindex);
    glVertexAttribPointer(GL_ARRAY_BUFFER,
                          obj->size, obj->type, obj->normalized,
                          obj->stride, obj->pointer);
    outlet_anything(obj->out, s, argc, argv);
}

static void gl_vertexarray_set(t_gl_vertexarray_obj *obj,
                               t_symbol *sym, int argc, t_atom *argv) {
    if (argc>1) {
        float *ptr = glMapBufferRange(GL_ARRAY_BUFFER, (unsigned)atom_getfloat(argv),
                                      sizeof(float)*(argc-1), GL_MAP_INVALIDATE_RANGE_BIT);
        int coord = argc-1;
        for (int i = 0; i<coord; i++) {
            ptr[i] = atom_getfloat(argv+i);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

static void gl_vertexarray_setattribloc(t_gl_vertexarray_obj *obj, t_float loc) {
    obj->attribindex = loc;
}

static void gl_vertexarray_setnormalized(t_gl_vertexarray_obj *obj, t_float val) {
    obj->normalized = (val != 0.0);
}

static void gl_vertexarray_readptr(t_gl_vertexarray_obj *obj,
                                   t_float offsetinto, t_float length,
                                   float *fptr) {
    GLint prevboundbuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevboundbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, obj->arraybuffer);
    glBufferSubData(GL_ARRAY_BUFFER, (int)offsetinto, ((unsigned)length)*sizeof(float), fptr);
    glBindBuffer(GL_ARRAY_BUFFER, prevboundbuffer);
}

static void gl_vertexarray_readarray(t_gl_vertexarray_obj *obj,
                                t_symbol *arrayname, t_float offsetinto) {
    t_garray *ga = (t_garray *)pd_findbyclass(arrayname, garray_class);
    int dsize;
    t_float *pointer;
    garray_getfloatarray(ga, &dsize, &pointer);
    GLint prevboundbuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prevboundbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, obj->arraybuffer);
    glBufferSubData(GL_ARRAY_BUFFER, (int)offsetinto, dsize, pointer);
    glBindBuffer(GL_ARRAY_BUFFER, prevboundbuffer);
}


void gl_vertexarray_setup(void) {
    gl_vertexarray_class = class_new(gensym("gl.vertexarray"),
                                     (t_newmethod)gl_vertexarray_new, 0,
                                     sizeof(t_gl_vertexarray_obj),
                                     CLASS_DEFAULT,
                                     A_SYMBOL, A_FLOAT, 0);
    
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_render,
                    render, A_GIMME, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_set,
                    gensym("set"), A_GIMME, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_readarray,
                    gensym("readarray"), A_SYMBOL, A_SYMBOL, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_readptr,
                    gensym("readptr"), A_FLOAT, A_FLOAT, A_POINTER, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_setattribloc,
                    gensym("attribloc"), A_FLOAT, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_setnormalized,
                    gensym("normalized"), A_FLOAT, 0);
    
}
