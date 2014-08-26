//
//  window.h
//  glance
//
//  Created by Tiago Rezende on 7/22/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#ifndef glance_window_h
#define glance_window_h

#include "m_pd.h"
#include "glance.h"
#include <stdio.h>
#include <stdbool.h>
#include <string>
#include <map>
#include <list>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

template <class T>
class resource_list {
    typedef std::map<t_symbol *, std::weak_ptr<T>> resource_map;
    static resource_map resources;
public:
    static std::shared_ptr<T> get_create(t_symbol *sym) {
        if (resources.find(sym) == resources.end()) {
            resources[sym] = std::shared_ptr<T>(new T());
        }
        return resources[sym].lock();
    }
    static std::shared_ptr<T> get(t_symbol *sym) {
        typename resource_map::const_iterator it = resources.find(sym);
        if (it == resources.end()) {
            return nullptr;
        }
        return resources[sym].lock();
    }
};

class glwindow_buffer {
public:
    glwindow_buffer();
    ~glwindow_buffer();
    GLuint buffer;
    static resource_list<glwindow_buffer> buffers;
};

class glwindow_vertexarray {
    
};

class glwindow_framebuffer {
public:
    glwindow_framebuffer();
    ~glwindow_framebuffer();
    GLuint buffer;
    static resource_list<glwindow_framebuffer> framebuffers;
};

class glwindow_program {
public:
    glwindow_program();
    ~glwindow_program();
    GLuint program;
    static resource_list<glwindow_program> programs;
};

class glwindow_texture {
public:
    glwindow_texture();
    ~glwindow_texture();
    GLuint texture;
    static resource_list<glwindow_texture> textures;
};


struct t_gl_win_obj;
struct t_gl_renderhead_obj;

typedef std::list<t_gl_win_obj *> win_obj_list;
typedef std::list<t_gl_renderhead_obj *> render_head_list;

struct glwindow_cpp {
    std::list<t_gl_win_obj *> win_objs;
    std::list<t_gl_renderhead_obj *> renderheads;
};

struct glwindow {
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
    bool reset;
    int refcount;
    glwindow_cpp *cpp;
    

    static std::map<t_symbol *, struct glwindow *> windows;
};

extern t_symbol *default_window;
glwindow *gl_find_window(t_symbol *name);

#endif
