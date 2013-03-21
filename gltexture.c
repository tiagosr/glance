//
//  gltexture.c
//  glance
//
//  Created by Tiago Rezende on 3/18/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include "glance.h"
#include <FreeImage.h>


static t_class *gl_texture_class;

typedef struct _gl_texture_obj {
    t_object x_obj;
    GLuint texture;
    GLenum texnum;
    GLenum magfilter, minfilter;
    bool with_alpha;
    bool with_border;
    float bcolor[4];
    t_outlet *out;
} t_gl_texture_obj;


static void *gl_texture_new(void) {
    t_gl_texture_obj *obj = (t_gl_texture_obj *)pd_new(gl_texture_class);
    glGenTextures(1, &obj->texture);
    obj->texnum = GL_TEXTURE0;
    obj->with_alpha = true;
    obj->magfilter = GL_LINEAR;
    obj->minfilter = GL_LINEAR;
    obj->with_border = false;
    obj->bcolor[0] = 0.0;
    obj->bcolor[1] = 0.0;
    obj->bcolor[2] = 0.0;
    obj->bcolor[3] = 0.0;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void gl_texture_destroy(t_gl_texture_obj *obj) {
    glDeleteTextures(1, &obj->texture);
}

static void gl_texture_load_file(t_gl_texture_obj *obj, t_symbol *fname) {
    FREE_IMAGE_FORMAT fmt = FreeImage_GetFileType(fname->s_name, 0);
    GLint activetexture = 0, boundtexture = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activetexture);
    glActiveTexture(obj->texnum);
    FIBITMAP * bmp = FreeImage_Load(fmt, fname->s_name, 0);
    if (bmp) {
        FIBITMAP *temp = bmp;
        bmp = FreeImage_ConvertTo32Bits(bmp);
        FreeImage_Unload(temp);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundtexture);
        glBindTexture(GL_TEXTURE_2D, obj->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, 8,
                     FreeImage_GetWidth(bmp), FreeImage_GetHeight(bmp),
                     obj->with_border?1:0, GL_BGRA,
                     GL_UNSIGNED_BYTE, FreeImage_GetBits(bmp));
        glBindTexture(GL_TEXTURE_2D, boundtexture);
        FreeImage_Unload(bmp);
    } else {
        error("failure loading file %s", fname->s_name);
    }
    glActiveTexture(activetexture);
}

static void gl_texture_texnum(t_gl_texture_obj *obj, t_float num) {
    obj->texnum = GL_TEXTURE0+num;
}

static void gl_texture_magfilter(t_gl_texture_obj *obj, t_symbol *sym) {
    GLint prevbound = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevbound);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    if (sym == gensym("LINEAR")) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else if (sym == gensym("NEAREST")) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glBindTexture(GL_TEXTURE_2D, prevbound);
}

static void gl_texture_minfilter(t_gl_texture_obj *obj, t_symbol *sym) {
    GLint prevbound = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevbound);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    if (sym == gensym("LINEAR")) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else if (sym == gensym("NEAREST")) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    } // mipmap ones will only make sense when mipmap is actually implemented
    glBindTexture(GL_TEXTURE_2D, prevbound);
}
static t_sym_uint_list gl_texwrap_modes[] = {
    {"CLAMP_TO_EDGE", GL_CLAMP_TO_EDGE},
    {"CLAMP_TO_BORDER", GL_CLAMP_TO_BORDER},
    {"MIRRORED_REPEAT", GL_MIRRORED_REPEAT},
    {"REPEAT", GL_REPEAT},
    {0,0}
};
static void gl_texture_wrap(t_gl_texture_obj *obj,
                            t_symbol *s, t_symbol *t) {
    GLint prevbound = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevbound);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    GLenum texwrapmode = 0;
    if (find_uint_for_sym(gl_texwrap_modes, s, &texwrapmode)) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texwrapmode);
    }
    if (find_uint_for_sym(gl_texwrap_modes, t, &texwrapmode)) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texwrapmode);
    }
    glBindTexture(GL_TEXTURE_2D, prevbound);
    
}


static void gl_texture_render(t_gl_texture_obj *obj, t_symbol *s, int argc, t_atom *argv) {
    GLint activetexture = 0, boundtexture = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activetexture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundtexture);
    glActiveTexture(obj->texnum);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    outlet_anything(obj->out, s, argc, argv);
    glBindTexture(GL_TEXTURE_2D, boundtexture);
    glActiveTexture(activetexture);
}

void gl_texture_setup(void) {
    gl_texture_class = class_new(gensym("gl.texture"),
                                 (t_newmethod)gl_texture_new,
                                 (t_method)gl_texture_destroy,
                                 sizeof(t_gl_texture_obj), CLASS_DEFAULT, 0);
    class_addmethod(gl_texture_class, (t_method)gl_texture_render,
                    render, A_GIMME, 0);
    class_addmethod(gl_texture_class, (t_method)gl_texture_load_file,
                    gensym("load"), A_SYMBOL, 0);
    
}


