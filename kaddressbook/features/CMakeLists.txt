
include_directories( ${CMAKE_SOURCE_DIR}/kaddressbook/interfaces ${CMAKE_SOURCE_DIR}/kaddressbook  )

########### next target ###############

set(kaddrbk_distributionlistng_PART_SRCS distributionlistngwidget.cpp )


kde4_add_plugin(kaddrbk_distributionlistng ${kaddrbk_distributionlistng_PART_SRCS})

target_link_libraries(kaddrbk_distributionlistng kaddressbookprivate kabinterfaces kabcommon kdepim ${KDEPIMLIBS_KABC_LIBS})

install(TARGETS kaddrbk_distributionlistng  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kaddrbk_resourceselection_PART_SRCS resourceselection.cpp )


kde4_add_plugin(kaddrbk_resourceselection ${kaddrbk_resourceselection_PART_SRCS})



target_link_libraries(kaddrbk_resourceselection kaddressbookprivate kabinterfaces ${KDEPIMLIBS_KABC_LIBS} ${QT_QT3SUPPORT_LIBS} ${KDE4_KDECORE_LIBS})

install(TARGETS kaddrbk_resourceselection  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES distributionlistng.desktop resourceselection.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kaddressbook)

