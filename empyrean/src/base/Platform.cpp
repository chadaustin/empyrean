#include "Log.h"
#include "Platform.h"


// Cygwin cannot use the Win32 code path because Cygwin maintains its
// own internal current directory independent of the Win32 one.  (But it
// updates the Win32 one when the Cygwin one changes.)
#if defined(WIN32) && !defined(__CYGWIN__)

#include <windows.h>

bool pyr::setCurrentDirectory(const std::string& path) {
    return SetCurrentDirectory(path.c_str()) != FALSE;
}

std::string pyr::getCurrentDirectory() {
    char buffer[MAX_PATH + 1];
    if (GetCurrentDirectory(MAX_PATH, buffer)) {
        return buffer;
    } else {
        return "";
    }
}

#else

#include <unistd.h>

bool pyr::setCurrentDirectory(const std::string& dir) {
    return chdir(dir.c_str()) == 0;
}

std::string pyr::getCurrentDirectory() {
    char buffer[2048];
    char* rv = getcwd(buffer, sizeof(buffer));
    return (rv ? rv : "");
}

#endif


// Now that we've got setCurrentDirectory() and getCurrentDirectory(), we
// can implement {get,set}StartDirectory.


#ifdef WIN32


#include <windows.h>

std::string pyr::getStartDirectory(const char* /*argv0*/) {
    char filename[MAX_PATH];
    GetModuleFileName(GetModuleHandle(0), filename, MAX_PATH);
    if (char* backslash = strrchr(filename, '\\')) {
        *backslash = 0;
        return filename;
    } else {
        return "";
    }
}


#else  // If it's not Windows, it's probably UNIX


#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Another way to implement this:
// http://www.flipcode.org/cgi-bin/fcarticles.cgi?show=4&id=64160

std::string pyr::getStartDirectory(const char* argv0) {
    if (!argv0) {
        return "";
    }

    std::string rv;

    char* path = strdup(argv0);
    if (char* slash = strrchr(path, '/')) {
        *slash = 0;
        rv = path;
    }

    free(path);
    return rv;
}


#endif // #ifdef WIN32


namespace pyr {

    namespace {
        Logger& _logger = Logger::get("pyr.Platform");
    }

    void setStartDirectory(const char* argv0) {
        std::string startDir = getStartDirectory(argv0);
        PYR_LOG(_logger, INFO, "Setting start directory to: " << startDir);
        if (!startDir.empty()) {
            if (!setCurrentDirectory(startDir.c_str())) {
                PYR_LOG(_logger, WARN, "SetCurrentDirectory(" << startDir << ") failed");
            }
        }
    }

}
