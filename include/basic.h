#ifndef BZR_BASIC_H
#define BZR_BASIC_H

#include <SDL.h>

#ifdef _MSC_VER
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#endif

#include <cassert>
#include <memory>
#include <stdexcept>

#ifdef __GNUC__
#define PACK(decl) decl __attribute__((__packed__))
#elif _MSC_VER
#define PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
#error Implement PACK for this compiler.
#endif

using namespace std;

#endif
