//
//  glbuffer.c
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


static t_class *gl_buffer_class;
typedef struct _gl_buffer {
    t_object x_obj;
    GLuint buffer_id;
    GLenum target;
    t_float *f_buffer;
    size_t f_buf_sz;
    t_outlet *out;
} t_gl_buffer;

static t_sym_uint_list gl_buffer_target_list[] = {
    {"ARRAY", GL_ARRAY_BUFFER},
    {"COPY_READ", GL_COPY_READ_BUFFER},
    {"COPY_WRITE", GL_COPY_WRITE_BUFFER},
    {"ELEMENT_ARRAY", GL_ELEMENT_ARRAY_BUFFER},
    {"PIXEL_PACK", GL_PIXEL_PACK_BUFFER},
    {"PIXEL_UNPACK", GL_PIXEL_UNPACK_BUFFER},
    {"TEXTURE", GL_TEXTURE_BUFFER},
    {"TRANSFORM_FEEDBACK", GL_TRANSFORM_FEEDBACK_BUFFER},
    {"UNIFORM", GL_UNIFORM_BUFFER},
    {0,0}
};

static void *gl_buffer_new(t_symbol *starget, t_float components, t_float flength) {
    t_gl_buffer *buf = NULL;
    GLenum target;
    if (find_uint_for_sym(gl_buffer_target_list, starget, &target)) {
        buf = (t_gl_buffer *)pd_new(gl_buffer_class);
        glGenBuffers(1, &buf->buffer_id);
        buf->out = outlet_new(&buf->x_obj, &s_anything);
        buf->target = target;
    }
    return (void *)buf;
}

static void gl_buffer_free(t_gl_buffer *buf) {
    glDeleteBuffers(1, &buf->buffer_id);
    outlet_free(buf->out);
}

static void gl_buffer_use(t_gl_buffer *buf) {
    glBindBuffer(buf->target, buf->buffer_id);
}

static void gl_buffer_render(t_gl_buffer *buf, t_symbol *s, int argc, t_atom *argv) {
    
    glBindBuffer(buf->target, buf->buffer_id);
    outlet_anything(buf->out, s, argc, argv);
}

void gl_buffer_setup(void) {
    
    gl_buffer_class = class_new(gensym("gl.buffer"),
                                (t_newmethod)gl_buffer_new,
                                (t_method)gl_buffer_free,
                                sizeof(t_gl_buffer), CLASS_DEFAULT,
                                A_SYMBOL, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_buffer_class, (t_method)gl_buffer_render,
                    render, A_GIMME, 0);
}