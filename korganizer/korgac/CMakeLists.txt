remove_definitions(-DKDE_DEFAULT_DEBUG_AREA=5850) #remove korganizer debug area
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5890)

add_subdirectory(pixmaps) 
add_subdirectory(tests) 
include_directories(
 ${CMAKE_SOURCE_DIR}/korganizer
 ${CMAKE_BINARY_DIR}/korganizer )


########### next target ###############

set(korgac_SRCS 
   korgacmain.cpp 
   alarmdialog.cpp 
   alarmdockwindow.cpp 
   koalarmclient.cpp)

set(korganizer_xml ${CMAKE_SOURCE_DIR}/korganizer/org.kde.korganizer.Korganizer.xml)
QT4_ADD_DBUS_INTERFACE(korgac_SRCS ${korganizer_xml} korganizer_interface)



qt4_add_dbus_adaptor(korgac_SRCS org.kde.korganizer.KOrgac.xml koalarmclient.h KOAlarmClient)


kde4_add_app_icon(korgac_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/korgac.png")

kde4_add_executable(korgac ${korgac_SRCS})

target_link_libraries(korgac ${KDE4_KDEUI_LIBS} ${KDE4_PHONON_LIBS} ${KDEPIMLIBS_KCAL_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS} korganizer_eventviewer korganizer_core)

install(TARGETS korgac ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES korgac.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})
install(FILES org.kde.korganizer.KOrgac.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
