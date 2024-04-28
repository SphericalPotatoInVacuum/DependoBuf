#pragma once

//Handled errors
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
//Decodes error by its index.
const char *GetErrorDescription(int err_no);