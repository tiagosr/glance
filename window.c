//
//  window.c
//  glance
//
//  Created by Tiago Rezende on 3/6/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include "glance.h"
#include <stdio.h>
#include "utstring.h"
#include "uthash.h"
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>

static t_class *gl_window_class;
static t_class *gl_win_class;
typedef struct _glwindow {
    t_object x_obj;
    t_symbol *name;
    SDL_Window *window;
    SDL_GLContext *glcontext;
    t_clock *dispatch_clock;
    t_clock *event_clock;
    float frame_delta_time;
    float event_delta_time;
    bool keep_rendering;
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

static void gl_win_event_tick(glwindow *win) {
    int arg_count, bytes_count;
    t_atom *arg_list;
    t_symbol *sym;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        arg_list = NULL;
        sym = NULL;
        arg_count = 0;
        bytes_count = 0;
        switch (event.type) {
            case SDL_WINDOWEVENT:
                
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                arg_list = getbytes(bytes_count = sizeof(t_atom)*3);
                sym = &s_list;
                SETSYMBOL(arg_list, (event.type==SDL_KEYUP)?gensym("keyup"):gensym("keydown"));
                SETFLOAT(arg_list+1, event.key.keysym.sym);
                SETFLOAT(arg_list+2, (event.type==SDL_KEYDOWN)?1:0);
                arg_count = 3;
                break;
            case SDL_MOUSEMOTION:
                arg_list = getbytes(bytes_count = sizeof(t_atom)*5);
                sym = &s_list;
                SETSYMBOL(arg_list, gensym("mousemotion"));
                SETFLOAT(arg_list+1, event.motion.x);
                SETFLOAT(arg_list+2, event.motion.y);
                SETFLOAT(arg_list+3, event.motion.xrel);
                SETFLOAT(arg_list+4, event.motion.yrel);
                arg_count = 5;
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                arg_list = getbytes(bytes_count = sizeof(t_atom)*4);
                sym = &s_list;
                SETSYMBOL(arg_list, (event.type==SDL_MOUSEBUTTONUP)?gensym("mousebuttonup"):gensym("mousebuttondown"));
                SETFLOAT(arg_list+1, event.button.x);
                SETFLOAT(arg_list+2, event.button.y);
                SETFLOAT(arg_list+3, event.button.button);
                arg_count = 4;
                break;
            default:
                break;
        }
        if (sym) {
            gl_win_send_list_to_outlets(win->win_head, sym, arg_count, arg_list);
        }
        if (arg_list) {
            freebytes(arg_list, bytes_count);
        }
    }
    if (win->window) {
        clock_delay(win->event_clock, win->event_delta_time);
    }
}

static void gl_win_window_tick(glwindow *win) {
    t_gl_renderhead_obj *head = win->rh_head;
    int counter = 0;
    while (head) {
        outlet_anything(head->render_out, render, 0, 0);
        head = head->next;
        counter++;
    }
    //post("sent to %d render heads.", counter);
    SDL_GL_SwapWindow(win->window);
    if (win->keep_rendering) {
        clock_delay(win->dispatch_clock, win->frame_delta_time);
    }
}

static glwindow *gl_win_new_window(t_symbol *name) {
    glwindow *newwin = (glwindow *)pd_new(gl_window_class);
    memset(newwin, 0, sizeof(glwindow));
    newwin->name = name;
    newwin->dispatch_clock = clock_new(&newwin->x_obj, (t_method)gl_win_window_tick);
    newwin->event_clock = clock_new(&newwin->x_obj, (t_method)gl_win_event_tick);
    newwin->frame_delta_time = 1000.0/30.0;
    newwin->event_delta_time = 1000.0/120.0;
    HASH_ADD_PTR(windows, name, newwin);
    return newwin;
}

static glwindow *gl_find_window(t_symbol *name) {
    glwindow *window = NULL;
    HASH_FIND_PTR(windows, &name, window);
    if (!window) {
        window = gl_win_new_window(name);
    }
    return window;
}

