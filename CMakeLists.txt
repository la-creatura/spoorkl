cmake_minimum_required(VERSION 3.16)

project(CPPoorkl VERSION 0.1 LANGUAGES CXX)

# CMakeLists.txt

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(OpenSSL REQUIRED)

#find_package(CURL REQUIRED)

#target_link_libraries(CPPoorkl PRIVATE CURL::libcurl)


#find_path(CURL_INCLUDE_DIR curl/curl.h PATHS "C:/libcurl/include")
#find_library(CURL_LIB curl PATHS "C:/libcurl/lib")



set(PROJECT_SOURCES
        main.cpp
        icon.rc
        resources.qrc
)


if(NOT DEFINED QT_VERSION_MAJOR)
    set(QT_VERSION_MAJOR 6)  # fallback if not set
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(CPPoorkl
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(CPPoorkl SHARED ${PROJECT_SOURCES})
    else()
        add_executable(CPPoorkl ${PROJECT_SOURCES})
    endif()
endif()

#install songs (i just got bored of this shit taking forever to install)

# не слышны в саду даже шорохи
# всё здесь замерло до утра
# если б знали бы как мне дороги
# подмосковные вечера
# если бы мне дороги
# подмосковные вечера
# речка движется и не движется
# вся из лунного серебра
# песня слышится и не слышится
# в эти тихие вечера
# песня слышится и не слышится
# в эти тихие вечера

# i give up on this one (autocorrect tried to fix it to say "i give up on the world")

# как то летом на рассвете
# загянул в соседний сад
# там смуглянка-молдаванка
# собирает виноград
# я краснею, я бледнею
# захотелость вдруг сказать
# станем над рекою
# зорки летние встречаaaть

# раааааааассссскуявныйййййй клён зеленый лист резной
# я влюбленныйййй и смущенный пред товой
# клён зеленый да клён кудрявый
# да раскуявный резной

# *раааааааассссскуявныйййййй клён зеленый лист резной
# *я влюбленныйййй и смущенный пред товой
# *клён зеленый да клён кудрявый
# *да раскуявный резной

# а смуглянка-молдаванка
# отвечала парню в лад
#... finished installing, fuck you openssl



#add_compile_options(/RTC1)
# sunny you gonna have a hell of a time installing all this shit

#target_include_directories(CPPoorkl PRIVATE ${CURL_INCLUDE_DIR})

add_subdirectory(external/QHotkey)

target_link_libraries(CPPoorkl PRIVATE
        Qt6::Widgets
        Qt6::Core
)

target_link_libraries(CPPoorkl PRIVATE OpenSSL::SSL OpenSSL::Crypto)

target_link_libraries(CPPoorkl PRIVATE QHotkey::QHotkey)


#target_link_options(CPPoorkl PRIVATE "/SUBSYSTEM:CONSOLE")

#target_link_directories(CPPoorkl PRIVATE ${CMAKE_SOURCE_DIR}/libs/openssl/lib)

#include_directories(${CMAKE_SOURCE_DIR}/libs)

#target_include_directories(CPPoorkl PRIVATE ${CMAKE_SOURCE_DIR}/libs)

if(6 VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.CPPoorkl)
endif()
set_target_properties(CPPoorkl PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        #LINK_FLAGS "/SUBSYSTEM:CONSOLE" # ENABLE FOR CONSOLE DEBUGGING
)



include(GNUInstallDirs)
install(TARGETS CPPoorkl
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


qt_finalize_executable(CPPoorkl)
