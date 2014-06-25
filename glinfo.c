//
//  glinfo.c
//  glance
//
//  Created by Tiago Rezende on 6/25/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"

static t_class *c_glinfo = NULL;

typedef struct _t_glinfo t_glinfo;

struct _t_glinfo {
    t_object x_obj;
    t_outlet *out_version;
    t_outlet *out_vendor;
    t_outlet *out_renderer;
    t_outlet *out_extensions;
    t_outlet *out_glsl_ver;
};

static t_glinfo *glinfo_new(void) {
    t_glinfo *info = (t_glinfo *)pd_new(c_glinfo);
    
    info->out_version = outlet_new(&info->x_obj, gensym("version"));
    info->out_vendor = outlet_new(&info->x_obj, gensym("vendor"));
    info->out_renderer = outlet_new(&info->x_obj, gensym("renderer"));
    info->out_extensions = outlet_new(&info->x_obj, gensym("extensions"));
    info->out_glsl_ver = outlet_new(&info->x_obj, gensym("glslver"));
    
    return info;
};

static void glinfo_free(t_glinfo *info) {
    outlet_free(info->out_version);
    outlet_free(info->out_vendor);
    outlet_free(info->out_renderer);
    outlet_free(info->out_extensions);
    outlet_free(info->out_glsl_ver);
}

static void glinfo_bang(t_glinfo *info) {
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    t_atom version[2];
    SETFLOAT(version, (float)major);
    SETFLOAT(version+1, (float)minor);
    post("gl.info: %s, major: %d, minor: %d\n", glGetString(GL_VERSION), major, minor);
    outlet_list(info->out_version, gensym("version"), 2, version);
    
    t_atom vendor[1];
    char *vendor_str = (char*)glGetString(GL_VENDOR);
    SETSYMBOL(vendor, gensym(vendor_str?vendor_str:"(null)"));
    outlet_list(info->out_vendor, gensym("vendor"), 1, vendor);
    
    t_atom renderer[1];
    char *renderer_str = (char*)glGetString(GL_RENDERER);
    SETSYMBOL(renderer, gensym(renderer_str?renderer_str:"(null)"));
    outlet_list(info->out_vendor, gensym("renderer"), 1, renderer);
    
    GLint numextensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numextensions);
    t_atom extensions[numextensions];
    for (int i = 0; i < numextensions; i++) {
        SETSYMBOL(extensions+i, gensym((char*)glGetStringi(GL_EXTENSIONS, i)));
    }
    outlet_list(info->out_extensions, gensym("extensions"), numextensions, extensions);
    
    t_atom glsl_version[1];
    char *glsl_version_str = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    SETSYMBOL(glsl_version, gensym(glsl_version_str?glsl_version_str:"(null)"));
    outlet_list(info->out_glsl_ver, gensym("glsl-version"), 1, glsl_version);
}

void gl_info_setup(void) {
    c_glinfo = class_new(gensym("gl.info"),
                         (t_newmethod)glinfo_new,
                         (t_method)glinfo_free,
                         sizeof(t_glinfo),
                         CLASS_DEFAULT, A_NULL, 0);
    class_addbang(c_glinfo, glinfo_bang);
}

