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

static void *gl_vertexarray_new(void) {
    t_gl_vertexarray_obj *obj = NULL;
    obj = (t_gl_vertexarray_obj *)pd_new(gl_vertexarray_class);
    glGenBuffers(1, &obj->arraybuffer);
    obj->attribindex = 0;
    obj->normalized = false;
    obj->pointer = NULL;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
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

static void gl_vertexarray_setattribloc(t_gl_vertexarray_obj *obj, t_float loc) {
    obj->attribindex = loc;
}

static void gl_vertexarray_setnormalized(t_gl_vertexarray_obj *obj, t_float val) {
    obj->normalized = (val != 0.0);
}

static void gl_vertexarray_readarray(t_gl_vertexarray_obj *obj,
                                t_symbol *arrayname, t_symbol *readmode) {
    GLenum glreadmode = GL_STATIC_COPY;
    if (readmode == gensym("STATIC_DRAW")) {
        glreadmode = GL_STATIC_DRAW;
    } else if (readmode == gensym("DYNAMIC_COPY")) {
        glreadmode = GL_DYNAMIC_COPY;
    } else if (readmode == gensym("DYNAMIC_DRAW")) {
        glreadmode = GL_DYNAMIC_DRAW;
    } else if (readmode == gensym("STREAM_COPY")) {
        glreadmode = GL_STREAM_COPY;
    } else if (readmode == gensym("STREAM_DRAW")) {
        glreadmode = GL_STREAM_DRAW;
    } else if (readmode == gensym("STREAM_COPY")) {
        glreadmode = GL_STREAM_COPY;
    }
    t_garray *ga = (t_garray *)pd_findbyclass(arrayname, garray_class);
    if (obj->pointer) {
        
    }
    garray_getfloatarray(ga, &obj->size, &obj->pointer);
    glBindBuffer(GL_ARRAY_BUFFER, obj->arraybuffer);
    glBufferData(GL_ARRAY_BUFFER, obj->size, obj->pointer, glreadmode);
}


void gl_vertexarray_setup(void) {
    gl_vertexarray_class = class_new(gensym("gl.vertexarray"),
                                     (t_newmethod)gl_vertexarray_new, 0,
                                     sizeof(t_gl_vertexarray_obj),
                                     CLASS_DEFAULT, 0);
    
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_render,
                    render, A_GIMME, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_readarray,
                    gensym("readarray"), A_SYMBOL, A_SYMBOL, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_setattribloc,
                    gensym("attribloc"), A_FLOAT, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_setnormalized,
                    gensym("normalized"), A_FLOAT, 0);
}
