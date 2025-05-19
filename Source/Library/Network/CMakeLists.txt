# The Network library is created by the def_library macro in the main CMakeLists.txt

# Only use RakNetStub when KAI_BUILD_RAKNET is OFF
if (NOT KAI_BUILD_RAKNET)
    add_definitions(-DKAI_USE_RAKNET_STUB)
endif()

if (KAI_BUILD_RAKNET)
    # Add include directories for RakNet
    include_directories(${KAI_HOME}/Ext/RakNet/Source)
    
    # Add compile definition to enable RakNet integration
    add_definitions(-DKAI_USE_RAKNET)
endif()

# Add network-specific compile definitions
add_definitions(-DKAI_NETWORK_DLL)