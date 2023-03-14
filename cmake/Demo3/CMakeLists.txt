# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (Demo3)
# 将源文件赋值到变量SOURCE_FILE中,可多个
set(SOURCE_FILE main.cc)

# 添加 math 子目录
add_subdirectory(math)

# 设置项目编译头文件搜索路径 -I
# 变量PROJECT_SOURCE_DIR是当前cmake的路径
include_directories(${PROJECT_SOURCE_DIR}/math)
# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/math)

# 指定生成目标
add_executable(Demo ${SOURCE_FILE})
# 设置要链接的静态库 -L
target_link_libraries(Demo MathFunctions)

