#pragma once

#define SAFE_DELETE(X)          if(X) delete (X); (X) = nullptr;
#define SAFE_DELETE_ARRAY(X)    if(X) delete [] (X); (X) = nullptr;

#define REFLIB_ASSERT(COND, MSG) \
        assert((COND) && (MSG))
#define REFLIB_ASSERT_RETURN_IF_FAILED(COND, MSG)  \
        if (!(COND)) { REFLIB_ASSERT((COND), (MSG)); return; }
#define REFLIB_ASSERT_RETURN_VAL_IF_FAILED(COND, MSG, VAL) \
        if (!(COND)) { REFLIB_ASSERT((COND), (MSG)); return (VAL); }

void DebugPrint(char *format, ...);