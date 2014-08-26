//
//  window.c
//  glance
//
//  Created by Tiago Rezende on 3/6/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "window.hh"
#include <stdlib.h>

static t_class *gl_window_class;
static t_class *gl_win_class;

static t_symbol
    *keydown,
    *keyup,
    *keyrepeat,
    *mousemotion,
    *mousebuttondown,
    *mousebuttonup,
    *mouseentered,
    *mouseexited,
    *mousewheel,
    *unicodechar,
    *windowposition,
    *windowsize,
    *windowclose,
    *windowfocus,
    *windowdefocus,
    *windowiconify,
    *windowrestore;

static void setup_symbols() {
    keydown = gensym("key-down");
    keyup = gensym("key-up");
    keyrepeat = gensym("key-repeat");
    mousemotion = gensym("mouse-motion");
    mouseentered = gensym("mouse-entered");
    mouseexited = gensym("mouse-exited");
    mousewheel = gensym("mouse-wheel");
    mousebuttondown = gensym("mousebutton-down");
    mousebuttondown = gensym("mousebutton-up");
    unicodechar = gensym("unicode-char");
    windowclose = gensym("window-close");
    windowdefocus = gensym("window-defocus");
    windowfocus = gensym("window-focus");
    windowiconify = gensym("window-iconify");
    windowrestore = gensym("window-restore");
    windowposition = gensym("window-position");
}

std::map<t_symbol *, glwindow *> glwindow::windows;

t_symbol *default_window;

struct t_gl_win_obj {
    t_object x_obj;
    std::string title;
    int width, height;
    bool fullscreen;
    glwindow *window;
    t_outlet *event_out;
};

struct t_gl_renderhead_obj {
    t_object x_obj;
    glwindow *window;
    t_outlet *render_out;
};


static void gl_win_destroy(t_gl_win_obj *obj);

/**
 * sends stuff over to a [gl.win]'s outlet
 */
static void gl_win_send_list_to_outlets(t_gl_win_obj *obj, t_symbol *s, int argc, t_atom *argv) {
    win_obj_list *list = &obj->window->cpp->win_objs;
    for (win_obj_list::const_iterator ci = list->begin(); ci!=list->end(); ci++) {
        outlet_list((*ci)->event_out, s, argc, argv);
        
    }
}


/**
 * the event handler for each window.
 * the frequency in which this function is called is currently set at 120 times/sec
 */