static void * gl_win_new(t_pd *dummy, t_symbol *s, int argc, t_atom *argv) {
    t_gl_win_obj *obj = (t_gl_win_obj *)pd_new(gl_win_class);
    obj->window = NULL;
    t_symbol *name = default_window;
    if (argc > 0) {
        name = atom_getsymbol(argv); // first argument should be the window name
    }
    obj->window = gl_find_window(name);
    obj->window->refcount++;
    obj->width = 640;
    obj->height = 480;
    obj->title = NULL;
    obj->next = obj->window->win_head;
    obj->window->win_head = obj;
    obj->event_out = outlet_new(&obj->x_obj, &s_list);
    return (void *) obj;
}

static void gl_win_obj_destroy(t_gl_win_obj *obj) {
    // remove this object from the queue
    if (obj->window->win_head == obj) {
        obj->window->win_head = obj->window->win_head->next;
    } else {
        t_gl_win_obj *win_head = obj->window->win_head;
        while (win_head) {
            if (win_head->next == obj) {
                win_head->next = obj->next;
            }
            win_head = win_head->next;
        }
    }
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
    outlet_free(obj->event_out);
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
    obj->title = malloc(strlen(s->s_name)+1);
    strcpy(obj->title, s->s_name);
    
    if (obj->window->window) {
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
    obj->window->glcontext = SDL_GL_CreateContext(obj->window->window);
    gl_win_event_tick(obj->window);
}

static void gl_win_fullscreen(t_gl_win_obj *obj, t_float fs) {
    if (obj->window) {
        SDL_SetWindowFullscreen(obj->window->window, fs!=0.0);
    }
    obj->fullscreen = fs != 0.0;
}

static void gl_win_render(t_gl_win_obj *obj, t_float f) {
    if ((f != 0) && obj->window->window) {
        obj->window->keep_rendering = true;
        gl_win_window_tick(obj->window);
    } else {
        obj->window->keep_rendering = false;
        clock_unset(obj->window->dispatch_clock);
    }
}

static void gl_win_window_destroy(glwindow *obj) {
    if (obj->window == NULL) {
        post("no window to destroy");
        return;
    }
    SDL_GL_DeleteContext(obj->glcontext);
    SDL_DestroyWindow(obj->window);
    obj->window = NULL;
}

static void gl_win_destroy(t_gl_win_obj *obj) {
    gl_win_window_destroy(obj->window);
}


static t_class *gl_head_class;

static void *gl_head_new(t_pd *dummy, t_symbol *sym, int argc, t_atom *argv) {
    t_gl_renderhead_obj *obj = (t_gl_renderhead_obj *)pd_new(gl_head_class);
    t_symbol *name = default_window;
    if (argc>=1) {
        name = atom_getsymbol(argv);
    }
    obj->window = gl_find_window(name);
    obj->window->refcount++;
    obj->next = obj->window->rh_head;
    obj->window->rh_head = obj;
    obj->render_out = outlet_new(&obj->x_obj, &s_list);
    return (void *)obj;
}

static void gl_head_destroy(t_gl_renderhead_obj *obj) {
    // remove this object from the queue
    if (obj->window->rh_head == obj) {
        obj->window->rh_head = obj->window->rh_head->next;
    } else {
        t_gl_renderhead_obj *rh_head = obj->window->rh_head;
        while (rh_head) {
            if (rh_head->next == obj) {
                rh_head->next = obj->next;
            }
            rh_head = rh_head->next;
        }
    }
    // remove window if no one is using it anymore
    if (--obj->window->refcount<=0) {
        gl_win_window_destroy(obj->window);
        HASH_DEL(windows, obj->window);
        free(obj->window);
    }
    outlet_free(obj->render_out);
}



void gl_win_setup(void) {
    default_window = gensym(" glance default window ");
    gl_window_class = class_new(gensym(" glance_internal_window"),// with a space, to avoid instantiation from within a patch
                                (t_newmethod)gl_win_new_window,
                                0, sizeof(glwindow), CLASS_NOINLET,
                                A_SYMBOL, 0);
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
    
    gl_head_class = class_new(gensym("gl.head"),
                              (t_newmethod)gl_head_new,
                              (t_method)gl_head_destroy,
                              sizeof(t_gl_renderhead_obj),
                              CLASS_DEFAULT, A_GIMME, 0);
    //class_sethelpsymbol(gl_win_class, gensym("gl_win"));
}
