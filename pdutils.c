//
//  pdutils.c
//  glance
//
//  Created by Tiago Rezende on 3/11/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "pdutils.h"



char *list_to_string(int argc, t_atom *argv) {
    char str[MAXPDSTRING];
    char buf[MAXPDSTRING];
    str[0] = 0;
    if (argc > 0) {
        atom_string(argv, buf, MAXPDSTRING);
        if (argc > 1) {
            sprintf(str, "%s %s", buf, list_to_string(argc-1, argv+1));
        } else {
            sprintf(str, "%s", buf);
        }
    }
    return strdup(str);
}
