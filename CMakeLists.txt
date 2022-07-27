cmake_minimum_required(VERSION 3.20)
project(BirlutiuProject)

set(CMAKE_CXX_STANDARD 14)

add_executable(BirlutiuProject main.cpp Shader.cpp Shader.hpp Camera.cpp Camera.hpp Mesh.cpp Mesh.hpp Model3D.cpp Model3D.hpp
        stb_image.cpp stb_image.h tiny_obj_loader.cpp tiny_obj_loader.h Window.cpp Window.h SkyBox.cpp SkyBox.hpp Animation.h Structures.h)
target_link_libraries(BirlutiuProject glfw GLEW GL)