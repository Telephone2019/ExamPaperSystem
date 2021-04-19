cmake_minimum_required (VERSION 3.8)

# 指定C/C++编译器
# set(CMAKE_C_COMPILER "/bin/gcc")
# set(CMAKE_CXX_COMPILER "/bin/gcc")

# 指定C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)

# 指定C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# 详细的make输出
set(CMAKE_VERBOSE_MAKEFILE ON)

# 添加C编译选项
# -g 是用来指示 gcc 编译器生成可调试的可执行文件
# SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# 添加C++编译选项
# -g 是用来指示 gcc 编译器生成可调试的可执行文件
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

project ("ExamPaperSystem")

# 将源代码添加到此项目的可执行文件。
add_executable (ExamPaperSystem "ExamPaperSystem.c")

# 链接额外的静态库和动态库
# 依赖关系（如果有）要满足：前面的库依赖后面的库
# -Wl,-Bdynamic 和 -Wl,-Bstatic 是 gcc 编译器用于切换动态/静态链接的编译选项
# 如果中间切换成了静态链接，末尾要恢复动态链接
# PRIVATE 的意思是这些库链接仅仅用于编译此目标

# 自己编写的库
# 添加自定义库的 CMAKE 文件所在的文件夹
add_subdirectory(src)
# 链接自定义库
target_link_libraries(ExamPaperSystem PRIVATE VList LogMe)

# 标准库
# GCC
# target_link_libraries(ExamPaperSystem PRIVATE -Wl,-Bdynamic pthread stdc++ -Wl,-Bstatic m -Wl,-Bdynamic)
# MSVC
# target_link_libraries(ExamPaperSystem PRIVATE )

# GCC 强制此目标的所有库链接使用静态链接
# target_link_libraries(ExamPaperSystem PRIVATE -static)
