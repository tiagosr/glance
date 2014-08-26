//
//  glsketch.c
//  glance
//
//  Created by Tiago Rezende on 7/10/14.
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
#include <map>
#include <list>
#include <vector>
#include "window.hh"

static t_class* gl_sketch_class = NULL;

typedef struct _gl_sketch t_gl_sketch;
typedef struct _gl_sketch_command t_gl_sketch_command;

struct gl_sketch_command {
    bool init;
    bool enabled;
    bool correct;
    void *data;
    size_t size;
    gl_sketch_command *set(t_symbol *sym, t_int argc, t_atom *argv) {
        this->sym = sym;
        this->argc = argc;
        this->argv = (t_atom *)copybytes(argv, sizeof(t_atom)*argc);
        this->correct = true;
        this->init = false;
        this->enabled = true;
        return this;
    }
    virtual void reset(t_gl_sketch *sketch) {};
    virtual void perform(t_gl_sketch *sketch) {};
    virtual ~gl_sketch_command() {};
    t_symbol *sym;
    t_int argc;
    t_atom *argv;
};

struct gl_sketch_command_creator_base {
    virtual ~gl_sketch_command_creator_base(){}
    virtual gl_sketch_command * create() = 0;
};

template <class T>
class gl_creator: public gl_sketch_command_creator_base {
public:
    virtual gl_sketch_command * create() {
        return new T;
    }
};

struct gl_sketch_command_factory {
    typedef std::map<t_symbol *, gl_sketch_command_creator_base *> creators_map;
    creators_map creators;
    void _register(const char *name, gl_sketch_command_creator_base *creator) {
        creators[gensym(name)] = creator;
    }
    gl_sketch_command *create(t_symbol * name) {
        creators_map::const_iterator it = creators.find(name);
        if(it == creators.end()) {
            return nullptr;
        } else return it->second->create();
    }
};

static gl_sketch_command_factory factory;

typedef std::list<gl_sketch_command*> gl_command_list;
struct _gl_sketch {
    t_object x_obj;
    bool already_reset;
    glwindow *window;
    t_outlet *out, *error_out;
    gl_command_list commands;
};

/* commands */

static t_sym_uint_list gl_switch_modes[] = {
    {"depth_test", GL_DEPTH_TEST},
    {"stencil_test", GL_STENCIL_TEST},
    {"scissor_test", GL_SCISSOR_TEST},
    {"blend", GL_BLEND},
    {"line_smooth", GL_LINE_SMOOTH},
    {"multisample", GL_MULTISAMPLE},
    {"cull_face", GL_CULL_FACE},
    {"program_point_size", GL_PROGRAM_POINT_SIZE},
    {"polygon_offset_fill", GL_POLYGON_OFFSET_FILL},
    {"polygon_offset_line", GL_POLYGON_OFFSET_LINE},
    {"polygon_offset_point", GL_POLYGON_OFFSET_POINT},
    {"clip_distance0", GL_CLIP_DISTANCE0},
    {"clip_distance1", GL_CLIP_DISTANCE1},
    {"clip_distance2", GL_CLIP_DISTANCE2},
    {"clip_distance3", GL_CLIP_DISTANCE3},
    {"clip_distance4", GL_CLIP_DISTANCE4},
    {"clip_distance5", GL_CLIP_DISTANCE5},
    {"clip_distance6", GL_CLIP_DISTANCE6},
    {"clip_distance7", GL_CLIP_DISTANCE7},
    {"primitive_restart", GL_PRIMITIVE_RESTART},
    {"sample_alpha_to_coverage", GL_SAMPLE_ALPHA_TO_COVERAGE},
    {"sample_alpha_to_one", GL_SAMPLE_ALPHA_TO_ONE},
    {"sample_coverage", GL_SAMPLE_COVERAGE},
    {"sample_mask", GL_SAMPLE_MASK},
    {"texture_cube_map_seamless", GL_TEXTURE_CUBE_MAP_SEAMLESS},
    {0,0}
};

