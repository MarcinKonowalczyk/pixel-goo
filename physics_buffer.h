#ifndef PHYSICS_BUFFER_H
#define PHYSICS_BUFFER_H

#include <glad.h>

typedef int PBindex;

enum PBwhich {
    current = 0,
    other = 1,
    screen = 2
};

typedef struct format {
    GLint format;
    GLint iformat;
} format;

#define PBE_1D format { .format = GL_RED, .iformat = GL_R32F}
#define PBE_2D format { .format = GL_RG, .iformat = GL_RG32F}
#define PBE_3D format { .format = GL_RGB, .iformat = GL_RGB32F}
#define PBE_4D format { .format = GL_RGBA, .iformat = GL_RGBA32F}

// #define PB_1D GL_R32F
// #define PB_2D GL_RG32F
// #define PB_3D GL_RGB32F
// #define PB_4D GL_RGBA32F

class PhysicsBuffer {
public:
    const char* name;
    PBindex current;
    PBindex other;
    int width;
    int height;
    PhysicsBuffer(const char* n, const GLuint* t, const GLuint* f) :
        name(n), width(0), height(0), textures(t), framebuffers(f), current(0), other(1), minmag_filter(GL_NONE), wrap_st(GL_NONE), dim(PBE_1D) {};
    void allocate(const PBwhich which, const PBindex index, const int width, const int height, const char* data);
    void reallocate(const PBwhich which, const int width, const int height);
    void flip_buffers();
    void bind(const PBwhich which);
    // void bind();
    void update();
    void update(const int P);

    GLuint minmag_filter;
    GLuint wrap_st;
    format dim;

private:
    const GLuint* textures;
    const GLuint* framebuffers;
    void set_index(const PBwhich which, const PBindex index);
    PBindex* which2index(const PBwhich which);
    void check_shape(const int width, const int height);
};

#endif /* PHYSICS_BUFFER_H */