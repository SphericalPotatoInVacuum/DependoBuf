#include "serializer/error_handler/error_handler.h"

int err_code = NOERR;

const char *GetErrorDesription(int err_no) {
    if (err_no == NOERR) {
        return "No errors occurred\n";
    } else if (err_no == EMPTYLIST) { //NOLINT
        return "Popping from empty list\n";
    } else if (err_no == ALLOCERR) {
        return "Could not allocate space on heap\n";
    } else if (err_no == UNKNKIND) {
        return "Unknown kind in layout\n";
    } else if (err_no == SERSIZEERR) {
        return "Calculated size of serialized data doesn't correspond to its input\n";
    } else if (err_no == DESERERR) {
        return "Failed to deserialize data\n";
    } else if (err_no == CONSTRERR) {
        return "ConstructPrimitiveValue cannot create constructed value\n";
    } else {
        return "Unknown error\n";
    }
}