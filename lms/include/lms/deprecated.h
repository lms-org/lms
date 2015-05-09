/**
  *Taken from http://stackoverflow.com/questions/295120/c-mark-as-deprecated/21265197#21265197
  */
#ifndef DEPRECATED
    #ifdef __GNUC__
        #define DEPRECATED __attribute__((deprecated))
    #elif defined(_MSC_VER)
        #define DEPRECATED __declspec(deprecated)
    #else
        #pragma message("WARNING: You need to implement DEPRECATED for this compiler")
        #define DEPRECATED
    #endif
#endif
