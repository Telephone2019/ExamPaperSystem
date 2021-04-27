cmake_minimum_required (VERSION 3.8)

# 设置此项目的自定义库的头文件目录
set(MyInclude1 "${PROJECT_SOURCE_DIR}/include")

################################################ 自定义库 ################################################

# 这是一个自定义库
add_library(LogMe "logme.c")

add_library(VList "vlist.c")

add_library(VUtils "vutils.c")

add_library(HttpParser "httpparser.c")

# 仅适用于 windows 平台
add_library(TCPServer "tcpserver.c")

########################################### 自定义库的头文件目录 ###########################################

# 使用自定义库需要添加头文件目录，PUBLIC 表明库本身和库的使用者都需要到此头文件目录寻找头文件
target_include_directories(LogMe PUBLIC ${MyInclude1})

target_include_directories(VList PUBLIC ${MyInclude1})

target_include_directories(VUtils PUBLIC ${MyInclude1})

target_include_directories(HttpParser PUBLIC ${MyInclude1})

# 仅适用于 windows 平台
target_include_directories(TCPServer PUBLIC ${MyInclude1})

######################################### 自定义库需要链接的其他库 #########################################

# 一些自定义库需要链接别的库，PRIVATE 表明链接的库仅用于自定义库本身

# 仅适用于 windows 平台
target_link_libraries(TCPServer PRIVATE LogMe Ws2_32 VUtils VList)

##########################################################################################################