struct gl_sketch_command_enable: public gl_sketch_command {
    GLenum mode;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 1) {
            if (!find_uint_for_sym(gl_switch_modes, atom_getsymbol(argv), &mode)) {
                error("glenable: unrecognized capability\n");
                correct = false;
            }
        } else {
            error("glenable: expected capability\n");
        }
    }
    void perform(t_gl_sketch *sketch) { glEnable(mode); }
};
struct gl_sketch_command_disable: public gl_sketch_command {
    GLenum mode;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 1) {
            if (!find_uint_for_sym(gl_switch_modes, atom_getsymbol(argv), &mode)) {
                error("gldisable: unrecognized capability\n");
                correct = false;
            }
        } else {
            error("gldisable: expected capability\n");
        }
    }
    void perform(t_gl_sketch *sketch) { glDisable(mode); }
};

struct gl_sketch_command_clear_color: public gl_sketch_command {
    GLfloat r, g, b, a;
    void reset(t_gl_sketch *sketch) {
        if(argc>=3) {
            if(argc >= 4) {
                a = atom_getfloat(argv+3);
            }
            r = atom_getfloat(argv);
            g = atom_getfloat(argv+1);
            b = atom_getfloat(argv+2);
        }
    }
    void perform(t_gl_sketch *sketch) {
        glClearColor(r, g, b, a);
    }
};

static t_sym_uint_list gl_clear_modes[] = {
    {"color_buffer_bit", GL_COLOR_BUFFER_BIT},
    {"depth_buffer_bit", GL_DEPTH_BUFFER_BIT},
    {"stencil_buffer_bit", GL_STENCIL_BUFFER_BIT},
    {0,0}
};
struct gl_sketch_command_clear: public gl_sketch_command {
    GLbitfield modes;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        for(int i = 0; i < argc; i++) {
            GLbitfield found;
            if(find_uint_for_sym(gl_clear_modes,
                                 atom_getsymbolarg(i, argc, argv), &found)) {
                modes |= found;
            }
        }
    }
    void perform(t_gl_sketch *sketch) { glClear(modes); }
};

static t_sym_uint_list gl_bind_buffer_targets[] = {
    {"array_buffer", GL_ARRAY_BUFFER},
    {"copy_read_buffer", GL_COPY_READ_BUFFER},
    {"copy_write_buffer", GL_COPY_WRITE_BUFFER},
    {"element_array_buffer", GL_ELEMENT_ARRAY_BUFFER},
    {"pixel_pack_buffer", GL_PIXEL_PACK_BUFFER},
    {"pixel_unpack_buffer", GL_PIXEL_UNPACK_BUFFER},
    {"texture_buffer", GL_TEXTURE_BUFFER},
    {"transform_feedback_buffer", GL_TRANSFORM_FEEDBACK_BUFFER},
    {"uniform_buffer", GL_UNIFORM_BUFFER},
    {0,0}
};
struct gl_sketch_command_bind_buffer: public gl_sketch_command {
    GLenum target;
    std::shared_ptr<glwindow_buffer> buffer;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if(argc == 2) {
            if(find_uint_for_sym(gl_bind_buffer_targets, atom_getsymbol(argv), &target)) {
                t_symbol *name = atom_getsymbolarg(1, argc, argv);
                if (!name) {
                    error("glbindbuffer: buffer name required\n");
                    correct = false;
                } else {
                    if (!(buffer = glwindow_buffer::buffers.get(name))) {
                        error("glbindbuffer: buffer name %s not found for this window\n", name->s_name);
                        correct = false;
                    }
                }
            } else {
                error("glbindbuffer: target not recognized\n");
                correct = false;
            }
        } else {
            error("glbindbuffer: wrong number of arguments (expected 2)\n");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glBindBuffer(target, buffer->buffer);
    }
};
struct gl_sketch_command_unbind_buffer: public gl_sketch_command {
    GLenum target;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 1) {
            if (!find_uint_for_sym(gl_bind_buffer_targets, atom_getsymbol(argv), &target)) {
                error("glunbindbuffer: target name not recognized\n");
                correct = false;
            }
        } else {
            error("glunbindbuffer: wrong number of arguments (2 expected)\n");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glBindBuffer(target, 0);
    }
};

