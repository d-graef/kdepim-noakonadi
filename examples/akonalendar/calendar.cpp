/*
    This file is part of Akonadi.

    Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include <kapplication.h>
#include <kcmdlineargs.h>

#include "mainwindow.h"

int main(  int argc,  char **argv )
{
    KCmdLineArgs::init( argc,  argv,  "akonalendar", 0, ki18n( "Akonalendar" ), "0.1", ki18n( "A calendar demo application" ) );
    KApplication app;

    MainWindow window;
    window.show();

    return app.exec();
}
