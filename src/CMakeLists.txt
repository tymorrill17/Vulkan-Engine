
# Add source to this project's executable.
add_executable(run_engine
    main.cpp
    vk_engine.cpp
    vk_engine.h
    vk_types.h
    vk_initializers.cpp
    vk_initializers.h
    vk_pipeline.cpp
    vk_pipeline.h
    vk_mesh.h
    vk_mesh.cpp
    vk_texture.h
    vk_texture.cpp
    vk_descriptors.h
    vk_descriptors.cpp)

# Sets the Visual Studio debugger directory
set_property(TARGET run_engine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:run_engine>")

target_include_directories(run_engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
# Links all of the 3rd party libraries
target_link_libraries(run_engine vkbootstrap vma glm tinyobjloader imgui stb_image)
# Links sdl and Vulkan libraries
target_link_libraries(run_engine Vulkan::Vulkan sdl2)
# Ensures Shaders are built before run_engine is
add_dependencies(run_engine Shaders)
