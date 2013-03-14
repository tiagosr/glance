//
//  glance.c
//  glance
//
//  Created by Tiago Rezende on 3/5/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include <stdio.h>

static t_class *glance_class;

typedef struct _glance_obj {
    t_object x_obj;
    //
} t_glance_obj;

static void glance_bang(t_glance_obj *obj) {
    
}
static void * glance_new(void) {
    t_glance_obj *obj = (t_glance_obj *)pd_new(glance_class);
    return (void *)obj;
}

static t_class *glancewin_class;
typedef struct _glancewin_obj {
    t_object x_obj;
    SDL_Window *window;
} t_glancewin_obj;

static void * glancewin_new(t_symbol *s, int argc, t_atom *argv) {
    t_glancewin_obj *obj = (t_glancewin_obj *)pd_new(glancewin_class);
    obj->window = NULL;
    inlet_new(&obj->x_obj, &obj->x_obj.ob_pd, &s_anything, &s_anything);
    return (void *) obj;
}


#define MAX_WIN_TITLE_SIZE 511
static void glancewin_title(t_glancewin_obj *obj, t_symbol *s, int argc, t_atom *argv) {
    if (obj->window == NULL) {
        post("no window was created for this glancewin object\n");
        return;
    }
    char title[MAX_WIN_TITLE_SIZE+1];
    char argt[MAX_WIN_TITLE_SIZE+1];
    memchr(title, 0, MAX_WIN_TITLE_SIZE+1);
    for (int i = 0; i < argc; i++) {
        argt[0] = 0;
        atom_string(argv+i, argt, MAX_WIN_TITLE_SIZE);
        strncat(title, argt, MAX_WIN_TITLE_SIZE);
    }
    SDL_SetWindowTitle(obj->window, title);
}

static void glancewin_create(t_glancewin_obj *obj, float width, float height ) {
    if (obj->window != NULL) {
        post("window already created");
        return;
    }
    obj->window = SDL_CreateWindow("glance",
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   width, height, SDL_WINDOW_OPENGL);
}

static void glancewin_destroy(t_glancewin_obj *obj) {
    if (obj->window == NULL) {
        post("no window to destroy");
        return;
    }
    SDL_DestroyWindow(obj->window);
    obj->window = NULL;
}

static t_class *glance_program_class;
struct _glance_program {
    t_object x_obj;
    GLuint progid;
    GLint compile_status;
    t_outlet *data_out;
};

typedef struct _glance_program t_glance_program;
static void * glance_program_new(void) {
    t_glance_program *obj = (t_glance_program *)pd_new(glance_program_class);
    inlet_new(&obj->x_obj, &obj->x_obj.ob_pd, &s_anything, &s_anything);
    obj->progid = glCreateProgram();
    obj->compile_status = 0;
    obj->data_out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void glance_program_compile(t_glance_program *obj) {
    glLinkProgram(obj->progid);
    glGetProgramiv(obj->progid, GL_LINK_STATUS, &(obj->compile_status));
    outlet_float(obj->data_out, obj->compile_status);
}

static void glance_program_bang(t_glance_program *obj) {
    if (obj->compile_status) {
        glUseProgram(obj->progid);
    }
}

void glance_setup(void) {
    glance_class = class_new(gensym("glance"),
                           (t_newmethod)glance_new,
                           0, sizeof(t_glance_obj),
                           CLASS_DEFAULT,
                           0);
    class_addbang(glance_class, glance_bang);
    glancewin_class = class_new(gensym("glancewin"),
                                (t_newmethod)glancewin_new,
                                0,
                                sizeof(t_glancewin_obj),
                                CLASS_DEFAULT,
                                A_GIMME, 0);
    class_addmethod(glancewin_class, (t_method)glancewin_title,
                    gensym("title"), A_GIMME, 0);
    class_addmethod(glancewin_class, (t_method)glancewin_create,
                    gensym("create"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(glancewin_class, (t_method)glancewin_destroy,
                    gensym("destroy"), 0);
    
    glance_program_class = class_new(gensym("glProgram"),
                                     (t_newmethod)glance_program_new, 0,
                                     sizeof(t_glance_program), CLASS_DEFAULT, 0);
    class_addmethod(glance_program_class, (t_method)glance_program_compile,
                    gensym("compile"), 0);
}
