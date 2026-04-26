# CMake generated Testfile for 
# Source directory: D:/VS Code/program/C++/softwarelab3/tests
# Build directory: D:/VS Code/program/C++/softwarelab3/build/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(BlackBoxTest "D:/VS Code/program/C++/softwarelab3/build/tests/test_black_box.exe")
set_tests_properties(BlackBoxTest PROPERTIES  _BACKTRACE_TRIPLES "D:/VS Code/program/C++/softwarelab3/tests/CMakeLists.txt;19;add_test;D:/VS Code/program/C++/softwarelab3/tests/CMakeLists.txt;0;")
add_test(WhiteBoxTest "D:/VS Code/program/C++/softwarelab3/build/tests/test_white_box.exe")
set_tests_properties(WhiteBoxTest PROPERTIES  _BACKTRACE_TRIPLES "D:/VS Code/program/C++/softwarelab3/tests/CMakeLists.txt;20;add_test;D:/VS Code/program/C++/softwarelab3/tests/CMakeLists.txt;0;")
subdirs("googletest-build")
