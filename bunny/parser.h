#ifndef PARSER_H
#define PARSER_H
#include <iostream>
#include <cstdio>
// GLFW
#include <GLFW/glfw3.h>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void parseIndex(const char* fileName, GLfloat* vertices, GLuint* indices, GLuint& vertices_size, GLuint& indices_size);
void parseNormal(const char* fileName, GLfloat* vertices, GLuint* indices, GLuint& vertices_size, GLuint& indices_size);
#endif
