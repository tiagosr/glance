//
//  glance.c
//  glance
//
//  Created by Tiago Rezende on 3/5/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "glance.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 4

static t_class *glance_class;

t_symbol *render, *reset, *cleanup;

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

static void glance_glfw_error_callback(int err_num, const char* err_msg) {
    post("GLFW error(%d): %s", err_num, err_msg);
}


void glance_setup(void) {
    render = gensym("glance_render");
    reset = gensym("glance_reset");
    cleanup = gensym("glance_cleanup");
    
    // the glance object will be a configuration/messaging object, but right now
    // there's no functionality for it.
    glance_class = class_new(gensym("glance"),
                           (t_newmethod)glance_new,
                           0, sizeof(t_glance_obj),
                           CLASS_DEFAULT,
                           0);
    class_addbang(glance_class, glance_bang);
    

    glfwSetErrorCallback((GLFWerrorfun)glance_glfw_error_callback);
    int result = glfwInit();
    if (result != GL_TRUE) {
        post("GLFW failed to init");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    
    // Setting up Pd object classes
    gl_win_setup();
    gl_shader_setup();
    gl_vertexarray_setup();
    gl_array_buffer_setup();
    gl_draw_setup();
    gl_uniform_setup();
    gl_uniform_matrix_setup();
    gl_viewport_setup();
    gl_scissor_setup();
    gl_clear_setup();
    gl_error_setup();
    gl_info_setup();
    gl_test_setup();
    
    // Show copyright info when setup is finished
    post("glance: OpenGL/windowing/input libraries for PD\n"
         "        version %d.%d\n"
         "        (c)2013 Tiago Rezende\n", VERSION_MAJOR, VERSION_MINOR);
}