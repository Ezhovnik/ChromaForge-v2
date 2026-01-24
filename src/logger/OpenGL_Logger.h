#ifndef LOGGER_OPENGL_LOGGER_H_
#define LOGGER_OPENGL_LOGGER_H_

#include "Logger.h"
#include <GL/glew.h>

class OpenGL_Logger {
private:
    bool isInitialized_;
    LogLevel logLevel_;

    OpenGL_Logger() : isInitialized_(false), logLevel_(LogLevel::DEBUG) {}
    ~OpenGL_Logger() = default;

    static const char* getGLErrorString(GLenum error);
    static const char* getGLDebugSourceString(GLenum source);
    static const char* getGLDebugTypeString(GLenum type);
    static const char* getGLDebugSeverityString(GLenum severity);
    
    bool shouldLog(GLenum severity) const;
    void setDebugMessageFilters();

    void logGLError(GLenum error, const char* context, const char* fileName, int lineNumber);
public:
    OpenGL_Logger(const OpenGL_Logger&) = delete;
    OpenGL_Logger& operator=(const OpenGL_Logger&) = delete;
    
    static OpenGL_Logger& getInstance() {
        static OpenGL_Logger instance;
        return instance;
    }
    
    void initialize(LogLevel level = LogLevel::DEBUG);
    void enableDebugOutput();
    void disableDebugOutput();

    void finalize();
    
    bool checkGLError(const char* context, const char* fileName, int lineNumber);
    
    static void GLAPIENTRY debugMessageCallback(GLenum source, GLenum type, GLuint id, 
                                                GLenum severity, GLsizei length, 
                                                const GLchar* message, const void* userParam);
};

#define GL_CHECK() OpenGL_Logger::getInstance().checkGLError(__func__, __FILE__, __LINE__)

#endif // LOGGER_OPENGL_LOGGER_H_
