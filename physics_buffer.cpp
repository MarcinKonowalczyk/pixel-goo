#include "physics_buffer.h"
#include <iostream>
#include <glad.h>

void PhysicsBuffer::allocate(const PBwhich which, const PBindex index, const int width, const int height, const char* data) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, textures[index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->minmag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->minmag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrap_st);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrap_st);

    switch (this->dim) {
        case PB_1D: {
            glTexImage2D(GL_TEXTURE_2D, 0, PB_1D, width, height, 0, GL_RED, GL_FLOAT, data);
        } break;
        case PB_2D: {
            glTexImage2D(GL_TEXTURE_2D, 0, PB_2D, width, height, 0, GL_RG, GL_FLOAT, data);
        } break;
        case PB_3D: {
            glTexImage2D(GL_TEXTURE_2D, 0, PB_3D, width, height, 0, GL_RGB, GL_FLOAT, data);
        } break;
        case PB_4D: {
            glTexImage2D(GL_TEXTURE_2D, 0, PB_4D, width, height, 0, GL_RGBA, GL_FLOAT, data);
        } break;
        default: {
            fprintf(stderr, "[%s] 'dim' not set before call to 'allocate'", this->name);
            exit(EXIT_FAILURE);
        }
    }
    // glTexImage2D(GL_TEXTURE_2D, 0, this->dim, width, height, 0, GL_RG, GL_FLOAT, data);

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

void PhysicsBuffer::reallocate(const PBwhich which, const int width, const int height) {
    this->allocate(which, this->current, width, height, NULL);
}

void PhysicsBuffer::flip_buffers() {
    PBindex temp = this->current;
    this->current = this->other;
    this->other = temp;
}

void PhysicsBuffer::bind(const PBwhich which) {
    PBindex index;
    switch (which) {
        case PBwhich::current: { index = this->current; } break;
        case PBwhich::other: { index = this->other; } break;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[index]);
    glViewport(0, 0, width, height); // Change the viewport to the size of the 1D texture vector
    glClear(GL_COLOR_BUFFER_BIT); // Dont need to clear it as its writing to each pixel anyway
}

// void PhysicsBuffer::bind() {
//     this->bind(PBwhich::other);
// }

void PhysicsBuffer::update() {
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Need to only write 4 points since vertex shader makes them into a quad over the entire screen anyway
}

void PhysicsBuffer::update(const int P) {
    glDrawArrays(GL_POINTS, 0, P);
}

void PhysicsBuffer::set_index(const PBwhich which, const PBindex index) {
    switch (which) {
        case PBwhich::current: { this->current = index; } break;
        case PBwhich::other: { this->other = index; } break;
    }
}