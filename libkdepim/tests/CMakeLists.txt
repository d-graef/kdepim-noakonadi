set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${Boost_INCLUDE_DIR} )

########### KTimeZoneComboBox unit test #############
set(ktimezonecomboboxtest_SRCS ktimezonecomboboxtest.cpp)
kde4_add_unit_test(ktimezonecomboboxtest TESTNAME ktimezonecomboboxtest ${ktimezonecomboboxtest_SRCS})

target_link_libraries(
  ktimezonecomboboxtest
  kdepim
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
)

########### next target ###############

set(testwizard_SRCS testwizard.cpp )


kde4_add_kcfg_files(testwizard_SRCS myconfig.kcfgc )


kde4_add_executable(testwizard TEST ${testwizard_SRCS})

target_link_libraries(testwizard  kdepim ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY})


########### next target ###############

set(testaddresseelineedit_SRCS testaddresseelineedit.cpp )



kde4_add_executable(testaddresseelineedit TEST ${testaddresseelineedit_SRCS})

target_link_libraries(testaddresseelineedit  kdepim ${KDE4_KDEUI_LIBS})


########### next target ###############

set(testdateedit_SRCS testdateedit.cpp )


kde4_add_executable(testdateedit TEST ${testdateedit_SRCS})

target_link_libraries(testdateedit  kdepim ${KDE4_KDEUI_LIBS})

########### next target ###############

set(testdistrlist_SRCS testdistrlist.cpp )


kde4_add_executable(testdistrlist TEST ${testdistrlist_SRCS})

target_link_libraries(testdistrlist kdepim ${KDE4_KDECORE_LIBS} ${KDEPIMLIBS_KABC_LIBS})

########### next target ###############

set(testldapclient_SRCS testldapclient.cpp )


kde4_add_executable(testldapclient TEST ${testldapclient_SRCS})

target_link_libraries(testldapclient kdepim ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KLDAP_LIBS})

########### next target ###############

set(testkmeditor_SRCS testkmeditor.cpp )


kde4_add_executable(testkmeditor TEST ${testkmeditor_SRCS})

target_link_libraries(testkmeditor  kdepim ${KDE4_KDEUI_LIBS} )



########### next target ###############

set(testkmeditorwin_SRCS testkmeditorwin.cpp )


kde4_add_executable(testkmeditorwin TEST ${testkmeditorwin_SRCS})

target_link_libraries(testkmeditorwin  kdepim ${KDE4_KDEUI_LIBS} )


########### next target ###############

set(testkincidencechooser_SRCS testkincidencechooser.cpp )


kde4_add_executable(testkincidencechooser TEST ${testkincidencechooser_SRCS})

target_link_libraries(testkincidencechooser  kdepim ${KDE4_KDEUI_LIBS} )

