/**
  *Used to export symbols for win systems
  */

#if (UNIX)
#ifdef lms_EXPORTS
#define lms_EXPORT __declspec(dllexport)
#else
#define lms_EXPORT __declspec(dllimport)
#endif
#else
#define lms_EXPORT
#endif
