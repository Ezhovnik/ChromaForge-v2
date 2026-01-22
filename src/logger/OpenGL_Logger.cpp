#include "OpenGL_Logger.h"
#include <iostream>

void OpenGL_Logger::initialize(LogLevel level) {
    if (isInitialized_) {
        if (logLevel_ <= LogLevel::WARN) LOG_WARN("OpenGL Logger is already initialized");
        return;
    }
    
    logLevel_ = level;
    isInitialized_ = true;
    
    // Check if GLEW is initialized
    if (glewInit() != GLEW_OK) {
        LOG_ERROR("Failed to initialize GLEW");
        return;
    }
    
    if (logLevel_ <= LogLevel::INFO) LOG_INFO("OpenGL Logger initialized with level: {}", static_cast<int>(logLevel_));
    
    if (logLevel_ <= LogLevel::DEBUG) {
        LOG_DEBUG("OpenGL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
        LOG_DEBUG("OpenGL Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        LOG_DEBUG("OpenGL Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        LOG_DEBUG("GLSL Version: {}", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
    }
    
    if (GLEW_ARB_debug_output || GLEW_KHR_debug) {
        if (logLevel_ <= LogLevel::DEBUG) LOG_DEBUG("OpenGL debug output is supported");
        enableDebugOutput();
    } else {
        if (logLevel_ <= LogLevel::WARN) LOG_WARN("OpenGL debug output is not supported");
    }
}

void OpenGL_Logger::finalize() {
    if (!isInitialized_) {
        if (logLevel_ <= LogLevel::WARN) LOG_WARN("OpenGL Logger is already finalized");
        return;
    }
    
    glDebugMessageCallback(nullptr, nullptr);
    
    glDisable(GL_DEBUG_OUTPUT);
    glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    
    isInitialized_ = false;
    
    if (logLevel_ <= LogLevel::INFO) LOG_INFO("OpenGL Logger disabled");
}

bool OpenGL_Logger::shouldLog(GLenum severity) const {
    // Map OpenGL severity to our LogLevel
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            return logLevel_ <= LogLevel::ERR;
        case GL_DEBUG_SEVERITY_MEDIUM:
            return logLevel_ <= LogLevel::WARN;
        case GL_DEBUG_SEVERITY_LOW:
            return logLevel_ <= LogLevel::DEBUG;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return logLevel_ <= LogLevel::TRACE;
        default:
            return logLevel_ <= LogLevel::INFO;
    }
}

void OpenGL_Logger::setDebugMessageFilters() {
    if (!(GLEW_ARB_debug_output || GLEW_KHR_debug)) {
        return;
    }
    
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
    
    switch (logLevel_) {
        case LogLevel::TRACE:
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_TRUE);
        case LogLevel::DEBUG:
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);
        case LogLevel::INFO:
        case LogLevel::WARN:
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
        case LogLevel::ERR:
        case LogLevel::CRITICAL:
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
            break;
        case LogLevel::OFF:
            break;
    }
    
    if (logLevel_ <= LogLevel::DEBUG) LOG_DEBUG("OpenGL debug filters set for log level: {}", static_cast<int>(logLevel_));
}

void OpenGL_Logger::enableDebugOutput() {
    if (!isInitialized_) {
        if (logLevel_ <= LogLevel::WARN) LOG_WARN("OpenGL Logger not initialized. Call initialize() first.");
        return;
    }
    
    if (GLEW_ARB_debug_output || GLEW_KHR_debug) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        
        glDebugMessageCallback(debugMessageCallback, this);
        
        setDebugMessageFilters();
        
        if (logLevel_ <= LogLevel::INFO) LOG_INFO("OpenGL debug output enabled with level: {}", static_cast<int>(logLevel_));
    }
}

void OpenGL_Logger::disableDebugOutput() {
    if (GLEW_ARB_debug_output || GLEW_KHR_debug) {
        glDisable(GL_DEBUG_OUTPUT);
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        
        if (logLevel_ <= LogLevel::INFO) LOG_INFO("OpenGL debug output disabled");
    }
}

