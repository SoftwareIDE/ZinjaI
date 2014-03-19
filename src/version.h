#define VERSION 20140319

#if defined(__WIN32__)
#define ARCHITECTURE "w32"
#elif defined(__WIN64__)
#define ARCHITECTURE "w64"
#elif defined(__APPLE__)
#define ARCHITECTURE "mac"
#elif defined(__linux__) && defined(__x86_64__)
#define ARCHITECTURE "l64"
#elif defined(__linux__) && defined(__i386__)
#define ARCHITECTURE "l32"
#elif defined(__x86_64__)
#define ARCHITECTURE "?64"
#elif defined(__i386__)
#define ARCHITECTURE "?32"
#else
#define ARCHITECTURE "???"
#endif

