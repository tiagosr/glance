//
//  glarraybuffer.c
//  glance
//
//  Created by Tiago Rezende on 1/19/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#include "window.hh"

static t_class * gl_array_buffer_class = NULL;

struct t_gl_array_buffer_obj {
    t_object x_obj;
    t_outlet *out;
    std::shared_ptr<glwindow_buffer> arraybuffer;
    GLsizeiptr size;
    t_symbol *array;
    float *data;
    GLenum usage;
    bool stream;
    bool read;
};

glwindow_buffer::glwindow_buffer() {
    glGenBuffers(1, &buffer);
}

glwindow_buffer::~glwindow_buffer() {
    glDeleteBuffers(1, &buffer);
}

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
    
    {"static_copy", GL_STATIC_COPY},
    {"static_draw", GL_STATIC_DRAW},
    {"static_read", GL_STATIC_READ},
    {"dynamic_copy", GL_DYNAMIC_COPY},
    {"dynamic_draw", GL_DYNAMIC_DRAW},
    {"dynamic_read", GL_DYNAMIC_READ},
    {"stream_copy", GL_STREAM_COPY},
    {"stream_draw", GL_STREAM_DRAW},
    {"stream_read", GL_STREAM_READ},
    {0,0}
};


static void* gl_array_buffer_new(t_symbol *name, t_symbol *array, t_symbol *usage) {
    t_gl_array_buffer_obj *obj = (t_gl_array_buffer_obj *)pd_new(gl_array_buffer_class);
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    obj->arraybuffer = glwindow_buffer::buffers.get_create(name);
    find_uint_for_sym(gl_arrayread_modes, usage, &obj->usage);
    obj->array = array;
    return obj;
}
static void gl_array_buffer_free(t_gl_array_buffer_obj *obj) {
    outlet_free(obj->out);
}

static void gl_array_buffer_render(t_gl_array_buffer_obj *obj,
                                   t_symbol *msg, int argc, t_atom *argv) {
    GLint old_buffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &old_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, obj->arraybuffer->buffer);
    if ((obj->stream && !obj->read) || !obj->data) {
        t_garray *garray = (t_garray *)pd_findbyclass(obj->array, garray_class);
        GLsizeiptr size = garray_npoints(garray)*sizeof(float);
        int isize = 0;
        garray_getfloatarray(garray, &isize, &obj->data);
        glBufferData(GL_ARRAY_BUFFER, size, obj->data, obj->usage);
    }
    outlet_anything(obj->out, msg, argc, argv);
    glBindBuffer(GL_ARRAY_BUFFER, old_buffer);
}


static void gl_array_buffer_refresh(t_gl_array_buffer_obj *obj) {
    t_garray *garray = (t_garray *)pd_findbyclass(obj->array, garray_class);
    if (garray) {
        GLsizeiptr size = garray_npoints(garray)*sizeof(float);
        if (obj->read) {
            if (size != obj->size) {
                garray_resize_long(garray, obj->size);
            }
            int isize = 0;
            garray_getfloatarray(garray, &isize, &obj->data);
            GLint old_buffer = 0;
            glGetIntegerv(GL_ARRAY_BUFFER, &old_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, obj->arraybuffer->buffer);
            glBufferData(GL_ARRAY_BUFFER, obj->size, obj->data, obj->usage);
            glBindBuffer(GL_ARRAY_BUFFER, old_buffer);
            
        } else {
            if (size) {
                int isize = 0;
                garray_getfloatarray(garray, &isize, &obj->data);
                GLint old_buffer = 0;
                glGetIntegerv(GL_ARRAY_BUFFER, &old_buffer);
                glBindBuffer(GL_ARRAY_BUFFER, obj->arraybuffer->buffer);
                glBufferData(GL_ARRAY_BUFFER, obj->size, obj->data, obj->usage);
                glBindBuffer(GL_ARRAY_BUFFER, old_buffer);
            }
        }
    }
}

static void gl_array_buffer_reset(t_gl_array_buffer_obj *obj,
                                  t_symbol *msg, int argc, t_atom *argv) {
    gl_array_buffer_refresh(obj);
    outlet_anything(obj->out, msg, argc, argv);
}

static void gl_array_buffer_cleanup(t_gl_array_buffer_obj *obj) {
}

void gl_array_buffer_setup(void) {
    gl_array_buffer_class = class_new(gensym("gl.arraybuffer"),
                                      (t_newmethod)gl_array_buffer_new,
                                      (t_method)gl_array_buffer_free,
                                      sizeof(t_gl_array_buffer_obj), CLASS_DEFAULT,
                                      A_SYMBOL, A_SYMBOL, A_SYMBOL, NULL);
    class_addmethod(gl_array_buffer_class,
                    (t_method)gl_array_buffer_render,
                    render, A_GIMME, NULL);
    class_addmethod(gl_array_buffer_class,
                    (t_method)gl_array_buffer_reset,
                    reset, A_GIMME, NULL);
}