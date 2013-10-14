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
#include <stdbool.h>
#include "utstring.h"
#include "uthash.h"

#if defined USE_SDL
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include <SDL.h>
#elif defined USE_GLFW
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#endif


static t_class *gl_window_class;
static t_class *gl_win_class;

typedef struct _glwindow {
    t_object x_obj;
    t_symbol *name;
#if defined USE_SDL
    SDL_Window *window;
    SDL_GLContext *glcontext;
#elif defined USE_GLFW
    GLFWwindow *window;
    double old_cursor_x, old_cursor_y;
#endif
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

/**
 * sends stuff over to a [gl.win]'s outlet
 */
static void gl_win_send_list_to_outlets(t_gl_win_obj *obj, t_symbol *s, int argc, t_atom *argv) {
    while (obj != NULL) {
        outlet_list(obj->event_out, s, argc, argv);
        obj = obj->next;
    }
}


/**
 * the event handler for each window.
 * the frequency in which this function is called is currently set at 120 times/sec
 */
static void gl_win_event_tick(glwindow *win) {
#if defined USE_SDL
    int arg_count, bytes_count;
    t_atom *arg_list;
    t_symbol *sym;
    SDL_Event event;
    // loop while there are events in the queue
    while (SDL_PollEvent(&event)) {
        // set up the argument lists for no event
        arg_list = NULL;
        sym = NULL;
        arg_count = 0;
        bytes_count = 0;
        switch (event.type) {
            case SDL_QUIT:
                arg_list = getbytes(bytes_count = sizeof(t_atom));
                sym = &s_list;
                SETSYMBOL(arg_list, gensym("quit"));
                arg_count = 1;
                break;
            case SDL_WINDOWEVENT:
            {
                // window events don't normally use both data1 and data2, but
                // they'll be set to 0 when not used, and should be ignored on
                // user patches.
                char *windowev = "none";
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SHOWN:
                        windowev = "shown";
                        break;
                    case SDL_WINDOWEVENT_CLOSE:
                        windowev = "close";
                        break;
                    case SDL_WINDOWEVENT_ENTER:
                        windowev = "enter";
                        break;
                    case SDL_WINDOWEVENT_EXPOSED:
                        windowev = "exposed";
                        break;
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        windowev = "focus_gained";
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        windowev = "focus_lost";
                        break;
                    case SDL_WINDOWEVENT_HIDDEN:
                        windowev = "hidden";
                        break;
                    case SDL_WINDOWEVENT_LEAVE:
                        windowev = "leave";
                        break;
                    case SDL_WINDOWEVENT_MAXIMIZED:
                        windowev = "maximized";
                        break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                        windowev = "minimized";
                        break;
                    case SDL_WINDOWEVENT_MOVED:
                        windowev = "moved";
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        windowev = "resized";
                        break;
                    case SDL_WINDOWEVENT_RESTORED:
                        windowev = "restored";
                        break;
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        windowev = "size_changed";
                        break;
                    default:
                        break;
                }
                arg_list = getbytes(bytes_count = sizeof(t_atom)*3);
                sym = &s_list;
                SETSYMBOL(arg_list, gensym(windowev));
                SETFLOAT(arg_list+1, event.window.data1);
                SETFLOAT(arg_list+2, event.window.data2);
                arg_count = 3;
            }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // special keys use some of the higher bits of an uint32_t,
                // making floats lose precision - need to treat these some
                // other way
                arg_list = getbytes(bytes_count = sizeof(t_atom)*3);
                sym = &s_list;
                SETSYMBOL(arg_list, (event.type==SDL_KEYUP)?gensym("keyup"):gensym("keydown"));
                SETFLOAT(arg_list+1, event.key.keysym.sym);
                SETFLOAT(arg_list+2, (event.type==SDL_KEYDOWN)?1:0);
                arg_count = 3;
                break;
            case SDL_MOUSEMOTION:
                // z movement (mouse wheel) is treated separately
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
                // fourth element in list is mouse button index
                arg_list = getbytes(bytes_count = sizeof(t_atom)*4);
                sym = &s_list;
                SETSYMBOL(arg_list, (event.type==SDL_MOUSEBUTTONUP)?gensym("mousebuttonup"):gensym("mousebuttondown"));
                SETFLOAT(arg_list+1, event.button.x);
                SETFLOAT(arg_list+2, event.button.y);
                SETFLOAT(arg_list+3, event.button.button);
                arg_count = 4;
                break;
            case SDL_MOUSEWHEEL:
                // x and y are deltas over wheel movement
                arg_list = getbytes(bytes_count = sizeof(t_atom)*3);
                sym = &s_list;
                SETSYMBOL(arg_list, gensym("mousewheel"));
                SETFLOAT(arg_list+1, event.wheel.x);
                SETFLOAT(arg_list+2, event.wheel.y);
                arg_count = 3;
                break;
            default:
                break;
        }
        // if sym is set to something (most likely &s_list) we send the lists
        // to the outlets of gl.win's
        if (sym) {
            gl_win_send_list_to_outlets(win->win_head, sym, arg_count, arg_list);
        }
        // if list storage was requested, that storage will be reclaimed
        if (arg_list) {
            freebytes(arg_list, bytes_count);
        }
    }

#elif defined USE_GLFW
    glfwPollEvents();
    
#endif
    // schedule next call to this function
    if (win->window) {
        clock_delay(win->event_clock, win->event_delta_time);
    }
}

