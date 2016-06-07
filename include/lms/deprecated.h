/**
  *Taken from
  *http://stackoverflow.com/questions/295120/c-mark-as-deprecated/21265197#21265197
  */
#ifndef LMS_DEPRECATED
#ifdef __GNUC__
#define LMS_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define LMS_DEPRECATED __declspec(deprecated)
#else
#define LMS_DEPRECATED
#endif
#endif
