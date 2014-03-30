#pragma once


namespace Z
{

    
typedef enum _result
{
    S_OK                = 0, 
    E_FAIL              = 1,
    
    E_INVALID_ARG       = 2,
    E_NULL_POINTER      = 3,
    E_OUTOFMEMORY       = 4,
    E_ACCESS_DENIED     = 5,
    E_BAD_HANDLE        = 6,
    E_UNEXPECTED        = 7,
    E_FILE_NOT_FOUND    = 8,
    E_NOT_FOUND         = 9,
    E_BAD_FILE_FORMAT   = 10,
    E_INVALID_OPERATION = 11,
    E_INVALID_DATA      = 12,
    E_OPENGL            = 13,
    E_NOTHING_TO_DO     = 14,
    E_EOF               = 15,
    E_ALREADY_EXISTS    = 16,

} RESULT;


#define SUCCEEDED(x)  \
        (Z::S_OK == (x) ? true : false) \


#define FAILED(x) \
        (Z::S_OK != (x) ? true : false) \



#define IGNOREHR(x)     (x)


//
// Check boolean for failure
//
#define CBR(x)  \
    { \
        if ( (x) != true) \
        { \
            rval = Z::E_FAIL; \
            goto Exit; \
        } \
    }

#define CBREx(x, r)  \
    { \
        if ( (x) != true) \
        { \
            rval = r; \
            goto Exit; \
        } \
    }



//
// Check pointer for NULL
//
#define CPR(x)  \
    { \
        if ( NULL == (x) ) \
        { \
            rval = Z::E_NULL_POINTER; \
            goto Exit; \
        } \
    }

#define CPREx(x, r)  \
    { \
        if ( NULL == (x) ) \
        { \
            rval = r; \
            goto Exit; \
        } \
}



//
// Check RESULT for failure
//
#define CHR(x)  \
    { \
        rval = x; \
        if ( SUCCEEDED(rval) != true) \
        { \
            goto Exit; \
        } \
    }

#define CHREx(x, r)  \
    { \
        rval = x; \
        if ( SUCCEEDED(rval) != true) \
        { \
            rval = r; \
            goto Exit; \
        } \
    }

} // END namespace Z