/**
 * render head function
 * dispatches render calls to gl.* objects
 */
static void gl_win_window_tick(glwindow *win) {
    t_gl_renderhead_obj *head = win->rh_head;
    while (head) {
        outlet_anything(head->render_out, render, 0, 0);
        head = head->next;
    }
#if defined USE_SDL
    // stuff rendered, swap window to show rendered stuff
    SDL_GL_SwapWindow(win->window);
#elif defined USE_GLFW
    glfwSwapBuffers(win->window);
#endif
    // schedule next call to this function
    if (win->keep_rendering) {
        clock_delay(win->dispatch_clock, win->frame_delta_time);
    }
}

#if defined USE_GLFW

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    t_gl_win_obj *winobj = glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = getbytes(bytes_count = sizeof(t_atom)*3);
    sym = &s_list;
    SETSYMBOL(arg_list, (action==GLFW_PRESS)?gensym("keydown"):((action==GLFW_REPEAT)?gensym("keyrepeat"):gensym("keyup")));
    SETFLOAT(arg_list+1, key);
    SETFLOAT(arg_list+2, (action==GLFW_PRESS||action==GLFW_REPEAT)?1:0);
    arg_count = 3;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_mouse_pos_callback(GLFWwindow *window, double x, double y) {
    t_gl_win_obj *winobj = glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = getbytes(bytes_count = sizeof(t_atom)*5);
    sym = &s_list;
    SETSYMBOL(arg_list, gensym("mousemotion"));
    SETFLOAT(arg_list+1, x);
    SETFLOAT(arg_list+2, y);
    SETFLOAT(arg_list+3, x - winobj->window->old_cursor_x);
    SETFLOAT(arg_list+4, x - winobj->window->old_cursor_y);
    arg_count = 5;
    winobj->window->old_cursor_x = x;
    winobj->window->old_cursor_y = y;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    t_gl_win_obj *winobj = glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = getbytes(bytes_count = sizeof(t_atom)*4);
    sym = &s_list;
    SETSYMBOL(arg_list, (action==GLFW_PRESS)?gensym("mousebuttondown"):gensym("mousebuttonup"));
    SETFLOAT(arg_list+1, winobj->window->old_cursor_x);
    SETFLOAT(arg_list+2, winobj->window->old_cursor_y);
    SETFLOAT(arg_list+3, button);
    arg_count = 4;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

#endif

/**
 * creates an internal window object, registering event and render callbacks
 * and saving the object to a symbol hash
 *
 * @param name window name
 * @return a glwindow object with all the setup
 */
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

/**
 * finds a named window object. if it doesn't exist, create it
 * (for internal use, so the automatic creation of a new object is not a problem)
 *
 * @param name window name
 * @return the corresponding window object
 */
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


static void gl_win_title(t_gl_win_obj *obj, t_symbol *sym, int argc, t_atom *argv) {
#if defined USE_SDL
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
#elif defined USE_GLFW
    if (obj->window->window <= 0) {
        if (obj->window->name == default_window) {
            post("no window was created for the default gl_win object\n");
        } else {
            post("no window was created for the \"$s\" gl_win object\n",
                 obj->window->name->s_name);
        }
    }
    if (obj->title) {
        free(obj->title);
    }
    obj->title = list_to_string(argc, argv);
    glfwSetWindowTitle(obj->window->window, obj->title);
#endif
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
#if defined USE_SDL
    obj->window->window = SDL_CreateWindow(obj->title,
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   obj->width, obj->height,
                                   SDL_WINDOW_OPENGL|
                                   (obj->fullscreen?SDL_WINDOW_FULLSCREEN:0));
    obj->window->glcontext = SDL_GL_CreateContext(obj->window->window);
    int gl_major = 0, gl_minor = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &gl_major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &gl_minor);
    post("gl context version: %d.%d",gl_major, gl_minor);
#elif defined USE_GLFW
    obj->window->window = glfwCreateWindow(obj->width, obj->height,
                                           (obj->title)?obj->title:"Glance window",
                                           NULL, NULL);
    glfwSetWindowUserPointer(obj->window->window, obj);
    glfwSetKeyCallback(obj->window->window, glfw_key_callback);
    glfwSetCursorPosCallback(obj->window->window, glfw_mouse_pos_callback);
    glfwSetMouseButtonCallback(obj->window->window, glfw_mouse_button_callback);
    
#endif
    gl_win_event_tick(obj->window);
}

static void gl_win_fullscreen(t_gl_win_obj *obj, t_float fs) {
#if defined USE_SDL
    if (obj->window) {
        SDL_SetWindowFullscreen(obj->window->window, fs!=0.0);
    }
#elif defined USE_GLFW
    
#endif
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
#if defined USE_SDL
    if (obj->window == NULL) {
        post("no window to destroy");
        return;
    }
    SDL_GL_DeleteContext(obj->glcontext);
    SDL_DestroyWindow(obj->window);
    obj->window = NULL;
#elif defined USE_GLFW
    if (obj->window == NULL) {
        post("no window to destroy");
        return;
    }
    glfwDestroyWindow(obj->window);
    obj->window = NULL;
#endif
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
