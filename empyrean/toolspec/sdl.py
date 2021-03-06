from ToolCommon import *

def generate(env):
    if env['PLATFORM'] == 'cygwin':
        env.Append(CPPPATH = '/usr/local/include/SDL',
                   CPPDEFINES = ['main=SDL_main', 'WIN32'],
                   CCFLAGS = ['-mwindows'],
                   LIBS = ['SDL', 'SDLmain'])
    elif env['PLATFORM'] == 'win32':
        env.Append(LIBS=['SDL', 'SDLmain'])
    else:
        ParseConfig(env, 'sdl-config', ['--cflags', '--libs'])

def exists(env):
    return 1
