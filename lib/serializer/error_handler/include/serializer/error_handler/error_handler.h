#pragma once

#include "stdio.h"

enum {
    NOERR,
    EMPTYLIST,
    ALLOCERR,
    UNKNKIND,
    SERSIZEERR,
    DESERERR,
    CONSTRERR
};

//Represents whether function ended normally or not.
extern int err_code;

char *GetErrorDescription(int err_no);