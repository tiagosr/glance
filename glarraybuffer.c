//
//  glarraybuffer.c
//  glance
//
//  Created by Tiago Rezende on 1/19/14.
//  Copyright (c) 2014 Tiago Rezende. All rights reserved.
//

#include "m_pd.h"
#include <stdio.h>
#include <stdbool.h>
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#include "glance.h"

static t_class * gl_array_buffer_class = NULL;