bool OpenGL_Logger::checkGLError(const char* context, const char* fileName, int lineNumber) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR && logLevel_ <= LogLevel::ERR) {
        logGLError(error, context, fileName, lineNumber);
        return false;
    }
    return true;
}

void OpenGL_Logger::logGLError(GLenum error, const char* context, const char* fileName, int lineNumber) {
    const char* errorStr = getGLErrorString(error);
    LOG_ERROR("OpenGL error in {} ({}:{}): {} (0x{:X})", context, fileName, lineNumber, errorStr, error);
}

void OpenGL_Logger::setLevel(LogLevel level) {
    logLevel_ = level;
    
    if (isInitialized_ && (GLEW_ARB_debug_output || GLEW_KHR_debug)) {
        setDebugMessageFilters();
        if (logLevel_ <= LogLevel::DEBUG) LOG_DEBUG("OpenGL Logger level changed to: {}", static_cast<int>(level));
    }
}

const char* OpenGL_Logger::getGLErrorString(GLenum error) {
    switch (error) {
        case GL_INVALID_ENUM:                  return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:                 return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:             return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW:                return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:               return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY:                 return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_CONTEXT_LOST:                  return "GL_CONTEXT_LOST";
        default:                               return "Unknown OpenGL error";
    }
}

const char* OpenGL_Logger::getGLDebugSourceString(GLenum source) {
    switch (source) {
        case GL_DEBUG_SOURCE_API:               return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     return "Window System";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:   return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:       return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION:       return "Application";
        case GL_DEBUG_SOURCE_OTHER:             return "Other";
        default:                                return "Unknown";
    }
}

const char* OpenGL_Logger::getGLDebugTypeString(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:               return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behavior";
        case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
        case GL_DEBUG_TYPE_MARKER:              return "Marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:          return "Push Group";
        case GL_DEBUG_TYPE_POP_GROUP:           return "Pop Group";
        case GL_DEBUG_TYPE_OTHER:               return "Other";
        default:                                return "Unknown";
    }
}

const char* OpenGL_Logger::getGLDebugSeverityString(GLenum severity) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:            return "High";
        case GL_DEBUG_SEVERITY_MEDIUM:          return "Medium";
        case GL_DEBUG_SEVERITY_LOW:             return "Low";
        case GL_DEBUG_SEVERITY_NOTIFICATION:    return "Notification";
        default:                                return "Unknown";
    }
}

void GLAPIENTRY OpenGL_Logger::debugMessageCallback(GLenum source, GLenum type, GLuint id, 
                                                    GLenum severity, GLsizei length, 
                                                    const GLchar* message, const void* userParam) {
    (void)length;
    
    const OpenGL_Logger* logger = static_cast<const OpenGL_Logger*>(userParam);
    
    if (logger && !logger->shouldLog(severity)) return;
    
    const char* sourceStr = getGLDebugSourceString(source);
    const char* typeStr = getGLDebugTypeString(type);
    const char* severityStr = getGLDebugSeverityString(severity);
    
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            LOG_ERROR("OpenGL Debug [ID: {}] [Source: {}] [Type: {}] [Severity: {}]: {}", 
                      id, sourceStr, typeStr, severityStr, message);
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            LOG_WARN("OpenGL Debug [ID: {}] [Source: {}] [Type: {}] [Severity: {}]: {}", 
                     id, sourceStr, typeStr, severityStr, message);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            LOG_DEBUG("OpenGL Debug [ID: {}] [Source: {}] [Type: {}] [Severity: {}]: {}", 
                      id, sourceStr, typeStr, severityStr, message);
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            LOG_TRACE("OpenGL Debug [ID: {}] [Source: {}] [Type: {}] [Severity: {}]: {}", 
                      id, sourceStr, typeStr, severityStr, message);
            break;
        default:
            LOG_INFO("OpenGL Debug [ID: {}] [Source: {}] [Type: {}] [Severity: {}]: {}", 
                     id, sourceStr, typeStr, severityStr, message);
            break;
    }
}
