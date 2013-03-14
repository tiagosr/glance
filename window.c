//
//  window.c
//  glance
//
//  Created by Tiago Rezende on 3/6/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include "utstring.h"
#include "uthash.h"
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>

static t_class *gl_win_class;
typedef struct _glwindow {
    t_symbol *name;
    SDL_Window *window;
    SDL_Thread *eventthread;
    struct _gl_renderhead_obj *rh_head;
    struct _gl_win_obj *win_head;
    int refcount;
    UT_hash_handle hh;
} glwindow;

static glwindow *windows = NULL;
static t_symbol *default_window;

typedef struct _gl_win_obj {
    t_object x_obj;
    char *title;
    int width, height;
    bool fullscreen;
    struct _gl_win_obj *next;
    glwindow *window;
    t_outlet *event_out;
} t_gl_win_obj;

typedef struct _gl_renderhead_obj {
    t_object x_obj;
    glwindow *window;
    struct _gl_renderhead_obj *next;
    t_outlet *render_out;
} t_gl_renderhead_obj;


static void gl_win_destroy(t_gl_win_obj *obj);

static void gl_win_send_list_to_outlets(t_gl_win_obj *obj, t_symbol *s, int argc, t_atom *argv) {
    while (obj != NULL) {
        outlet_list(obj->event_out, s, argc, argv);
        obj = obj->next;
    }
}

static void gl_win_thread(glwindow *win) {
    SDL_Event event;
    int arg_count;
    t_atom *arg_list;
    t_symbol *sym;
    while (SDL_WaitEvent(&event)) {
        arg_list = NULL;
        sym = NULL;
        arg_count = 0;
        switch (event.type) {
            case SDL_WINDOWEVENT:
                
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                sym = (event.type==SDL_KEYUP)?gensym("keyup"):gensym("keydown");
                
            default:
                break;
        }
        if (sym) {
            gl_win_send_list_to_outlets(win->win_head, sym, arg_count, arg_list);
        }
        if (arg_list) {
            free(arg_list);
        }
    }
}

static void * gl_win_new(t_symbol *s, int argc, t_atom *argv) {
    t_gl_win_obj *obj = (t_gl_win_obj *)pd_new(gl_win_class);
    obj->window = NULL;
    t_symbol *name = default_window;
    HASH_FIND_PTR(windows, name, obj->window);
    if (!obj->window) {
        obj->window = malloc(sizeof(glwindow));
        memset(obj->window, 0, sizeof(glwindow));
        HASH_ADD_PTR(windows, name, obj->window);
    }
    obj->window->refcount++;
    obj->title = NULL;
    obj->event_out = outlet_new(&obj->x_obj, &s_list);
    return (void *) obj;
}

static void gl_win_obj_destroy(t_gl_win_obj *obj) {
    if (--obj->window->refcount<=0) {
        if (obj->window->window) {
            gl_win_destroy(obj);
        }
        HASH_DEL(windows, obj->window);
        free(obj->window);
    }
    if (obj->title) {
        free(obj->title);
    }
}


static void gl_win_title(t_gl_win_obj *obj, t_symbol *s) {
    if (obj->window->window == NULL) {
        if (obj->window->name == default_window) {
            post("no window was created for the default gl_win object\n");
        } else {
            post("no window was created for the \"$s\" gl_win object\n",
                 obj->window->name->s_name);
        }
        return;
    }
    if (obj->title) {
        free(obj->title);
    }
    
    if (obj->window) {
        SDL_SetWindowTitle(obj->window->window, obj->title);
    }
}

static void gl_win_dimen(t_gl_win_obj *obj, float width, float height) {
    obj->width = width;
    obj->height = height;
}

static void gl_win_create(t_gl_win_obj *obj) {
    if (obj->window->window) {
        post("window already created");
        return;
    }
    obj->window->window = SDL_CreateWindow(obj->title,
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   obj->width, obj->height,
                                   SDL_WINDOW_OPENGL|
                                   (obj->fullscreen?SDL_WINDOW_FULLSCREEN:0));
}

static void gl_win_fullscreen(t_gl_win_obj *obj, t_float fs) {
    if (obj->window) {
        SDL_SetWindowFullscreen(obj->window->window, fs!=0.0);
    }
    obj->fullscreen = fs != 0.0;
}

static void gl_win_render(t_gl_win_obj *obj, t_float f) {
    
}

static void gl_win_destroy(t_gl_win_obj *obj) {
    if (obj->window == NULL) {
        post("no window to destroy");
        return;
    }
    SDL_DestroyWindow(obj->window->window);
    obj->window = NULL;
}


static t_class *gl_head_class;

static void *gl_head_new(t_symbol *sym) {
    t_gl_renderhead_obj *obj = (t_gl_renderhead_obj *)pd_new(gl_head_class);
    
    obj->render_out = outlet_new(&obj->x_obj, &s_list);
    return (void *)obj;
}



void gl_win_setup(void) {
    default_window = gensym("");
    gl_win_class = class_new(gensym("gl.win"),
                                (t_newmethod)gl_win_new,
                                (t_method)gl_win_obj_destroy,
                                sizeof(t_gl_win_obj),
                                CLASS_DEFAULT,
                                A_GIMME, 0);
    class_addmethod(gl_win_class, (t_method)gl_win_title,
                    gensym("title"), A_GIMME, 0);
    class_addmethod(gl_win_class, (t_method)gl_win_create,
                    gensym("create"), 0);
    class_addmethod(gl_win_class, (t_method)gl_win_dimen,
                    gensym("dimen"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_win_class, (t_method)gl_win_destroy,
                    gensym("destroy"), 0);
    class_addmethod(gl_win_class, (t_method)gl_win_fullscreen,
                    gensym("fullscreen"), A_FLOAT, 0);
    class_addfloat(gl_win_class, (t_method)gl_win_render);
    //class_sethelpsymbol(gl_win_class, gensym("gl_win"));
}