struct gl_sketch_command_use_program: public gl_sketch_command {
    std::shared_ptr<glwindow_program> program;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 1) {
            t_symbol *name = atom_getsymbol(argv);
            if (name) {
                if (!(program = glwindow_program::programs.get(name))) {
                    error("glbindprogram: program named %s not found\n", name->s_name);
                    correct = false;
                }
            } else {
                error("glbindprogram: program name not specified\n");
                correct = false;
            }
            
        } else {
            error("glbindprogram: wrong number arguments (1 expected)\n");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glUseProgram(program->program);
    }
};

static t_sym_uint_list gl_texture_targets[] = {
    {"texture_1d", GL_TEXTURE_1D},
    {"texture_2d", GL_TEXTURE_2D},
    {"texture_3d", GL_TEXTURE_3D},
    {0,0}
};
struct gl_sketch_command_bind_texture: public gl_sketch_command {
    GLenum target;
    std::shared_ptr<glwindow_texture> tex;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 2) {
            GLenum target;
            if (find_uint_for_sym(gl_texture_targets, atom_getsymbol(argv), &target)) {
                t_symbol *name = atom_getsymbol(argv+1);
                if (name) {
                    if(!(tex = glwindow_texture::textures.get(name))) {
                        error("glbindtexture: texture name %s not found\n", name->s_name);
                        correct = false;
                    }
                } else {
                    error("glbindtexture: expected texture name\n");
                    correct = false;
                }
            }
        } else {
            error("glbindtexture: expected texture target and texture name\n");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glBindTexture(target, tex->texture);
    }
};
struct gl_sketch_command_unbind_texture: public gl_sketch_command {
    GLenum target;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 1) {
            GLenum target;
            if (!find_uint_for_sym(gl_texture_targets, atom_getsymbol(argv), &target)) {
                error("glbindtexture: expected texture target\n");
            }
        } else {
            error("glbindtexture: expected texture target\n");
        }
    }
    void perform(t_gl_sketch *sketch) {
        glBindTexture(target, 0);
    }
};

static t_sym_uint_list gl_draw_modes[] = {
    {"points",GL_POINTS},
    {"lines", GL_LINES},
    {"line_strip", GL_LINE_STRIP},
    {"line_loop", GL_LINE_LOOP},
    {"triangles", GL_TRIANGLES},
    {"triangle_strip", GL_TRIANGLE_STRIP},
    {"triangle_fan", GL_TRIANGLE_FAN},
    {"quads", GL_QUADS},
    {"lines_adjacency", GL_LINES_ADJACENCY},
    {"line_strip_adjacency", GL_LINE_STRIP_ADJACENCY},
    {"triangles_adjacency", GL_TRIANGLES_ADJACENCY},
    {"triangle_strip_adjacency", GL_TRIANGLE_STRIP_ADJACENCY},
    {0,0}
};
static t_sym_uint_list gl_index_array_types[] = {
    {"unsigned_byte", GL_UNSIGNED_BYTE},
    {"unsigned_short", GL_UNSIGNED_SHORT},
    {"unsigned_int", GL_UNSIGNED_INT},
    {0,0}
};
struct gl_sketch_command_draw_arrays: public gl_sketch_command {
    GLenum mode;
    GLint first;
    GLsizei count;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 3) {
            if (!find_uint_for_sym(gl_draw_modes, atom_getsymbol(argv), &mode)) {
                error("glbindtexture: expected texture target\n");
                correct = false;
            } else {
                first = atom_getint(argv+1);
                count = atom_getint(argv+2);
            }
        } else {
            error("glbindtexture: expected draw mode, first index and count\n");
            correct = true;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glDrawArrays(mode, first, count);
    }
};
struct gl_sketch_command_draw_elements: public gl_sketch_command {
    GLenum mode;
    GLsizei count;
    GLenum type;
    GLsizei offset;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc > 2) {
            if (!find_uint_for_sym(gl_draw_modes, atom_getsymbol(argv), &mode)) {
                error("glbindtexture: expected texture target\n");
                correct = false;
            } else {
                count = atom_getint(argv+1);
                if (!find_uint_for_sym(gl_index_array_types, atom_getsymbol(argv+2), &type)) {
                    error("gldrawelements: expected index array type");
                    correct = false;
                } else {
                    offset = atom_getintarg(3, argc, argv);
                }
            }
        } else {
            error("glbindtexture: expected draw mode, first index and count\n");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glDrawElements(mode, count, type, (void*)offset);
    }
};

