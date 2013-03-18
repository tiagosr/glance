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



static void *gl_buffer_new(t_symbol *starget) {
    t_gl_buffer *buf = NULL;
    bool create = true;
    GLenum target;
    if (starget == gensym("array")) {
        target = GL_ARRAY_BUFFER;
    } else if (starget == gensym("copy-read")) {
        target = GL_COPY_READ_BUFFER;
    } else if (starget == gensym("copy-write")) {
        target = GL_COPY_WRITE_BUFFER;
    } else if (starget == gensym("element-array")) {
        target = GL_ELEMENT_ARRAY_BUFFER;
    } else if (starget == gensym("pixel-pack")) {
        target = GL_PIXEL_PACK_BUFFER;
    } else if (starget == gensym("pixel-unpack")) {
        target = GL_PIXEL_UNPACK_BUFFER;
    } else if (starget == gensym("texture")) {
        target = GL_TEXTURE_BUFFER;
    } else if (starget == gensym("transform-feedback")) {
        target = GL_TRANSFORM_FEEDBACK_BUFFER;
    } else if (starget == gensym("uniform")) {
        target = GL_UNIFORM_BUFFER;
    } else {
        create = false;
    }
    if (create) {
        buf = (t_gl_buffer *)pd_new(gl_buffer_class);
        glGenBuffers(1, &buf->buffer_id);
        buf->out = outlet_new(&buf->x_obj, &s_anything);
        buf->target = target;
    }
    return (void *)buf;
}

static void gl_buffer_free(t_gl_buffer *buf) {
    glDeleteBuffers(1, &buf->buffer_id);
}

static void gl_buffer_use(t_gl_buffer *buf) {
    glBindBuffer(buf->target, buf->buffer_id);
}

static void gl_buffer_render(t_gl_buffer *buf, t_symbol *s, int argc, t_atom *argv) {
    gl_buffer_use(buf);
    outlet_anything(buf->out, s, argc, argv);
}

void gl_buffer_setup(void) {
    
    gl_buffer_class = class_new(gensym("gl.buffer"),
                                (t_newmethod)gl_buffer_new,
                                (t_method)gl_buffer_free,
                                sizeof(t_gl_buffer), CLASS_DEFAULT,
                                0);
}