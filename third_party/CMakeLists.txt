
# vulkan
add_library(vulkan INTERFACE)
set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)
target_sources(vulkan INTERFACE 
    ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h
)
target_include_directories(vulkan INTERFACE ${VULKAN_INCLUDE_DIR})

# DirectX-Headers
add_library(DirectX-Headers INTERFACE)
set(DIRECTX_HEADERS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DirectX-Headers/include)
target_sources(DirectX-Headers INTERFACE 
    ${DIRECTX_HEADERS_INCLUDE_DIR}/directx/d3d12.h
    ${DIRECTX_HEADERS_INCLUDE_DIR}/directx/d3dx12.h
    ${DIRECTX_HEADERS_INCLUDE_DIR}/directx/d3d12shader.h
)
target_include_directories(DirectX-Headers INTERFACE ${DIRECTX_HEADERS_INCLUDE_DIR})

# Metal C++ headers
if(NOT DEFINED ARHI_METAL_CPP_URL)
    set(ARHI_METAL_CPP_URL "https://developer.apple.com/metal/cpp/files/metal-cpp_26.4.zip")
endif()

# Fetch & setup Metal C++ headers
if(ARHI_METAL)
    message(STATUS "Fetching Metal C++ headers ...")
    FetchPackage(metal_cpp URL ${ARHI_METAL_CPP_URL})
    add_library(arhi-metal-cpp INTERFACE)
    target_include_directories(arhi-metal-cpp INTERFACE ${metal_cpp_SOURCE_DIR})
    target_link_libraries(arhi-metal-cpp INTERFACE "-framework Metal")
endif (ARHI_METAL)
