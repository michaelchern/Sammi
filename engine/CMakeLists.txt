set(ENGINE_ROOT_DIR "${SAMMI_ROOT_DIR}/engine")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")
set(ENGINE_ASSET_DIR "/asset")

set(DEPLOY_CONFIG_DIR "configs/deployment")
set(DEVELOP_CONFIG_DIR "configs/development")

set(vulkan_include ${THIRD_PARTY_DIR}/VulkanSDK/include)



add_subdirectory(3rdparty)



add_subdirectory(source/runtime)
add_subdirectory(source/editor)