struct gl_sketch_command_viewport: public gl_sketch_command {
    GLint x, y;
    GLsizei w, h;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 4) {
            x = atom_getintarg(0, argc, argv);
            y = atom_getintarg(1, argc, argv);
            w = atom_getintarg(2, argc, argv);
            h = atom_getintarg(3, argc, argv);
        } else {
            error("glviewport: expected x, y, width and height\n");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glViewport(x, y, w, h);
    }
};
struct gl_sketch_command_scissor: public gl_sketch_command {
    GLint x, y;
    GLsizei w, h;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 4) {
            x = atom_getintarg(0, argc, argv);
            y = atom_getintarg(1, argc, argv);
            w = atom_getintarg(2, argc, argv);
            h = atom_getintarg(3, argc, argv);
        } else {
            error("glscissor: expected x, y, width and height\n");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glScissor(x, y, w, h);
    }
};
struct gl_sketch_command_depth_range: public gl_sketch_command {
    GLclampf near, far;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 2) {
            near = atom_getfloatarg(0, argc, argv);
            far = atom_getfloatarg(1, argc, argv);
        } else {
            error("glscissor: expected near and far values\n");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glDepthRangef(near, far);
    }
};

struct gl_sketch_command_flush: public gl_sketch_command {
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {}
    void perform(t_gl_sketch *sketch) {
        glFlush();
    }
};
struct gl_sketch_command_finish: public gl_sketch_command {
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {}
    void perform(t_gl_sketch *sketch) {
        glFinish();
    }
};

struct gl_sketch_command_color_mask: public gl_sketch_command {
    GLboolean r, g, b, a;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 4) {
            r = atom_getintarg(0, argc, argv)? GL_TRUE : GL_FALSE;
            g = atom_getintarg(1, argc, argv)? GL_TRUE : GL_FALSE;
            b = atom_getintarg(2, argc, argv)? GL_TRUE : GL_FALSE;
            a = atom_getintarg(3, argc, argv)? GL_TRUE : GL_FALSE;
        } else {
            error("glcolormask: expected red, green, blue and alpha mask values");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glColorMask(r, g, b, a);
    }
};
struct gl_sketch_command_depth_mask: public gl_sketch_command {
    GLboolean depth;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 1) {
            depth = atom_getintarg(0, argc, argv)? GL_TRUE : GL_FALSE;
        } else {
            error("gldepthmask: expected depth mask value (1/0)");
            correct = false;
        }
    }
    void perform(t_gl_sketch *sketch) {
        glDepthMask(depth);
    }
};

