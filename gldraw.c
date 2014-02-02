//
//  gldrawarrays.c
//  glance
//
//  Created by Tiago Rezende on 3/16/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"


static t_class *gl_drawarrays_class, *gl_drawelements_class;
typedef struct _gl_drawarrays {
    t_object x_obj;
    GLenum mode;
    GLint first;
    GLsizei count;
    t_outlet *out;
} t_gl_drawarrays;

typedef struct _gl_drawelements {
    t_object x_obj;
    GLenum mode;
    GLsizei count;
    GLsizei primcount;
    GLsizei indices_count;
    GLint *indices;
    t_outlet *out;
} t_gl_drawelements;

static void gl_drawarrays_mode(t_gl_drawarrays *obj, t_symbol *sym);
static void gl_drawelements_mode(t_gl_drawelements *obj, t_symbol *sym);

static void *gl_drawarrays_new(t_symbol *smode, t_float first, t_float count) {
    t_gl_drawarrays *obj = (t_gl_drawarrays *)pd_new(gl_drawarrays_class);
    gl_drawarrays_mode(obj, smode);
    obj->first = first;
    obj->count = count;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}
static void *gl_drawelements_new(t_symbol *smode, t_float count) {
    t_gl_drawelements *obj = (t_gl_drawelements *)pd_new(gl_drawelements_class);
    gl_drawelements_mode(obj, smode);
    obj->primcount = 0;
    obj->count = count;
    obj->indices_count = obj->count;
    obj->indices = malloc(sizeof(GLint)*obj->indices_count);
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void gl_drawarrays_free(t_gl_drawarrays *obj) {
    outlet_free(obj->out);
}

static void gl_drawelements_free(t_gl_drawelements *obj) {
    free(obj->indices);
    outlet_free(obj->out);
}

static t_sym_uint_list gl_draw_modes[] = {
    {"POINTS", GL_POINTS},
    {"LINES", GL_LINES},
    {"LINE_STRIP", GL_LINE_STRIP},
    {"TRIANGLES", GL_TRIANGLES},
    {"TRIANGLE_FAN", GL_TRIANGLE_FAN},
    {"TRIANGLE_STRIP", GL_TRIANGLE_STRIP},
    {0,0}
};

static void gl_drawarrays_mode(t_gl_drawarrays *obj, t_symbol *sym) {
    find_uint_for_sym(gl_draw_modes, sym, &obj->mode);
}

static void gl_drawelements_mode(t_gl_drawelements *obj, t_symbol *sym) {
    find_uint_for_sym(gl_draw_modes, sym, &obj->mode);
}


static void gl_drawarrays_first(t_gl_drawarrays *obj, t_float v) {
    obj->first = v;
}
static void gl_drawarrays_count(t_gl_drawarrays *obj, t_float v) {
    obj->count = v;
}

static void gl_drawelements_count(t_gl_drawelements *obj, t_float v) {
    obj->count = v;
}
static void gl_drawelements_primcount(t_gl_drawelements *obj, t_float v) {
    obj->primcount = v;
}


static void gl_drawelements_setindex(t_gl_drawelements *obj, t_float f_item, t_float index) {
    int item = f_item;
    if ((item >= 0) && (item < obj->indices_count)) {
        obj->indices[item] = index;
    } else if ((item < 0)&& (-item < obj->indices_count)) {
        obj->indices[obj->indices_count-item] = index;
    }
}


static void gl_drawarrays_render(t_gl_drawarrays *obj, t_symbol *s, int argc, t_atom *argv) {
    glDrawArrays(obj->mode, obj->first, obj->count);
    outlet_anything(obj->out, s, argc, argv);
}
static void gl_drawelements_render(t_gl_drawelements *obj, t_symbol *s, int argc, t_atom *argv) {
    if (obj->primcount <= 1) {
        glDrawElements(obj->mode, obj->count, GL_INT, obj->indices);
    } else {
        glDrawElementsInstanced(obj->mode, obj->count, GL_INT, obj->indices, obj->primcount);
    }
    outlet_anything(obj->out, s, argc, argv);
}

void gl_draw_setup(void) {
    gl_drawarrays_class = class_new(gensym("gl.drawarrays"),
                                (t_newmethod)gl_drawarrays_new,
                                (t_method)gl_drawelements_free,
                                sizeof(t_gl_drawarrays), CLASS_DEFAULT,
                                A_SYMBOL, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_drawarrays_class, (t_method)gl_drawarrays_mode, gensym("mode"), A_SYMBOL, 0);
    class_addmethod(gl_drawarrays_class, (t_method)gl_drawarrays_first, gensym("first"), A_FLOAT, 0);
    class_addmethod(gl_drawarrays_class, (t_method)gl_drawarrays_count, gensym("count"), A_FLOAT, 0);
    class_addmethod(gl_drawarrays_class, (t_method)gl_drawarrays_render, render, A_GIMME, 0);
    
    gl_drawelements_class = class_new(gensym("gl.drawelements"),
                                      (t_newmethod)gl_drawelements_new,
                                      (t_method)gl_drawelements_free,
                                      sizeof(t_gl_drawelements), CLASS_DEFAULT,
                                      A_SYMBOL, A_FLOAT, 0);
    class_addmethod(gl_drawelements_class, (t_method)gl_drawelements_mode, gensym("mode"), A_SYMBOL, 0);
    class_addmethod(gl_drawelements_class, (t_method)gl_drawelements_count, gensym("count"), A_FLOAT, 0);
    class_addmethod(gl_drawelements_class, (t_method)gl_drawelements_render, render, A_GIMME, 0);
}

void gl_drawarrays_setup(void) { gl_draw_setup(); }
void gl_drawelements_setup(void) { gl_draw_setup(); }