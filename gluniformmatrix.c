//
//  gluniformmatrix.c
//  glance
//
//  Created by Tiago Rezende on 3/18/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <SDL.h>
#include <OpenGL/OpenGL.h>
#include <OpenGl/gl3.h>
#include "glance.h"

/*
 * Matrix operations translated from the glmatrix javascript library
 * http://glmatrix.net
 */

static t_class
    *gl_uniform_mtx2f_class = NULL,
    *gl_uniform_mtx3f_class = NULL,
    *gl_uniform_mtx4f_class = NULL;


typedef struct _gl_uniform_mtx2f {
    t_object x_obj;
    GLint location;
    bool transpose;
    t_symbol *name;
    t_float value[4];
    //t_inlet *in_val[4]; // though 4 is a manageable inlet count, this won't be
    // reasonable for matrices
    t_outlet *out;
} t_gl_uniform_mtx2f;

typedef struct _gl_uniform_mtx3f {
    t_object x_obj;
    GLint location;
    bool transpose;
    t_symbol *name;
    t_float value[9];
    //t_inlet *in_val[16];
    t_outlet *out;
} t_gl_uniform_mtx3f;

typedef struct _gl_uniform_mtx4f {
    t_object x_obj;
    GLint location;
    bool transpose;
    t_symbol *name;
    t_float value[16];
    //t_inlet *in_val[16];
    t_outlet *out;
} t_gl_uniform_mtx4f;


