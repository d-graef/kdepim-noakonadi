/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>                   
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef LOCATIONWIDGET_H
#define LOCATIONWIDGET_H


#include "extensionwidget.h"

class KComboBox;
class KHTMLPart;
class QPushButton;

namespace KABC {
class Address;
class AddressBook;
}

class LocationWidget : public ExtensionWidget
{
    Q_OBJECT

  public:
    LocationWidget( ViewManager*, QWidget *parent, const char *name = 0 );
    virtual ~LocationWidget();

    void addresseeSelectionChanged();

    QString title() const;
    QString identifier() const;

  private slots:
    void loadLocationPage();

  signals:
    void modified( KABC::Addressee::List );

  private:
    QString createUrl( const KABC::Address& );

    KABC::Address::List mAddressList;
    KComboBox *mAddressTypeCombo;
    KHTMLPart *mHTMLPart;

    QPushButton *mLoadButton;
};

#endif
