# 设置此项目的自定义库的头文件目录
set(MyInclude "${PROJECT_SOURCE_DIR}/include")

# 这是一个自定义库
add_library(LogMe "logme.c")

add_library(VList "vlist.c")

# 使用自定义库需要添加头文件目录，PUBLIC 表明库本身和库的使用者都需要到此头文件目录寻找头文件
target_include_directories(LogMe PUBLIC ${MyInclude})

target_include_directories(VList PUBLIC ${MyInclude})