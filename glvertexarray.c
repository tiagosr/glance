//
//  glvertexarray.c
//  glance
//
//  Created by Tiago Rezende on 3/16/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <stdbool.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"

static t_class *gl_vertexarray_class;

typedef struct _gl_vertexarray_obj {
    t_object x_obj;
    GLuint vertexarray;
    GLuint attribindex;
    GLint size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    int vtx_comps, vtx_offset;
    int col_comps, col_offset;
    int nml_comps, nml_offset;
    bool init;
    t_outlet *out;
} t_gl_vertexarray_obj;


static void *gl_vertexarray_new(t_float position, t_float components, t_float flength) {
    t_gl_vertexarray_obj *obj = NULL;
    obj = (t_gl_vertexarray_obj *)pd_new(gl_vertexarray_class);
    obj->init = false;
    obj->attribindex = position;
    obj->normalized = false;
    obj->size = sizeof(float)*(unsigned)components*(unsigned)flength;
    obj->stride = components*sizeof(float);
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}
#define GLWRAP(x) {GLenum err; x; while((err = glGetError())!=GL_NO_ERROR){ post("" #x  " -> error %d on line %d", err, __LINE__); } };
static void gl_vertexarray_init(t_gl_vertexarray_obj *obj) {
    GLWRAP(glGenVertexArrays(1, &obj->vertexarray))
    //int prevboundvtxarray = 0, prevboundvtxattribenabled = 0;
    //glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prevboundvtxarray);
    //glGetVertexAttribiv(obj->attribindex, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &prevboundvtxattribenabled);
    {GLenum err;
    glBindVertexArray(obj->vertexarray);
    while ((err = glGetError())!= GL_NO_ERROR) {
        post("error %d on line %d: vertexarray=%d", err, __LINE__, obj->vertexarray);
    }
    }
    GLWRAP(glEnableVertexAttribArray(obj->attribindex));
    GLWRAP(
    glVertexAttribPointer(obj->attribindex,
                          obj->size, obj->type, obj->normalized,
                          obj->stride, 0)
           );
    obj->init = true;
}

static void gl_vertexarray_destroy(t_gl_vertexarray_obj *obj) {
    if (obj->init) {
        glDeleteVertexArrays(1, &obj->vertexarray);
    }
    outlet_free(obj->out);
    //freebytes(obj->pointer, obj->size);
}

static void gl_vertexarray_render(t_gl_vertexarray_obj *obj,
                                  t_symbol *s, int argc, t_atom *argv) {
    /*
    if (!obj->init) {
        gl_vertexarray_init(obj);
    }
    */
    GLWRAP(glEnableVertexAttribArray(obj->attribindex));
    /*
    GLWRAP(glBindVertexArray(obj->vertexarray));
    GLWRAP(glVertexAttribPointer(obj->attribindex,
                          obj->size, obj->type, obj->normalized,
                          obj->stride, 0));
     
    // */
    outlet_anything(obj->out, s, argc, argv);
    
    glDisableVertexAttribArray(obj->attribindex);
}

static void gl_vertexarray_setattribloc(t_gl_vertexarray_obj *obj, t_float loc) {
    obj->attribindex = loc;
}

static void gl_vertexarray_setnormalized(t_gl_vertexarray_obj *obj, t_float val) {
    obj->normalized = (val != 0.0);
}



void gl_vertexarray_setup(void) {
    gl_vertexarray_class = class_new(gensym("gl.vertexarray"),
                                     (t_newmethod)gl_vertexarray_new, 0,
                                     sizeof(t_gl_vertexarray_obj),
                                     CLASS_DEFAULT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_init, reset,
                    A_NULL, 0);
    class_addmethod(gl_vertexarray_class, (t_method)gl_vertexarray_render,
                    render, A_GIMME, 0);
}
