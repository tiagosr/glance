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
    {"PRIMITIVE_RESTART_INDEX", GL_PRIMITIVE_RESTART_INDEX},
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

