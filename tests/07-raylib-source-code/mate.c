#define MATE_IMPLEMENTATION
#include "../../mate.h"

static char *GetCFlags(void) {
  FlagBuilder flagsBuilder = FlagBuilderCreate();

  // NOTE: Default Raylib Flags
  FlagBuilderAdd(&flagsBuilder,
                 "std=c99",
                 "D_GNU_SOURCE",
                 "DGL_SILENCE_DEPRECATION=199309L",
                 "fno-sanitize=undefined" // https://github.com/raysan5/raylib/issues/3674
  );

  if (isLinux()) {
    FlagBuilderAdd(&flagsBuilder, "DPLATFORM_DESKTOP_GLFW", "D_GLFW_X11");
  }

  if (isFreeBSD()) {
    FlagBuilderAdd(&flagsBuilder, "DPLATFORM_DESKTOP_GLFW", "D_GLFW_X11");
    // Required due to raylib's use of X11 on FreeBSD
    FlagBuilderAdd(&flagsBuilder, "isystem /usr/local/include");
  }

  if (isMacOs()) {
    FlagBuilderAdd(&flagsBuilder, "DPLATFORM_DESKTOP_GLFW", "D_GL_SILENCE_DEPRECATION=1");
    // Required due to raylib's use of Objective-C types on macOS.
    FlagBuilderAdd(&flagsBuilder, "x objective-c");
  }
  if (isWindows()) {
    FlagBuilderAdd(&flagsBuilder, "DPLATFORM_DESKTOP_GLFW");
  }

  return flagsBuilder.buffer.data;
}

i32 main(void) {
  StartBuild();
  {
    { // Compile static lib
      StaticLib staticLib = CreateStaticLib((StaticLibOptions){
          .output = "libraylib",
          .flags = GetCFlags(),
      });

      AddFile(staticLib, "./src/rcore.c");
      AddFile(staticLib, "./src/utils.c");
      AddFile(staticLib, "./src/rglfw.c");

      AddFile(staticLib, "./src/rshapes.c");
      AddFile(staticLib, "./src/rtextures.c");
      AddFile(staticLib, "./src/rtext.c");
      AddFile(staticLib, "./src/rmodels.c");
      AddFile(staticLib, "./src/raudio.c");

      AddIncludePaths(staticLib, "./src/platforms");
      AddIncludePaths(staticLib, "./src/external/glfw/include");

      if (isLinux()) {
        LinkSystemLibraries(staticLib, "GL", "rt", "dl", "m", "X11", "Xcursor", "Xext", "Xfixes", "Xi", "Xinerama", "Xrandr", "Xrender");
      }
      if (isFreeBSD()) {
        // Link regular system libraries.
        AddLibraryPaths(staticLib, "/usr/local/lib");
        LinkSystemLibraries(staticLib, "GL", "rt", "dl", "m", "X11", "Xcursor", "Xext", "Xfixes", "Xi", "Xinerama", "Xrandr", "Xrender");
      }
      if (isMacOs()) {
        // Link regular system libraries.
        LinkSystemLibraries(staticLib, "m");
        // Link macOS system frameworks.
        LinkFrameworks(staticLib, "Foundation", "AppKit", "IOKit", "OpenGL", "CoreVideo");
      }
      if (isWindows()) {
        LinkSystemLibraries(staticLib, "winmm", "gdi32", "opengl32");
      }

      InstallStaticLib(staticLib);
    }

    { // Run simple example
      Executable executable = CreateExecutable((ExecutableOptions){
        .output = "basic-example",
        .std = FLAG_STD_C99,
        .warnings = FLAG_WARNINGS_NONE,
      });

      AddFile(executable, "./src/basic-example.c");

      AddIncludePaths(executable, "./src");
      AddLibraryPaths(executable, "./build"); // TODO: LinkStaticLib()

      if (isLinux()) {
        LinkSystemLibraries(executable, "raylib", "GL", "rt", "dl", "m", "X11");
      }
      if (isFreeBSD()) {
        // Link regular system libraries.
        AddLibraryPaths(executable, "/usr/local/lib");
        LinkSystemLibraries(executable, "raylib", "GL", "rt", "dl", "m", "X11");
      }
      if (isMacOs()) {
        // Link regular system libraries.
        LinkSystemLibraries(executable, "raylib", "m");
        // Link macOS system frameworks.
        LinkFrameworks(executable, "CoreVideo", "IOKit", "Cocoa", "GLUT", "OpenGL");
      }
      if (isWindows()) {
        LinkSystemLibraries(executable, "raylib", "winmm", "gdi32", "opengl32");
      }

      InstallExecutable(executable);
      RunCommand(executable.outputPath);
    }
  }
  EndBuild();
}
