#ifdef lms_EXPORTS
#define lms_EXPORT __declspec(dllexport)
#else
#define lms_EXPORT __declspec(dllimport)
#endif