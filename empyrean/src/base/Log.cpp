//#define PYR_LOG_TO_STDOUT
//#define PYR_LOG_TO_WIN32_DEBUGGER

#if defined(WIN32)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#include <iostream>
#include "Log.h"


// Perhaps this should go into Constants.h
static const char* DEFAULT_LOG_FILE = "default.log";


namespace pyr {

    PYR_DEFINE_SINGLETON(Log)
    
    Log::Log() {
        _stream = 0;
    }

    Log::~Log() {
        close();
    }

    void Log::open(const std::string& filename) {
#if defined(WIN32) && defined(PYR_LOG_TO_STDOUT)
        AllocConsole();

#ifndef __CYGWIN__
        // See http://www.flipcode.com/cgi-bin/msg.cgi?showThread=00003996&forum=general&id=-1
        FILE* out = _fdopen(_open_osfhandle((intptr_t)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT), "w");
        FILE* err = _fdopen(_open_osfhandle((intptr_t)GetStdHandle(STD_ERROR_HANDLE),  _O_TEXT), "w");
        if (out) *stdout = *out;
        if (err) *stderr = *err;
#endif
#endif

        if (_file.is_open()) {
            // Perhaps this should just close the current file and open
            // a new one.  Until then...
            throw LogFileOpenFailure("Log file already open");
        }

        // Open the file
        _file.open(filename.c_str());
        if (!_file.is_open()) {
            throw LogFileOpenFailure("Error opening file: " + filename);
        }

        _stream = &_file;
    }

    void Log::write(const std::string& message) {
        // If a file is not open, open the default one.
        if (!_file.is_open()) {
            try {
                open(DEFAULT_LOG_FILE);
            }
            catch (const LogFileOpenFailure&) {
                // use cerr as a last resort
                _stream = &std::cerr;
            }
        }

        doWrite(std::string(_indent * 2, ' ') + message);
    }

    void Log::beginBlock() {
        ++_indent;
    }

    void Log::endBlock() {
        --_indent;
        PYR_ASSERT(_indent >= 0, "endBlock() called too many times");
    }

    void Log::doWrite(const std::string& message) {
        PYR_ASSERT(_stream, "_stream pointer was never set");
        (*_stream) << message << std::endl;

#ifdef PYR_LOG_TO_STDOUT
        std::cout << message << std::endl;
#endif

#if defined(WIN32) && defined(PYR_LOG_TO_WIN32_DEBUGGER)
        OutputDebugString(message.c_str());
        OutputDebugString("\n");
#endif
    }

    void Log::close() {
#if defined(WIN32) && defined(PYR_LOG_TO_STDOUT)
        FreeConsole();
#endif

        // Close the file if needs be
        if (_file.is_open()) {
            _file.close();
        }
        _stream = 0;
    }

}
