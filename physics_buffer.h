#ifndef PHYSICS_BUFFER_H
#define PHYSICS_BUFFER_H

#include <glad.h>

typedef int PBindex;

enum PBwhich {
    current = 0,
    other = 1
};

#define PB_1D GL_R32F
#define PB_2D GL_RG32F
#define PB_3D GL_RGB32F
#define PB_4D GL_RGBA32F

class PhysicsBuffer {
public:
    const char* name;
    PBindex current;
    PBindex other;
    int width;
    int height;
    PhysicsBuffer(const char* n, const GLuint* t, const GLuint* f) :
        name(n), width(0), height(0), textures(t), framebuffers(f), current(0), other(1), minmag_filter(GL_NONE), wrap_st(GL_NONE), dim(PB_1D) {};
    void allocate(const PBwhich which, const PBindex index, const int width, const int height, const char* data);
    void reallocate(const PBwhich which, const int width, const int height);
    void flip_buffers();
    // void bind();
    void bind(const PBwhich which);
    void update();
    void update(const int P);

    GLuint minmag_filter;
    GLuint wrap_st;
    GLuint dim;

private:
    const GLuint* textures;
    const GLuint* framebuffers;
    void set_index(const PBwhich which, const PBindex index);
    PBindex* which2index(const PBwhich which);
};

#endif /* PHYSICS_BUFFER_H */