//
//  gltest.c
//  glance
//
//  Created by Tiago Rezende on 1/8/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"

static t_class *gl_test_class;

typedef struct _gl_test t_gl_test;
struct _gl_test {
    t_object obj;
    bool initialized;
    GLuint vertexshader, pixelshader;
    GLuint testprogram;
    
    GLuint vertexarray;
    GLuint vertexarraybuffer;
    GLuint colorarray;
    GLuint colorarraybuffer;
};

static const char vertexprogram[] = "//precision mediump float;\n\
attribute vec4 position;\n\
attribute vec4 color;\n\
void main() {\n\
    gl_Position = position;\n\
    //gl_Color = color;\n\
}\n\
";

static const char colorprogram[] = "//precision mediump float;\n\
void main() {\n\
    gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);\n\
    gl_FragDepth = gl_FragCoord.z;\n\
}\n\
";

static const char * vertexprograms[] = {vertexprogram};
static const char * colorprograms[] = {colorprogram};

static void * gl_test_new(void) {
    t_gl_test *test_obj = (t_gl_test *)pd_new(gl_test_class);
    test_obj->initialized = false;
    return (void *)test_obj;
}
static void gl_test_destroy(t_gl_test *obj) {
    glDeleteBuffers(1, &obj->vertexarraybuffer);
    glDeleteVertexArrays(1, &obj->vertexarray);
}
static float vtxdata[] = {
    0.0, 0.5, 0.0, 0.0,
    0.5, -0.5, 0.0, 0.0,
    -0.5, -0.5, 0.0, 0.0,
};

static float colordata[] = {
    1.0, 0.0, 0.0, 1.0,
    0.0, 1.0, 0.0, 1.0,
    0.0, 0.0, 1.0, 1.0
};

static void gl_test_gen_vertexbuffer(t_gl_test *obj) {
    glGenBuffers(1, &obj->vertexarraybuffer);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vertexarraybuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtxdata), vtxdata, GL_STATIC_DRAW);
}

static void gl_test_gen_colorbuffer(t_gl_test *obj) {
    glGenBuffers(1, &obj->colorarraybuffer);
    glBindBuffer(GL_ARRAY_BUFFER, obj->colorarraybuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colordata), colordata, GL_STATIC_DRAW);
}

static void gl_test_initialize(t_gl_test *obj) {
    
    obj->pixelshader = glCreateShader(GL_FRAGMENT_SHADER);
    GLint colorprogramlengths[1] = {strlen(colorprogram)};
    glShaderSource(obj->pixelshader, 1, colorprograms, colorprogramlengths);
    glCompileShader(obj->pixelshader);
    GLint compile_status = 0;
    glGetShaderiv(obj->pixelshader, GL_COMPILE_STATUS, &compile_status);
    if (!compile_status) {
        GLint infologlength = 0;
        glGetShaderiv(obj->pixelshader, GL_INFO_LOG_LENGTH, &infologlength);
        char *log = malloc(infologlength+1);
        glGetShaderInfoLog(obj->pixelshader, infologlength, &infologlength, log);
        error("gl.test - fail on color program: %s", log);
        free(log);
    }
    
    obj->vertexshader = glCreateShader(GL_VERTEX_SHADER);
    GLint vertexprogramlengths[1] = {strlen(vertexprogram)};
    glShaderSource(obj->vertexshader, 1, vertexprograms, vertexprogramlengths);
    glCompileShader(obj->vertexshader);
    glGetShaderiv(obj->vertexshader, GL_COMPILE_STATUS, &compile_status);
    if (!compile_status) {
        GLint infologlength = 0;
        glGetShaderiv(obj->vertexshader, GL_INFO_LOG_LENGTH, &infologlength);
        char *log = malloc(infologlength+1);
        glGetShaderInfoLog(obj->vertexshader, infologlength, &infologlength, log);
        error("gl.test - fail on vertex program: %s", log);
        free(log);
    }
    
    obj->testprogram = glCreateProgram();
    glAttachShader(obj->testprogram, obj->vertexshader);
    glAttachShader(obj->testprogram, obj->pixelshader);
    
    glLinkProgram(obj->testprogram);
    glGetProgramiv(obj->testprogram, GL_LINK_STATUS, &compile_status);
    if (!compile_status) {
        GLint infologlength = 0;
        glGetProgramiv(obj->pixelshader, GL_INFO_LOG_LENGTH, &infologlength);
        char *log = malloc(infologlength+1);
        glGetShaderInfoLog(obj->pixelshader, infologlength, &infologlength, log);
        error("gl.test - fail on link: %s", log);
        free(log);
    }
    
    glGenVertexArrays(1, &obj->vertexarray);
    glBindVertexArray(obj->vertexarray);
    
    gl_test_gen_vertexbuffer(obj);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    
    gl_test_gen_colorbuffer(obj);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
}

static void gl_test_reset(t_gl_test *obj, t_symbol *s, int argc, t_atom* argv) {
    gl_test_initialize(obj);
    obj->initialized = true;
}

static void gl_test_render(t_gl_test *obj, t_symbol *s, int argc, t_atom* argv) {
    if (!obj->initialized) {
        gl_test_initialize(obj);
        obj->initialized = true;
    }
    GLint current_program = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
    glUseProgram(obj->testprogram);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glUseProgram(current_program);
}

void gl_test_setup(void) {
    gl_test_class = class_new(gensym("gl.test"),
                              (t_newmethod)gl_test_new,
                              (t_method)gl_test_destroy,
                              sizeof(t_gl_test),
                              CLASS_DEFAULT,
                              0);
    class_addmethod(gl_test_class,
                    (t_method)gl_test_render, render, A_GIMME, 0);
}
