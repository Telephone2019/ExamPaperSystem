cmake_minimum_required (VERSION 3.8)

# 设置此项目的自定义库的头文件目录
set(MyInclude1 "${PROJECT_SOURCE_DIR}/include")

################################################ 自定义库 ################################################

# 这是一个自定义库
add_library(LogMe "logme.c")

add_library(VList "vlist.c")

add_library(VUtils "vutils.c")

add_library(HttpParser "httpparser.c")

add_library(HttpUtils "httputils.c")

add_library(llhttp "llhttp.c" "llhttp_api.c" "llhttp_http.c")

# 仅适用于 windows 平台
add_library(TCPServer "tcpserver.c")

# 仅适用于 linux 平台
add_library(TCPServerLinux "tcpserverlinux.c")

############################################## 自定义导入库 ##############################################

# 对于自定义导入库来说，GLOBAL 是必须的，用于确保上级CMakeLists.txt能使用这个库。
# 如果导入的是动态库，使用 SHARED 关键字；如果导入的是静态库，使用 STATIC 关键字。
# 需要使用 IMPORTED_LOCATION 关键字指定导入的库的位置（不管是动态库还是静态库）。
# 如果导入的是 DLL 动态库，需要额外使用 IMPORTED_IMPLIB 关键字指定 DLL库的配套导入库 的位置。
# 如果使用了自定义导入库并且导入的是动态库，程序运行时必须在恰当的地方提供与.def同名（如果你没有.def，那么就与库同名）的动态库文件

# 仅适用于 windows 平台
add_library(SQLite3_win_x64 SHARED IMPORTED GLOBAL)
set_target_properties(SQLite3_win_x64 PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/SQLite3_win_x64.dll"
    IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/SQLite3_win_x64.lib"
)

########################################### 自定义库的头文件目录 ###########################################

# 使用自定义库需要添加头文件目录，PUBLIC 表明库本身和库的使用者都需要到此头文件目录寻找头文件
target_include_directories(LogMe PUBLIC ${MyInclude1})

target_include_directories(VList PUBLIC ${MyInclude1})

target_include_directories(VUtils PUBLIC ${MyInclude1})

target_include_directories(HttpParser PUBLIC ${MyInclude1})

target_include_directories(HttpUtils PUBLIC ${MyInclude1})

target_include_directories(llhttp PUBLIC ${MyInclude1})

# 仅适用于 windows 平台
target_include_directories(TCPServer PUBLIC ${MyInclude1})

target_include_directories(SQLite3_win_x64 INTERFACE ${MyInclude1})

# 仅适用于 linux 平台
target_include_directories(TCPServerLinux PUBLIC ${MyInclude1})

######################################### 自定义库需要链接的其他库 #########################################

# 一些自定义库需要链接别的库，PRIVATE 表明链接的库仅用于自定义库本身
target_link_libraries(HttpParser PRIVATE VUtils llhttp)
target_link_libraries(HttpParser PUBLIC VList)
# 只有 windows 平台才有的链接库
target_link_libraries(HttpParser PRIVATE Shlwapi)

# 仅适用于 windows 平台
target_link_libraries(TCPServer PRIVATE LogMe Ws2_32 VUtils Mswsock HttpUtils)
target_link_libraries(TCPServer PUBLIC HttpParser VList)

# 仅适用于 linux 平台
target_link_libraries(TCPServerLinux PUBLIC VList)

############################################# 自定义库的安装 #############################################

# 此处不可安装导入库

# 仅适用于 windows 平台
# install(TARGETS SQLite3_win DESTINATION ${PROJECT_BINARY_DIR})

########################################### 自定义导入库的安装 ###########################################

# 仅适用于 windows 平台
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/SQLite3_win_x64.dll" DESTINATION ${PROJECT_BINARY_DIR})

##########################################################################################################