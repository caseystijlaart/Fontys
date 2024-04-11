project(Targets)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(UNIX)
    if(BUILD_CONTROL_UNIT)
        find_package(mongocxx REQUIRED)
        find_package(nlohmann_json REQUIRED)

        add_executable(ControlUnit control-unit.cpp)
        target_link_libraries(ControlUnit
            PRIVATE
            libs::hal-control
            libs::common
            paho-mqtt3a
            paho-mqttpp3
            mongo::mongocxx_shared
            nlohmann_json::nlohmann_json
        )
        set_target_properties(ControlUnit PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

        add_executable(ControlUnitTest control-unit-test.cpp)
        target_link_libraries(ControlUnitTest
            PRIVATE
            libs::hal-control
            libs::common
            paho-mqtt3a
            paho-mqttpp3
            mongo::mongocxx_shared
            nlohmann_json::nlohmann_json
        )
        set_target_properties(ControlUnitTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

        add_executable(DatabaseTest database-test.cpp)
        target_link_libraries(DatabaseTest
            PRIVATE
            libs::hal-control
            mongo::mongocxx_shared
            nlohmann_json::nlohmann_json
        )
        set_target_properties(DatabaseTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
    endif()

    if(BUILD_CAMERA_UNIT)
        find_package(OpenCV REQUIRED)

        add_executable(CameraUnit camera-unit.cpp)
        target_link_libraries(CameraUnit
        PRIVATE
        ${OpenCV_LIBS}
        libs::hal-camera
        libs::common
        nlohmann_json::nlohmann_json
    )

    set_target_properties(CameraUnit PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
    endif()

    if(BUILD_COMMON_UNIT)
    add_executable(mqtt_test mqttTest.cpp)

    target_link_libraries(mqtt_test
        PRIVATE
        libs::common
        paho-mqtt3a
        paho-mqttpp3
        nlohmann_json::nlohmann_json
    )
    
    set(TEST_EXECUTABLE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
    set_target_properties(mqtt_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_EXECUTABLE_OUTPUT_DIRECTORY})
    endif()
endif()
