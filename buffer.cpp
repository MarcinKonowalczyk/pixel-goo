#include "buffer.h"
#include <iostream>
#include <glad.h>

void Buffer::allocate(const Bwhich which, const PBindex index, const int width, const int height, const char* data) {
    this->check_shape(width, height);
    if (which == Bwhich::screen) {
        fprintf(stderr, "[%s] cannot allocate the screen buffer", this->name);
        exit(EXIT_FAILURE);
    }

    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, textures[index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->minmag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->minmag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrap_st);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrap_st);

    glTexImage2D(GL_TEXTURE_2D, 0, this->dim.iformat, width, height, 0, this->dim.format, GL_FLOAT, data);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[index]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[index], 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "[%s] framebuffer is not complete", this->name);
        exit(EXIT_FAILURE);
    }

    this->set_index(which, index);
    this->width = width;
    this->height = height;
}

void Buffer::reallocate(const Bwhich which, const int width, const int height) {
    this->allocate(which, this->current, width, height, NULL);
}

void Buffer::flip_buffers() {
    PBindex temp = this->current;
    this->current = this->other;
    this->other = temp;
}

void Buffer::bind(const Bwhich which) {
    GLint framebuffer;
    switch (which) {
        case Bwhich::current: { framebuffer = framebuffers[this->current]; } break;
        case Bwhich::other: { framebuffer = framebuffers[this->other]; } break;
        case Bwhich::screen: { framebuffer = 0; } break;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, width, height); // Change the viewport to the size of the 1D texture vector
    glClear(GL_COLOR_BUFFER_BIT); // Dont need to clear it as its writing to each pixel anyway
}

// void Buffer::bind() {
//     this->bind(Bwhich::other);
// }

void Buffer::update() {
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Need to only write 4 points since vertex shader makes them into a quad over the entire screen anyway
}

void Buffer::update(const int P) {
    glDrawArrays(GL_POINTS, 0, P);
}

void Buffer::set_index(const Bwhich which, const PBindex index) {
    switch (which) {
        case Bwhich::current: { this->current = index; } break;
        case Bwhich::other: { this->other = index; } break;
        case Bwhich::screen: {
            fprintf(stderr, "[%s] can't set object index to screen buffer", this->name);
            exit(EXIT_FAILURE);
        } break;
    }
}

void Buffer::check_shape(const int width, const int height) {
    GLint max_renderbuffer_size;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_renderbuffer_size);
    if (width > max_renderbuffer_size) {
        fprintf(stderr, "[%s] Physics framebuffer width (%d) larger than renderbuffer size (%d)", this->name, width, max_renderbuffer_size);
        exit(EXIT_FAILURE);
    }
    if (height > max_renderbuffer_size) {
        fprintf(stderr, "[%s] Physics framebuffer width (%d) larger than renderbuffer size (%d)", this->name, height, max_renderbuffer_size);
        exit(EXIT_FAILURE);
    }
}