static void *gl_uniform_mtx2f_new(t_symbol *unifname) {
    t_gl_uniform_mtx2f *obj = (t_gl_uniform_mtx2f *)pd_new(gl_uniform_mtx2f_class);
    obj->location = -1;
    for (int j = 0; j<2; j++) {
        for (int i = 0; i<2; i++) {
            obj->value[j*2+i] = (j==i)?1.0:0.0;
        }
    }
    obj->transpose = false;
    obj->name = unifname;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void *gl_uniform_mtx3f_new(t_symbol *unifname) {
    t_gl_uniform_mtx3f *obj = (t_gl_uniform_mtx3f *)pd_new(gl_uniform_mtx3f_class);
    obj->location = -1;
    for (int j = 0; j<3; j++) {
        for (int i = 0; i<3; i++) {
            obj->value[j*3+i] = (j==i)?1.0:0.0;
        }
    }
    obj->transpose = false;
    obj->name = unifname;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void *gl_uniform_mtx4f_new(t_symbol *unifname) {
    t_gl_uniform_mtx4f *obj = (t_gl_uniform_mtx4f *)pd_new(gl_uniform_mtx4f_class);
    obj->location = -1;
    for (int j = 0; j<4; j++) {
        for (int i = 0; i<4; i++) {
            obj->value[j*4+i] = (j==i)?1.0:0.0;
        }
    }
    obj->transpose = false;
    obj->name = unifname;
    obj->out = outlet_new(&obj->x_obj, &s_anything);
    return (void *)obj;
}

static void gl_uniform_mtx2f_mtx(t_gl_uniform_mtx2f *obj,
                                 t_float v0, t_float v1,
                                 t_float v2, t_float v3) {
    obj->value[0] = v0; obj->value[1] = v1;
    obj->value[2] = v2; obj->value[3] = v3;
}
static void gl_uniform_mtx3f_mtx(t_gl_uniform_mtx3f *obj,
                                 t_symbol *sym, int argc, t_atom *argv) {
    if (argc < 9) {
        // fail silently?
    } else {
        for (int i = 0; i < 9; i++) {
            obj->value[i] = atom_getfloatarg(i, argc, argv);
        }
    }
}

static void gl_uniform_mtx4f_mtx(t_gl_uniform_mtx4f *obj,
                                 t_symbol *sym, int argc, t_atom *argv) {
    if (argc < 16) {
        // fail silently?
    } else {
        for (int i = 0; i < 16; i++) {
            obj->value[i] = atom_getfloatarg(i, argc, argv);
        }
    }
}


static void gl_uniform_mtx2f_setname(t_gl_uniform_mtx2f *obj, t_symbol *sym) {
    obj->location = -1; obj->name = sym;
}
static void gl_uniform_mtx3f_setname(t_gl_uniform_mtx3f *obj, t_symbol *sym) {
    obj->location = -1; obj->name = sym;
}
static void gl_uniform_mtx4f_setname(t_gl_uniform_mtx4f *obj, t_symbol *sym) {
    obj->location = -1; obj->name = sym;
}


static void gl_uniform_mtx2f_identity(t_gl_uniform_mtx2f *obj)
{
    for (int j = 0; j<2; j++) {
        for (int i = 0; i<2; i++) {
            obj->value[j*2+i] = (j==i)?1.0:0.0;
        }
    }
}

static void gl_uniform_mtx3f_identity(t_gl_uniform_mtx3f *obj)
{
    for (int j = 0; j<3; j++) {
        for (int i = 0; i<3; i++) {
            obj->value[j*3+i] = (j==i)?1.0:0.0;
        }
    }
}

static void gl_uniform_mtx4f_identity(t_gl_uniform_mtx4f *obj)
{
    for (int j = 0; j<4; j++) {
        for (int i = 0; i<4; i++) {
            obj->value[j*4+i] = (j==i)?1.0:0.0;
        }
    }
}

static void gl_uniform_mtx4f_set_translation(t_gl_uniform_mtx4f *obj,
                                             t_float x, t_float y, t_float z) {
    for (int j = 0; j<3; j++) {
        for (int i = 0; i<4; i++) {
            obj->value[j*4+i] = (j==i)?1.0:0.0;
        }
    }
    obj->value[12] = x;
    obj->value[13] = y;
    obj->value[14] = z;
    obj->value[15] = 1.0;
}

static void gl_uniform_mtx4f_set_translation4(t_gl_uniform_mtx4f *obj,
                                              t_float x, t_float y,
                                              t_float z, t_float w) {
    for (int j = 0; j<3; j++) {
        for (int i = 0; i<4; i++) {
            obj->value[j*4+i] = (j==i)?1.0:0.0;
        }
    }
    obj->value[12] = x;
    obj->value[13] = y;
    obj->value[14] = z;
    obj->value[15] = w;
}

static void gl_uniform_mtx4f_translate(t_gl_uniform_mtx4f *obj,
                                       t_float x, t_float y, t_float z) {
    obj->value[12] = obj->value[0]*x + obj->value[4]*y + obj->value[8]*z + obj->value[12];
    obj->value[13] = obj->value[1]*x + obj->value[5]*y + obj->value[9]*z + obj->value[13];
    obj->value[14] = obj->value[2]*x + obj->value[6]*y + obj->value[10]*z + obj->value[14];
    obj->value[15] = obj->value[3]*x + obj->value[7]*y + obj->value[11]*z + obj->value[15];
}

static void gl_uniform_mtx4f_translate4(t_gl_uniform_mtx4f *obj,
                                        t_float x, t_float y, t_float z, t_float w) {
    obj->value[12] = obj->value[0]*x + obj->value[4]*y + obj->value[8]*z + obj->value[12]*w;
    obj->value[13] = obj->value[1]*x + obj->value[5]*y + obj->value[9]*z + obj->value[13]*w;
    obj->value[14] = obj->value[2]*x + obj->value[6]*y + obj->value[10]*z + obj->value[14]*w;
    obj->value[15] = obj->value[3]*x + obj->value[7]*y + obj->value[11]*z + obj->value[15]*w;
}

static void gl_uniform_mtx4f_set_scale(t_gl_uniform_mtx4f *obj,
                                       t_float x, t_float y, t_float z) {
    for (int j = 0; j<4; j++) {
        for (int i = 0; i<4; i++) {
            obj->value[j*4+i] = 0.0;
        }
    }
    obj->value[0] = x;
    obj->value[5] = y;
    obj->value[10] = z;
    obj->value[15] = 1.0;
}

static void gl_uniform_mtx4f_set_scale4(t_gl_uniform_mtx4f *obj,
                                       t_float x, t_float y, t_float z, t_float w) {
    for (int j = 0; j<4; j++) {
        for (int i = 0; i<4; i++) {
            obj->value[j*4+i] = 0.0;
        }
    }
    obj->value[0] = x;
    obj->value[5] = y;
    obj->value[10] = z;
    obj->value[15] = w;
}


static void gl_uniform_mtx4f_scale(t_gl_uniform_mtx4f *obj,
                                   t_float x, t_float y, t_float z) {
    obj->value[0] *= x;
    obj->value[1] *= x;
    obj->value[2] *= x;
    obj->value[3] *= x;

    obj->value[4] *= y;
    obj->value[5] *= y;
    obj->value[6] *= y;
    obj->value[7] *= y;
    
    obj->value[8] *= z;
    obj->value[9] *= z;
    obj->value[10] *= z;
    obj->value[11] *= z;
}
static void gl_uniform_mtx4f_scale4(t_gl_uniform_mtx4f *obj,
                                   t_float x, t_float y, t_float z, t_float w) {
    obj->value[0] *= x;
    obj->value[1] *= x;
    obj->value[2] *= x;
    obj->value[3] *= x;
    
    obj->value[4] *= y;
    obj->value[5] *= y;
    obj->value[6] *= y;
    obj->value[7] *= y;
    
    obj->value[8] *= z;
    obj->value[9] *= z;
    obj->value[10] *= z;
    obj->value[11] *= z;

    obj->value[12] *= w;
    obj->value[13] *= w;
    obj->value[14] *= w;
    obj->value[15] *= w;
}

static void gl_uniform_mtx4f_rotate(t_gl_uniform_mtx4f *obj,
                                    t_float angle, t_float x, t_float y, t_float z) {
    t_float
        a00 = obj->value[0],  a01 = obj->value[1],  a02 = obj->value[2],  a03 = obj->value[3],
        a10 = obj->value[4],  a11 = obj->value[5],  a12 = obj->value[6],  a13 = obj->value[7],
        a20 = obj->value[8],  a21 = obj->value[9],  a22 = obj->value[10], a23 = obj->value[11];
    t_float len = sqrtf(x*x + y*y + z*z);
    if (fabsf(len) < 0.00000001f) {
        // abort to avoid divide by zero
        return;
    }
    len = 1.0/len;
    x *= len;
    y *= len;
    z *= len;
    t_float s = sinf(angle), c = cosf(angle);
    t_float t = 1.0 - c;
    t_float
        b00 = x * x * t + c,
        b01 = y * x * t + z * s,
        b02 = z * x * t - y * s,
        b10 = x * y * t - z * s,
        b11 = y * y * t + c,
        b12 = z * y * t + x * s,
        b20 = x * z * t + y * s,
        b21 = y * z * t - x * s,
        b22 = z * z * t + c;
    
    obj->value[0]  = a00 * b00 + a10 * b01 + a20 * b02;
    obj->value[1]  = a01 * b00 + a11 * b01 + a21 * b02;
    obj->value[2]  = a02 * b00 + a12 * b01 + a22 * b02;
    obj->value[3]  = a03 * b00 + a13 * b01 + a23 * b02;
    obj->value[4]  = a00 * b10 + a10 * b11 + a20 * b12;
    obj->value[5]  = a01 * b10 + a11 * b11 + a21 * b12;
    obj->value[6]  = a02 * b10 + a12 * b11 + a22 * b12;
    obj->value[7]  = a03 * b10 + a13 * b11 + a23 * b12;
    obj->value[8]  = a00 * b20 + a10 * b21 + a20 * b22;
    obj->value[9]  = a01 * b20 + a11 * b21 + a21 * b22;
    obj->value[10] = a02 * b20 + a12 * b21 + a22 * b22;
    obj->value[11] = a03 * b20 + a13 * b21 + a23 * b22;
}

static void gl_uniform_mtx4f_rotate_x(t_gl_uniform_mtx4f *obj, t_float angle) {
    t_float s = sinf(angle), c = cosf(angle);
    t_float
        a10 = obj->value[4],
        a11 = obj->value[5],
        a12 = obj->value[6],
        a13 = obj->value[7],
        a20 = obj->value[8],
        a21 = obj->value[9],
        a22 = obj->value[10],
        a23 = obj->value[11];
    obj->value[4] = a10 * c + a20 * s;
    obj->value[5] = a11 * c + a21 * s;
    obj->value[6] = a12 * c + a22 * s;
    obj->value[7] = a13 * c + a23 * s;
    obj->value[8] = a20 * c - a10 * s;
    obj->value[9] = a21 * c - a11 * s;
    obj->value[10] = a22 * c - a12 * s;
    obj->value[11] = a23 * c - a13 * s;
}

static void gl_uniform_mtx4f_rotate_y(t_gl_uniform_mtx4f *obj, t_float angle) {
    t_float s = sinf(angle), c = cosf(angle);
    t_float
        a00 = obj->value[0],
        a01 = obj->value[1],
        a02 = obj->value[2],
        a03 = obj->value[3],
        a20 = obj->value[8],
        a21 = obj->value[9],
        a22 = obj->value[10],
        a23 = obj->value[11];
    obj->value[4] =  a00 * c - a20 * s;
    obj->value[5] =  a01 * c - a21 * s;
    obj->value[6] =  a02 * c - a22 * s;
    obj->value[7] =  a03 * c - a23 * s;
    obj->value[8] =  a20 * c + a00 * s;
    obj->value[9] =  a21 * c + a01 * s;
    obj->value[10] = a22 * c + a02 * s;
    obj->value[11] = a23 * c + a03 * s;
}

static void gl_uniform_mtx4f_rotate_z(t_gl_uniform_mtx4f *obj, t_float angle) {
    t_float s = sinf(angle), c = cosf(angle);
    t_float
        a00 = obj->value[0],
        a01 = obj->value[1],
        a02 = obj->value[2],
        a03 = obj->value[3],
        a10 = obj->value[4],
        a11 = obj->value[5],
        a12 = obj->value[6],
        a13 = obj->value[7];
    obj->value[4] =  a00 * c + a10 * s;
    obj->value[5] =  a01 * c + a11 * s;
    obj->value[6] =  a02 * c + a12 * s;
    obj->value[7] =  a03 * c + a13 * s;
    obj->value[8] =  a10 * c - a00 * s;
    obj->value[9] =  a11 * c - a01 * s;
    obj->value[10] = a12 * c - a02 * s;
    obj->value[11] = a13 * c - a03 * s;
}

static void gl_uniform_mtx4f_frustum(t_gl_uniform_mtx4f *obj,
                                     t_symbol *sym, int argc, t_atom *argv) {
    if (argc < 6) {
        return;
    }
    t_float
        left = atom_getfloatarg(0, argc, argv),
        right = atom_getfloatarg(1, argc, argv),
        bottom = atom_getfloatarg(2, argc, argv),
        top = atom_getfloatarg(3, argc, argv),
        near = atom_getfloatarg(4, argc, argv),
        far = atom_getfloatarg(5, argc, argv),
        rl = 1.0 / (right - left),
        tb = 1.0 / (top - bottom),
        nf = 1.0 / (near - far);
    obj->value[0]  = (near * 2.0) * rl;
    obj->value[1]  = 0;
    obj->value[2]  = 0;
    obj->value[3]  = 0;
    obj->value[4]  = 0;
    obj->value[5]  = (near * 2.0) * tb;
    obj->value[6]  = 0;
    obj->value[7]  = 0;
    obj->value[8]  = (right + left) * rl;
    obj->value[9]  = (top + bottom) * tb;
    obj->value[10] = (far + near) * nf;
    obj->value[11] = -1;
    obj->value[12] = 0;
    obj->value[13] = 0;
    obj->value[14] = (far * near * 2) * nf;
    obj->value[15] = 0;
}

static void gl_uniform_mtx4f_perspective(t_gl_uniform_mtx4f *obj,
                                         t_float fovy, t_float aspect,
                                         t_float near, t_float far) {
    
    t_float f = 1.0/tanf(fovy/2.0), nf = 1.0/(near - far);
    obj->value[0]  = f / aspect;
    obj->value[1]  = 0;
    obj->value[2]  = 0;
    obj->value[3]  = 0;
    obj->value[4]  = 0;
    obj->value[5]  = f;
    obj->value[6]  = 0;
    obj->value[7]  = 0;
    obj->value[8]  = 0;
    obj->value[9]  = 0;
    obj->value[10] = (far + near) * nf;
    obj->value[11] = -1;
    obj->value[12] = 0;
    obj->value[13] = 0;
    obj->value[14] = (far * near * 2) * nf;
    obj->value[15] = 0;
}

static void gl_uniform_mtx4f_ortho(t_gl_uniform_mtx4f *obj,
                                   t_symbol *sym, int argc, t_atom *argv) {
    if (argc < 6) {
        return;
    }
    t_float
        left = atom_getfloatarg(0, argc, argv),
        right = atom_getfloatarg(1, argc, argv),
        bottom = atom_getfloatarg(2, argc, argv),
        top = atom_getfloatarg(3, argc, argv),
        near = atom_getfloatarg(4, argc, argv),
        far = atom_getfloatarg(5, argc, argv),
        lr = 1.0 / (left - right),
        bt = 1.0 / (bottom - top),
        nf = 1.0 / (near - far);
    obj->value[0]  = -2.0 * lr;
    obj->value[1]  = 0;
    obj->value[2]  = 0;
    obj->value[3]  = 0;
    obj->value[4]  = 0;
    obj->value[5]  = -2.0 * bt;
    obj->value[6]  = 0;
    obj->value[7]  = 0;
    obj->value[8]  = 0;
    obj->value[9]  = 0;
    obj->value[10] = 2.0 * nf;
    obj->value[11] = 0;
    obj->value[12] = (left + right) * lr;
    obj->value[13] = (top + bottom) * bt;
    obj->value[14] = (far * near) * nf;
    obj->value[15] = 1;
}

static void gl_uniform_mtx2f_render(t_gl_uniform_mtx2f *obj,
                                    t_symbol *s, int argc, t_atom *argv) {
    GLint program;
    if (obj->location == -1) {
        glGetIntegeri_v(GL_CURRENT_PROGRAM, 0, &program);
        obj->location = glGetUniformLocation(program, obj->name->s_name);
    }
    glUniformMatrix2fv(obj->location,
                       1, obj->transpose, obj->value);
    outlet_anything(obj->out, s, argc, argv);
}
static void gl_uniform_mtx3f_render(t_gl_uniform_mtx3f *obj,
                                    t_symbol *s, int argc, t_atom *argv) {
    GLint program;
    if (obj->location == -1) {
        glGetIntegeri_v(GL_CURRENT_PROGRAM, 0, &program);
        obj->location = glGetUniformLocation(program, obj->name->s_name);
    }
    glUniformMatrix3fv(obj->location,
                       1, obj->transpose, obj->value);
    outlet_anything(obj->out, s, argc, argv);
}
static void gl_uniform_mtx4f_render(t_gl_uniform_mtx4f *obj,
                                    t_symbol *s, int argc, t_atom *argv) {
    GLint program;
    if (obj->location == -1) {
        glGetIntegeri_v(GL_CURRENT_PROGRAM, 0, &program);
        obj->location = glGetUniformLocation(program, obj->name->s_name);
    }
    glUniformMatrix4fv(obj->location,
                       1, obj->transpose, obj->value);
    outlet_anything(obj->out, s, argc, argv);
}

void gl_uniform_matrix_setup(void)
{
    gl_uniform_mtx2f_class = class_new(gensym("gl.uniformmatrix2f"),
                                       (t_newmethod)gl_uniform_mtx2f_new,
                                       0,
                                       sizeof(t_gl_uniform_mtx2f), CLASS_DEFAULT,
                                       A_SYMBOL, 0);
    class_addmethod(gl_uniform_mtx2f_class, (t_method)gl_uniform_mtx2f_mtx,
                    gensym("mtx"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx2f_class, (t_method)gl_uniform_mtx2f_setname,
                    gensym("name"), A_SYMBOL, 0);
    class_addmethod(gl_uniform_mtx2f_class, (t_method)gl_uniform_mtx2f_identity,
                    gensym("identity"), 0);
    class_addmethod(gl_uniform_mtx2f_class, (t_method)gl_uniform_mtx2f_render,
                    render, A_GIMME, 0);
    
    gl_uniform_mtx3f_class = class_new(gensym("gl.uniformmatrix3f"),
                                       (t_newmethod)gl_uniform_mtx3f_new,
                                       0,
                                       sizeof(t_gl_uniform_mtx3f), CLASS_DEFAULT,
                                       A_SYMBOL, 0);
    class_addmethod(gl_uniform_mtx3f_class, (t_method)gl_uniform_mtx3f_mtx,
                    gensym("mtx"), A_GIMME, 0);
    class_addmethod(gl_uniform_mtx3f_class, (t_method)gl_uniform_mtx3f_setname,
                    gensym("name"), A_SYMBOL, 0);
    class_addmethod(gl_uniform_mtx3f_class, (t_method)gl_uniform_mtx3f_identity,
                    gensym("identity"), 0);
    class_addmethod(gl_uniform_mtx3f_class, (t_method)gl_uniform_mtx3f_render,
                    render, A_GIMME, 0);
    
    gl_uniform_mtx4f_class = class_new(gensym("gl.uniformmatrix4f"),
                                       (t_newmethod)gl_uniform_mtx4f_new,
                                       0,
                                       sizeof(t_gl_uniform_mtx4f), CLASS_DEFAULT,
                                       A_SYMBOL, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_mtx,
                    gensym("mtx"), A_GIMME, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_setname,
                    gensym("name"), A_SYMBOL, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_identity,
                    gensym("identity"), 0);

    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_translate,
                    gensym("translate"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_translate4,
                    gensym("translate4"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_set_translation,
                    gensym("set-translation"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_set_translation4,
                    gensym("set-translation4"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_set_scale,
                    gensym("set-scale"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_set_scale4,
                    gensym("set-scale4"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_scale,
                    gensym("scale"), A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_scale4,
                    gensym("scale4"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);

    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_rotate,
                    gensym("rotate"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_rotate_x,
                    gensym("rotate-x"), A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_rotate_y,
                    gensym("rotate-y"), A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_rotate_z,
                    gensym("rotate-z"), A_FLOAT, 0);

    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_frustum,
                    gensym("frustum"), A_GIMME, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_perspective,
                    gensym("perspective"),
                    A_FLOAT, A_FLOAT,
                    A_FLOAT, A_FLOAT, 0);
    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_ortho,
                    gensym("ortho"), A_GIMME, 0);

    class_addmethod(gl_uniform_mtx4f_class, (t_method)gl_uniform_mtx4f_render,
                    render, A_GIMME, 0);
}

void gl_uniformmatrix2f_setup(void) { gl_uniform_matrix_setup(); }
void gl_uniformmatrix3f_setup(void) { gl_uniform_matrix_setup(); }
void gl_uniformmatrix4f_setup(void) { gl_uniform_matrix_setup(); }
