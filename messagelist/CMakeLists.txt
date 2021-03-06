project(messagelist)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(libmessagelist_SRCS
    core/aggregation.cpp
    core/configprovider.cpp
    core/delegate.cpp
    core/filter.cpp
    core/item.cpp
    core/groupheaderitem.cpp
    core/manager.cpp
    core/messageitem.cpp
    core/messageitemsetmanager.cpp
    core/model.cpp
    core/modelinvariantindex.cpp
    core/modelinvariantrowmapper.cpp
    core/optionset.cpp
    core/theme.cpp
    core/themedelegate.cpp
    core/storagemodelbase.cpp
    core/sortorder.cpp
    core/subjectutils.cpp
    core/view.cpp
    core/widgetbase.cpp

    utils/aggregationcombobox.cpp
    utils/aggregationconfigbutton.cpp
    utils/aggregationeditor.cpp
    utils/comboboxutils.cpp
    utils/configureaggregationsdialog.cpp
    utils/configurethemesdialog.cpp
    utils/optionseteditor.cpp
    utils/themecombobox.cpp
    utils/themeconfigbutton.cpp
    utils/themeeditor.cpp

#    pane.cpp
#    storagemodel.cpp
#    widget.cpp
)

set(libmessagelist_core_HEADERS
    core/enums.h
    core/item.h
    core/messageitem.h
    core/model.h
    core/modelinvariantindex.h
    core/modelinvariantrowmapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/core/settings.h
    core/storagemodelbase.h
    core/view.h
    core/widgetbase.h
)

set(libmessagelist_utils_HEADERS
    utils/aggregationcombobox.h
    utils/aggregationconfigbutton.h
    utils/configureaggregationsdialog.h
    utils/configurethemesdialog.h
    utils/themecombobox.h
    utils/themeconfigbutton.h
)

set(libmessagelist_HEADERS
    pane.h
    storagemodel.h
    widget.h
)

kde4_add_kcfg_files(libmessagelist_SRCS USE_RELATIVE_PATH
    core/settings.kcfgc
)

kde4_add_library(messagelist SHARED ${libmessagelist_SRCS})
target_link_libraries(messagelist
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
#    ${KDEPIMLIBS_AKONADI_LIBS}
#    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    messagecore
)

set_target_properties(messagelist PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS messagelist ${INSTALL_TARGETS_DEFAULT_ARGS})
#install( FILES ${libmessagelist_HEADERS}
#         DESTINATION ${INCLUDE_INSTALL_DIR}/messagelist
#         COMPONENT devel)
#install( FILES ${libmessagelist_core_HEADERS}
#         DESTINATION ${INCLUDE_INSTALL_DIR}/messagelist/core
#         COMPONENT devel)
#install( FILES ${libmessagelist_utils_HEADERS}
#         DESTINATION ${INCLUDE_INSTALL_DIR}/messagelist/utils
#         COMPONENT devel)