static t_sym_uint_list gl_logic_ops[] = {
    {"clear", GL_CLEAR},
    {"and", GL_AND},
    {"and_reverse", GL_AND_REVERSE},
    {"and_inverted", GL_AND_INVERTED},
    {"copy", GL_COPY},
    {"copy_inverted", GL_COPY_INVERTED},
    {"or", GL_OR},
    {"or_inverted", GL_OR_INVERTED},
    {"or_reverse", GL_OR_REVERSE},
    {"nand", GL_NAND},
    {"nor", GL_NOR},
    {"set", GL_SET},
    {"equiv", GL_EQUIV},
    {"invert", GL_INVERT},
    {"noop", GL_NOOP},
    {0,0}
};
struct gl_sketch_command_logic_op: public gl_sketch_command {
    GLenum mode;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 1) {
            if (!find_uint_for_sym(gl_logic_ops, atom_getsymbol(argv), &mode)) {
                error("gllogicop: unrecognized operation\n");
                correct = false;
            }
        } else {
            error("gllogicop: expected logic operation\n");
        }
    }
    void perform(t_gl_sketch *sketch) { glLogicOp(mode); }
};

static t_sym_uint_list gl_blend_modes[] = {
    {"zero", GL_ZERO},
    {"one",  GL_ONE},
    {"src_color",           GL_SRC_COLOR},
    {"one_minus_src_color", GL_ONE_MINUS_SRC_COLOR},
    {"src_alpha",           GL_SRC_ALPHA},
    {"one_minus_src_alpha", GL_ONE_MINUS_SRC_ALPHA},
    {"src_alpha_saturate",  GL_SRC_ALPHA_SATURATE},
    {"src1_color",           GL_SRC1_COLOR},
    {"one_minus_src1_color", GL_ONE_MINUS_SRC1_COLOR},
    {"src1_alpha",           GL_SRC1_ALPHA},
    {"one_minus_src1_alpha", GL_ONE_MINUS_SRC1_ALPHA},
    {"constant_color",           GL_CONSTANT_COLOR},
    {"constant_alpha",           GL_CONSTANT_ALPHA},
    {"one_minus_constant_color", GL_ONE_MINUS_CONSTANT_COLOR},
    {"one_minus_constant_alpha", GL_ONE_MINUS_CONSTANT_ALPHA},
    {0,0}
};
struct gl_sketch_command_blend_func: public gl_sketch_command {
    GLenum src_mode, dst_mode;
    void reset(t_gl_sketch *sketch, t_symbol *sym, t_int argc, t_atom *argv) {
        if (argc == 2) {
            if (!find_uint_for_sym(gl_blend_modes, atom_getsymbol(argv), &src_mode) ||
                !find_uint_for_sym(gl_blend_modes, atom_getsymbol(argv+1), &dst_mode)) {
                error("glblendfunc: unrecognized equation\n");
                correct = false;
            }
        } else {
            error("glblendfunc: expected source and destination blend equations\n");
        }
    }
    void perform(t_gl_sketch *sketch) { glBlendFunc(src_mode, dst_mode); }
};



/***************/


static void gl_sketch_do_reset(t_gl_sketch *sketch) {
    for (gl_command_list::const_iterator i = sketch->commands.begin();
         i != sketch->commands.end();
         i++) {
        (*i)->reset(sketch);
    }
    sketch->already_reset = true;
}

static void gl_sketch_do_render(t_gl_sketch *sketch, t_symbol *sym, int argc, t_atom *argv) {
    for (gl_command_list::const_iterator i = sketch->commands.begin();
         i != sketch->commands.end();
         i++) {
        if (!(*i)->init) {
            (*i)->reset(sketch);
        }
        if ((*i)->enabled && (*i)->correct) {
            (*i)->perform(sketch);
            GLenum err;
            t_symbol *sym;
            while ((err = glGetError()) != GL_NO_ERROR) {
                switch (err) {
                    case GL_INVALID_ENUM:
                        sym = gensym("INVALID_ENUM");
                        break;
                    case GL_INVALID_FRAMEBUFFER_OPERATION:
                        sym = gensym("INVALID_FRAMEBUFFER_OPERATION");
                        break;
                    case GL_INVALID_INDEX:
                        sym = gensym("INVALID_INDEX");
                        break;
                    case GL_INVALID_OPERATION:
                        sym = gensym("INVALID_OPERATION");
                        break;
                    case GL_INVALID_VALUE:
                        sym = gensym("INVALID_VALUE");
                        break;
                    case GL_OUT_OF_MEMORY:
                        sym = gensym("OUT_OF_MEMORY");
                        break;
                    default:
                        break;
                }
                outlet_symbol(sketch->error_out, sym);
            }
        }
    }
}