static void gl_win_event_tick(glwindow *win) {
    glfwPollEvents();
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
    glfwMakeContextCurrent(win->window);
    
    for(render_head_list::const_iterator ci = win->cpp->renderheads.begin();
        ci != win->cpp->renderheads.end();
        ci++) {
        if (!win->reset) {
            outlet_anything((*ci)->render_out, reset, 0, 0);
        }
        outlet_anything((*ci)->render_out, render, 0, 0);
    }
    
    glfwSwapBuffers(win->window);
    // schedule next call to this function
    if (win->keep_rendering) {
        clock_delay(win->dispatch_clock, win->frame_delta_time);
    }
    win->reset = true;
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*3);
    sym = &s_list;
    SETSYMBOL(arg_list, (action==GLFW_PRESS)?keydown:((action==GLFW_REPEAT)?keyrepeat:keyup));
    SETFLOAT(arg_list+1, key);
    SETFLOAT(arg_list+2, (action==GLFW_PRESS||action==GLFW_REPEAT)?1:0);
    arg_count = 3;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_mouse_pos_callback(GLFWwindow *window, double x, double y) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*5);
    sym = &s_list;
    SETSYMBOL(arg_list, mousemotion);
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
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*4);
    sym = &s_list;
    SETSYMBOL(arg_list, (action==GLFW_PRESS)?mousebuttondown:mousebuttonup);
    SETFLOAT(arg_list+1, winobj->window->old_cursor_x);
    SETFLOAT(arg_list+2, winobj->window->old_cursor_y);
    SETFLOAT(arg_list+3, button);
    arg_count = 4;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_mouse_enter_callback(GLFWwindow *window, int entered) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*2);
    sym = &s_list;
    SETSYMBOL(arg_list, (entered==GL_TRUE)?mouseentered:mouseexited);
    SETFLOAT(arg_list+1, entered);
    arg_count = 2;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_unicode_char_callback(GLFWwindow *window, unsigned int codepoint) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*2);
    sym = &s_list;
    SETSYMBOL(arg_list, unicodechar);
    SETFLOAT(arg_list+1, codepoint);
    arg_count = 2;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_mouse_scroll_callback(GLFWwindow *window, double x, double y) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*3);
    sym = &s_list;
    SETSYMBOL(arg_list, mousewheel);
    SETFLOAT(arg_list+1, x);
    SETFLOAT(arg_list+2, y);
    arg_count = 3;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_window_pos_callback(GLFWwindow *window, int w, int h) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*3);
    sym = &s_list;
    SETSYMBOL(arg_list, windowposition);
    SETFLOAT(arg_list+1, w);
    SETFLOAT(arg_list+2, h);
    arg_count = 3;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_window_resize_callback(GLFWwindow *window, int w, int h) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*3);
    sym = &s_list;
    SETSYMBOL(arg_list, windowsize);
    SETFLOAT(arg_list+1, w);
    SETFLOAT(arg_list+2, h);
    arg_count = 3;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_window_close_callback(GLFWwindow *window) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom));
    sym = &s_list;
    SETSYMBOL(arg_list, windowclose);
    arg_count = 1;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_window_focus_callback(GLFWwindow *window, int focus) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*2);
    sym = &s_list;
    SETSYMBOL(arg_list, (focus==GL_TRUE)?windowfocus:windowdefocus);
    SETFLOAT(arg_list+1, focus);
    arg_count = 2;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}

static void glfw_window_iconify_callback(GLFWwindow *window, int focus) {
    t_gl_win_obj *winobj = (t_gl_win_obj *)glfwGetWindowUserPointer(window);
    t_atom *arg_list;
    int bytes_count, arg_count;
    t_symbol *sym;
    arg_list = (t_atom *)getbytes(bytes_count = sizeof(t_atom)*2);
    sym = &s_list;
    SETSYMBOL(arg_list, (focus==GL_TRUE)?windowiconify:windowrestore);
    SETFLOAT(arg_list+1, focus);
    arg_count = 2;
    gl_win_send_list_to_outlets(winobj, sym, arg_count, arg_list);
    freebytes(arg_list, bytes_count);
}



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
    glwindow::windows[name] = newwin;
    return newwin;
}

/**
 * finds a named window object. if it doesn't exist, create it
 * (for internal use, so the automatic creation of a new object is not a problem)
 *
 * @param name window name
 * @return the corresponding window object
 */
glwindow *gl_find_window(t_symbol *name) {
    if (glwindow::windows.find(name)==glwindow::windows.end()) {
        return gl_win_new_window(name);
    } else {
        return glwindow::windows[name];
    }
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
    obj->title = "glance window";
    obj->window->cpp->win_objs.push_back(obj);
    obj->event_out = outlet_new(&obj->x_obj, &s_list);
    return (void *) obj;
}

static void gl_win_obj_destroy(t_gl_win_obj *obj) {
    // remove this object from the queue
    obj->window->cpp->win_objs.remove(obj);
    if (--obj->window->refcount<=0) {
        if (obj->window->window) {
            gl_win_destroy(obj);
        }
        glwindow::windows.erase(obj->window->name);
        free(obj->window);
    }
    outlet_free(obj->event_out);
}


