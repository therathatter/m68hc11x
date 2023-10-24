# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/root/CLionProjects/m68hc11/cmake-build-debug/_deps/hello_imgui-src"
  "C:/Users/root/CLionProjects/m68hc11/cmake-build-debug/_deps/hello_imgui-build"
  "C:/Users/root/CLionProjects/m68hc11/cmake-build-debug/_deps/hello_imgui-subbuild/hello_imgui-populate-prefix"
  "C:/Users/root/CLionProjects/m68hc11/cmake-build-debug/_deps/hello_imgui-subbuild/hello_imgui-populate-prefix/tmp"
  "C:/Users/root/CLionProjects/m68hc11/cmake-build-debug/_deps/hello_imgui-subbuild/hello_imgui-populate-prefix/src/hello_imgui-populate-stamp"
  "C:/Users/root/CLionProjects/m68hc11/cmake-build-debug/_deps/hello_imgui-subbuild/hello_imgui-populate-prefix/src"
  "C:/Users/root/CLionProjects/m68hc11/cmake-build-debug/_deps/hello_imgui-subbuild/hello_imgui-populate-prefix/src/hello_imgui-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/root/CLionProjects/m68hc11/cmake-build-debug/_deps/hello_imgui-subbuild/hello_imgui-populate-prefix/src/hello_imgui-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/root/CLionProjects/m68hc11/cmake-build-debug/_deps/hello_imgui-subbuild/hello_imgui-populate-prefix/src/hello_imgui-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
