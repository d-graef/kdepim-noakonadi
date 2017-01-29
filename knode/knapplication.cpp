/*
    knapplication.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <kwindowsystem.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kconfig.h>

#include "knode.h"
#include "knapplication.h"
#include "knconvert.h"
#include "knglobals.h"
#include "knmainwidget.h"
#include "knapplication.moc"


int KNApplication::newInstance()
{

  QWidgetList widgetTList;
  static KNMainWindow* mainWin;

  kDebug(5003) <<"KNApplication::newInstance()";

  KConfigGroup conf(knGlobals.config(), "GENERAL");
  QString ver=conf.readEntry("Version");

  if(!ver.isEmpty() && ver!=KNODE_VERSION) { //new version installed
    if(KNConvert::needToConvert(ver)) { //we need to convert
      kDebug(5003) <<"KNApplication::newInstance() : conversion needed";
      KNConvert *convDlg=new KNConvert(ver);
      if(!convDlg->exec()) { //reject()
        if(convDlg->conversionDone()) //conversion has already happened but the user has canceled afterwards
          conf.writeEntry("Version", KNODE_VERSION);
        exit(0);
        return(0);
      } else //conversion done
        conf.writeEntry("Version", KNODE_VERSION);
      delete convDlg;
    }
    else //new version but no need to convert anything => just save the new version
      conf.writeEntry("Version", KNODE_VERSION);
  }

  widgetTList = topLevelWidgets();

   if (widgetTList.empty()) {
    if ( isSessionRestored() ) {
      int n = 1;
      while (KNMainWindow::canBeRestored(n)){
        if (KNMainWindow::classNameOfToplevel(n)=="KNMainWindow") {
          mainWin = new KNMainWindow;
          mainWin->restore(n);
        }
        n++;
      }
    }


    if (widgetTList.empty()) {
      mainWin = new KNMainWindow;
      mainWin->show();
    }
  }

  // Handle window activation and startup notification
  KUniqueApplication::newInstance();

  // process URLs...
  KNMainWidget *w = static_cast<KNMainWindow*>(mainWin)->mainWidget();
  w->handleCommandLine();

  kDebug(5003) <<"KNApplication::newInstance() done";
  return 0;
}
