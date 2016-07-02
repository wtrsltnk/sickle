#ifndef _SHADER_H_
#define _SHADER_H_

#include <string>
#include <vector>
#include <GL/glextl.h>

GLuint shader_LoadProgram(const std::string & vertShaderStr,
			 const std::string & fragShaderStr);

#ifdef SHADER_H_IMPL

GLuint shader_LoadProgram(const std::string & vertShaderStr,
			 const std::string & fragShaderStr)
{
    if (glCreateShader == 0)
	{
	    Log("glCreateShader not loaded");
	    return 0;
	}
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *vertShaderSrc = vertShaderStr.c_str();
    const char *fragShaderSrc = fragShaderStr.c_str();

    GLint result = GL_FALSE;
    int logLength;

    // Compile vertex shader
    glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
    glCompileShader(vertShader);

    // Check vertex shader
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
	{
	    glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);
	    std::vector <
		char >vertShaderError((logLength > 1) ? logLength : 1);
	    glGetShaderInfoLog(vertShader, logLength, NULL,
			       &vertShaderError[0]);
	    Log("error compiling vertex shader: %s", &vertShaderError[0]);
	    return 0;
	}
    // Compile fragment shader
    glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
    glCompileShader(fragShader);

    // Check fragment shader
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
	{
	    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);
	    std::vector <
		char >fragShaderError((logLength > 1) ? logLength : 1);
	    glGetShaderInfoLog(fragShader, logLength, NULL,
			       &fragShaderError[0]);
	    Log("error compiling fragment shader: %s",
		&fragShaderError[0]);
		return 0;
	}

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
	{
	    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	    std::vector <
		char >programError((logLength > 1) ? logLength : 1);
	    glGetProgramInfoLog(program, logLength, NULL,
				&programError[0]);
	    Log("error linking program: %s", &programError[0]);
	    return 0;
	}

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    //Log("program successfully linked");

    return program;
}

#endif				// SHADER_H_IMPL

#endif				// _SHADER_H_
