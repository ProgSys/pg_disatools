/*
 *  GNU Lesser General Public License (LGPL):
 *
 *	Copyright (C) 2016  ProgSys
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation, version 3 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public License
 *	along with this program.  If not, see http://doc.qt.io/qt-5/lgpl.html
 *	or http://www.gnu.org/licenses/
 */
#include <openGL/PG_GLError.h>

/*
#if defined(WIN32) || defined(__MINGW32__)
	#include <GL/glew.h>
#elif __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL3/gl3.h>
#endif
*/

#if __APPLE__
	#include <OpenGL/gl3.h>
#else
	#include <GL/glew.h>
#endif

bool check_gl_error(const char *file, int line) {
        GLenum err (glGetError());
        while(err!=GL_NO_ERROR) {
                std::string error;

                switch(err) {
					//The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.
					case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
					// An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.
					case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
					// A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.
					case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
					// There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.
					case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
					// The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.
					case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
					//An attempt has been made to perform an operation that would cause an internal stack to underflow.
					case GL_STACK_UNDERFLOW:  error="STACK_UNDERFLOW";  break;
					// An attempt has been made to perform an operation that would cause an internal stack to overflow.
					case GL_STACK_OVERFLOW:  error="STACK_OVERFLOW";  break;
					default: error="UNKNOWN"; break;
                }

                std::cerr<<"["<<file<<":"<<line<<"]: "<<"GL_"<<error<<" error!"<<std::endl;
                err=glGetError();
        }
        return err;

}