typedef gl_sketch_command*(*gl_sketch_generator)(t_symbol *sym, t_int argc, t_atom *argv);

static void gl_sketch_do_anything(t_gl_sketch *sketch, t_symbol *sym, int argc, t_atom *argv) {
    if (gensym("reset") == sym) {
        sketch->commands.clear();
    } else {
        gl_sketch_command *cmd = factory.create(sym);
        if (cmd) {
            sketch->commands.push_back(cmd);
        } else {
            
        }
    }
}

static t_gl_sketch *gl_sketch_new(t_symbol *sym, int argc, t_atom *argv) {
    t_gl_sketch *sketch = (t_gl_sketch*)pd_new(gl_sketch_class);
    t_symbol *name = default_window;
    if (argc > 0) {
        name = atom_getsymbol(argv);
    }
    sketch->window = gl_find_window(name);
    sketch->out = outlet_new(&sketch->x_obj, gensym("out"));
    sketch->error_out = outlet_new(&sketch->x_obj, gensym("error"));
    sketch->already_reset = false;
    return sketch;
}

static void gl_sketch_destroy(t_gl_sketch *sketch) {
    gl_sketch_do_anything(sketch, gensym("reset"), 0, NULL);
    outlet_free(sketch->out);
    outlet_free(sketch->error_out);
}

void gl_sketch_setup(void) {
    factory._register("glenable", new gl_creator<gl_sketch_command_enable>);
    factory._register("gldisable", new gl_creator<gl_sketch_command_disable>);
    factory._register("glclearcolor", new gl_creator<gl_sketch_command_clear_color>);
    factory._register("glclear", new gl_creator<gl_sketch_command_clear>);
    factory._register("glbindbuffer", new gl_creator<gl_sketch_command_bind_buffer>);
    factory._register("glunbindbuffer", new gl_creator<gl_sketch_command_unbind_buffer>);
    factory._register("gluseprogram", new gl_creator<gl_sketch_command_use_program>);
    factory._register("glbindtexture", new gl_creator<gl_sketch_command_bind_texture>);
    factory._register("glunbindtexture", new gl_creator<gl_sketch_command_unbind_texture>);
    factory._register("gldrawarrays", new gl_creator<gl_sketch_command_draw_arrays>);
    factory._register("gldrawelements", new gl_creator<gl_sketch_command_draw_elements>);
    factory._register("glviewport", new gl_creator<gl_sketch_command_viewport>);
    factory._register("glscissor", new gl_creator<gl_sketch_command_scissor>);
    factory._register("gldepthrange", new gl_creator<gl_sketch_command_depth_range>);
    factory._register("glflush", new gl_creator<gl_sketch_command_flush>);
    factory._register("glfinish", new gl_creator<gl_sketch_command_finish>);
    factory._register("gldepthmask", new gl_creator<gl_sketch_command_depth_mask>);
    factory._register("glcolormask", new gl_creator<gl_sketch_command_color_mask>);
    factory._register("gllogicop", new gl_creator<gl_sketch_command_logic_op>);
    factory._register("glblendfunc", new gl_creator<gl_sketch_command_blend_func>);
    
    
    gl_sketch_class = class_new(gensym("gl.sketch"),
                                (t_newmethod)gl_sketch_new,
                                (t_method)gl_sketch_destroy,
                                sizeof(t_gl_sketch),
                                CLASS_DEFAULT, A_GIMME, 0);
    class_addanything(gl_sketch_class, gl_sketch_do_anything);
    class_addmethod(gl_sketch_class, (t_method)gl_sketch_do_reset, reset, A_NULL, 0);
    class_addmethod(gl_sketch_class, (t_method)gl_sketch_do_render, render, A_GIMME, 0);
}