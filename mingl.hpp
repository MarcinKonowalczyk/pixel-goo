#ifndef MINGL_H
#define MINGL_H

#include <cstdint>

struct GLFWwindow;

struct MinGLColor {
	float rgba[4];
};

class MinGL
{
public:
	bool init(unsigned width, unsigned height, const char* title);
	bool windowShouldClose() const;
	void pollEvents() const;
	void processInput() const;
	void putPixel(int x, int y) const;
	void flush(float r, float g, float b, float a);
	void shutdown() const;
	GLFWwindow* getWindow();

private:
	GLFWwindow* window;
	int m_displayW;
	int m_displayH;
	unsigned m_shaderProgram;
};

#endif /* MINGL_H */