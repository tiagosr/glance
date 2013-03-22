//
//  glenable.c
//  glance
//
//  Created by Tiago Rezende on 3/21/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//


#include "m_pd.h"
#include <stdio.h>
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include "glance.h"

static t_class *gl_switch_class;

typedef struct _gl_switch_obj {
    t_object x_obj;
    GLenum mode;
    bool enable;
    bool active;
    t_outlet *out;
} t_gl_switch_obj;

static t_sym_uint_list gl_switch_modes[] = {
    {"DEPTH_TEST", GL_DEPTH_TEST},
    {"STENCIL_TEST", GL_STENCIL_TEST},
    {"SCISSOR_TEST", GL_SCISSOR_TEST},
    {"BLEND", GL_BLEND},
    {"LINE_SMOOTH", GL_LINE_SMOOTH},
    {"MULTISAMPLE", GL_MULTISAMPLE},
    {"CULL_FACE", GL_CULL_FACE},
    {"PROGRAM_POINT_SIZE", GL_PROGRAM_POINT_SIZE},
    {"POLYGON_OFFSET_FILL", GL_POLYGON_OFFSET_FILL},
    {"POLYGON_OFFSET_LINE", GL_POLYGON_OFFSET_LINE},
    {"POLYGON_OFFSET_POINT", GL_POLYGON_OFFSET_POINT},
    {"CLIP_DISTANCE0", GL_CLIP_DISTANCE0},
    {"CLIP_DISTANCE1", GL_CLIP_DISTANCE1},
    {"CLIP_DISTANCE2", GL_CLIP_DISTANCE2},
    {"CLIP_DISTANCE3", GL_CLIP_DISTANCE3},
    {"CLIP_DISTANCE4", GL_CLIP_DISTANCE4},
    {"CLIP_DISTANCE5", GL_CLIP_DISTANCE5},
    {"CLIP_DISTANCE6", GL_CLIP_DISTANCE6},
    {"CLIP_DISTANCE7", GL_CLIP_DISTANCE7},
    {"PRIMITIVE_RESTART", GL_PRIMITIVE_RESTART},
    {"SAMPLE_ALPHA_TO_COVERAGE", GL_SAMPLE_ALPHA_TO_COVERAGE},
    {"SAMPLE_ALPHA_TO_ONE", GL_SAMPLE_ALPHA_TO_ONE},
    {"SAMPLE_COVERAGE", GL_SAMPLE_COVERAGE},
    {"SAMPLE_MASK", GL_SAMPLE_MASK},
    {0,0}
};

static void *gl_switch_enable_new(t_symbol *sym) {
    GLenum mode = 0;
    if (!find_uint_for_sym(gl_switch_modes, sym, &mode)) {
        error("mode %s not recognized\n", sym->s_name);
        return NULL;
    }
    t_gl_switch_obj *obj = (t_gl_switch_obj *)pd_new(gl_switch_class);
    obj->enable = true;
    obj->active = true;
    obj->mode = mode;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}
static void *gl_switch_disable_new(t_symbol *sym) {
    GLenum mode = 0;
    if (!find_uint_for_sym(gl_switch_modes, sym, &mode)) {
        error("mode %s not recognized\n", sym->s_name);
        return NULL;
    }
    t_gl_switch_obj *obj = (t_gl_switch_obj *)pd_new(gl_switch_class);
    obj->enable = false;
    obj->active = true;
    obj->mode = mode;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void gl_switch_destroy(t_gl_switch_obj *obj) {
    outlet_free(obj->out);
}

static void gl_switch_set(t_gl_switch_obj *obj, t_float active) {
    if (active == 0) {
        obj->active = false;
    } else {
        obj->active = true;
    }
}

static void gl_switch_enable(t_gl_switch_obj *obj, t_float enable) {
    if (enable == 0) {
        obj->enable = false;
    } else {
        obj->enable = true;
    }
}

static void gl_switch_render(t_gl_switch_obj *obj, t_symbol *sym, int argc, t_atom *argv) {
    if (obj->active) {
        if (obj->enable) {
            glEnable(obj->mode);
        } else {
            glDisable(obj->mode);
        }
    }
    outlet_anything(obj->out, sym, argc, argv);
}

void gl_switch_setup(void) {
    gl_switch_class = class_new(gensym("gl.enable"),
                                (t_newmethod)gl_switch_enable_new,
                                (t_method)gl_switch_destroy,
                                sizeof(t_gl_switch_obj),
                                CLASS_DEFAULT, A_SYMBOL, 0);
    class_addcreator((t_newmethod)gl_switch_disable_new,
                     gensym("gl.disable"), A_SYMBOL);
    class_addmethod(gl_switch_class, (t_method)gl_switch_render,
                    render, A_GIMME, 0);
    class_addmethod(gl_switch_class, (t_method)gl_switch_enable,
                    gensym("enable"), A_FLOAT, 0);
}

