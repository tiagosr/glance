//
//  glance.h
//  glance
//
//  Created by Tiago Rezende on 3/11/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#ifndef glance_glance_h
#define glance_glance_h

#include "pdutils.h"

extern t_symbol *render, *reset, *cleanup;
void gl_win_setup(void);
void gl_sketch_setup(void);
void gl_program_setup(void);
void gl_vertexarray_setup(void);
void gl_array_buffer_setup(void);
void gl_uniform_setup(void);
void gl_uniform_matrix_setup(void);
void gl_test_setup(void);
void gl_error_setup(void);
void gl_info_setup(void);


#endif
