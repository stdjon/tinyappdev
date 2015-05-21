Very small libraries for C/C++ application development and testing. Currently consists of the following:

  * **PML** - the Proxy Memory Layer, a memory allocation framework for C and C++.
  * **miniconf** - a lightweight commandline and ini-file parser.
  * **testframe** - a very small unit test framework.

Each library consists of one or two header files, and one or two implementation files, which can be copied into an existing project or built as standalone libs. An extensible make-based build system is provided - it is easy to add new library and binary targets to the existing source tree.