static void gl_win_title(t_gl_win_obj *obj, t_symbol *sym, int argc, t_atom *argv) {
    if (obj->window->window <= 0) {
        if (obj->window->name == default_window) {
            post("no window was created for the default gl_win object\n");
        } else {
            post("no window was created for the \"$s\" gl_win object\n",
                 obj->window->name->s_name);
        }
    }
    obj->title = list_to_string(argc, argv);
    glfwSetWindowTitle(obj->window->window, obj->title.c_str());
}

static void gl_win_post_info(t_gl_win_obj *obj) {
    if (obj->window->window <= 0) {
        if (obj->window->name == default_window) {
            post("no window was created for the default gl_win object\n");
        } else {
            post("no window was created for the \"$s\" gl_win object\n",
                 obj->window->name->s_name);
        }
    } else {
        int major = glfwGetWindowAttrib(obj->window->window, GLFW_CONTEXT_VERSION_MAJOR);
        int minor = glfwGetWindowAttrib(obj->window->window, GLFW_CONTEXT_VERSION_MINOR);
        post("GLFW info for window %s: major %d, minor %d", obj->window->name, major, minor);
    }
}

static void gl_win_dimen(t_gl_win_obj *obj, float width, float height) {
    obj->width = width;
    obj->height = height;
}

static std::list<GLFWwindow *> windows;
static void gl_win_create(t_gl_win_obj *obj) {
    if (obj->window->window) {
        post("window already created");
        return;
    }
    
    obj->window->window = glfwCreateWindow(obj->width, obj->height,
                                           obj->title.c_str(),
                                           NULL, windows.size()?windows.front():NULL);
    windows.push_back(obj->window->window);
    
    glfwSetWindowUserPointer(obj->window->window, obj);
    glfwSetKeyCallback(obj->window->window, glfw_key_callback);
    glfwSetCursorPosCallback(obj->window->window, glfw_mouse_pos_callback);
    glfwSetMouseButtonCallback(obj->window->window, glfw_mouse_button_callback);
    glfwSetScrollCallback(obj->window->window, glfw_mouse_scroll_callback);
    glfwSetCursorEnterCallback(obj->window->window, glfw_mouse_enter_callback);
    glfwSetCharCallback(obj->window->window, glfw_unicode_char_callback);
    glfwSetWindowCloseCallback(obj->window->window, glfw_window_close_callback);
    glfwSetWindowFocusCallback(obj->window->window, glfw_window_focus_callback);
    glfwSetWindowIconifyCallback(obj->window->window, glfw_window_iconify_callback);
    glfwSetWindowPosCallback(obj->window->window, glfw_window_pos_callback);
    glfwSetWindowSizeCallback(obj->window->window, glfw_window_resize_callback);
    
    glfwGetCursorPos(obj->window->window, &obj->window->old_cursor_x, &obj->window->old_cursor_y);
    gl_win_event_tick(obj->window);
}

static void gl_win_fullscreen(t_gl_win_obj *obj, t_float fs) {
    
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
    windows.remove(obj->window);
    glfwDestroyWindow(obj->window);
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
    obj->window->cpp->renderheads.push_back(obj);
    obj->render_out = outlet_new(&obj->x_obj, &s_list);
    return (void *)obj;
}

static void gl_head_destroy(t_gl_renderhead_obj *obj) {
    // remove this object from the queue
    obj->window->cpp->renderheads.remove(obj);
    // remove window if no one is using it anymore
    if (--obj->window->refcount<=0) {
        gl_win_window_destroy(obj->window);
        glwindow::windows.erase(obj->window->name);
        free(obj->window);
    }
    outlet_free(obj->render_out);
}



void gl_win_setup(void) {
    
    setup_symbols();
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
                    gensym("create"), A_NULL, 0);
    class_addmethod(gl_win_class, (t_method)gl_win_dimen,
                    gensym("dimen"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_win_class, (t_method)gl_win_destroy,
                    gensym("destroy"), A_NULL, 0);
    class_addmethod(gl_win_class, (t_method)gl_win_post_info,
                    gensym("post-info"), A_NULL, 0);
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
