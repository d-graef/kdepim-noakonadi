/*
    This file is part of KAddressbook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <unistd.h>

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qgroupbox.h>
#include <qwidgetfactory.h>
#include <qregexp.h>

#include <kaboutdata.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klistview.h>
#include <klocale.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kactivelabel.h>
#include <kdirwatch.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>

#include "kabprefs.h"

#include "kcmkabcustomfields.h"

extern "C"
{
  KCModule *create_kabcustomfields( QWidget *parent, const char * ) {
    return new KCMKabCustomFields( parent, "kcmkabcustomfields" );
  }
}

class PageItem : public QCheckListItem
{
  public:
    PageItem( QListView *parent, const QString &path )
      : QCheckListItem( parent, "", QCheckListItem::CheckBox ),
        mPath( path ), mIsActive( false )
    {
      mName = path.mid( path.findRev( '/' ) + 1 );

      QWidget *wdg = QWidgetFactory::create( mPath, 0, 0 );
      if ( wdg ) {
        setText( 0, wdg->caption() );

        QPixmap pm = QPixmap::grabWidget( wdg );
        QImage img = pm.convertToImage().smoothScale( 300, 300, QImage::ScaleMin );
        mPreview = img;

        QObjectList *list = wdg->queryList( "QWidget" );
        QObjectListIt it( *list );

        QMap<QString, QString> allowedTypes;
        allowedTypes.insert( "QLineEdit", i18n( "Text" ) );
        allowedTypes.insert( "QSpinBox", i18n( "Numeric Value" ) );
        allowedTypes.insert( "QCheckBox", i18n( "Boolean" ) );
        allowedTypes.insert( "QComboBox", i18n( "Selection" ) );
        allowedTypes.insert( "QDateTimeEdit", i18n( "Date & Time" ) );
        allowedTypes.insert( "KLineEdit", i18n( "Text" ) );
        allowedTypes.insert( "KDateTimeWidget", i18n( "Date & Time" ) );
        allowedTypes.insert( "KDatePicker", i18n( "Date" ) );

        while ( it.current() ) {
          if ( allowedTypes.find( it.current()->className() ) != allowedTypes.end() ) {
            QString name = it.current()->name();
            if ( name.startsWith( "X_" ) ) {
              new QListViewItem( this, name,
                                 allowedTypes[ it.current()->className() ],
                                 it.current()->className(),
                                 QWhatsThis::textFor( static_cast<QWidget*>( it.current() ) ) );
            }
          }

          ++it;
        }

        delete list;
      } else
        delete wdg;
    }

    QString name() const { return mName; }
    QString path() const { return mPath; }

    QPixmap preview()
    {
      return mPreview;
    }

    void setIsActive( bool isActive ) { mIsActive = isActive; }
    bool isActive() const { return mIsActive; }

  protected:
    void paintBranches( QPainter *p, const QColorGroup & cg, int w, int y, int h )
    {
      QListViewItem::paintBranches( p, cg, w, y, h );
    }

  private:
    QString mName;
    QString mPath;
    QPixmap mPreview;
    bool mIsActive;
};

KCMKabCustomFields::KCMKabCustomFields( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  initGUI();

  connect( mPageView, SIGNAL( selectionChanged( QListViewItem* ) ),
           this, SLOT( updatePreview( QListViewItem* ) ) );
  connect( mPageView, SIGNAL( clicked( QListViewItem* ) ),
           this, SLOT( itemClicked( QListViewItem* ) ) );

  connect( mDeleteButton, SIGNAL( clicked() ),
           this, SLOT( deleteFile() ) );
  connect( mImportButton, SIGNAL( clicked() ),
           this, SLOT( importFile() ) );
  connect( mDesignerButton, SIGNAL( clicked() ),
           this, SLOT( startDesigner() ) );

  load();

  // Install a dirwatcher that will detect newly created or removed designer files
  KDirWatch *dw = new KDirWatch( this );
  dw->addDir( kabLocalDir() + "contacteditorpages", true );
  connect( dw, SIGNAL( created(const QString&) ), SLOT( rebuildList() ) );
  connect( dw, SIGNAL( deleted(const QString&) ), SLOT( rebuildList() ) );
  connect( dw, SIGNAL( dirty(const QString&) ),   SLOT( rebuildList() ) );

}

void KCMKabCustomFields::deleteFile()
{
  QListViewItem *item = mPageView->selectedItem();
  if ( item ) {
    PageItem *pageItem = static_cast<PageItem*>( item->parent() ? item->parent() : item );
    if (KMessageBox::warningContinueCancel(this,
         i18n( "<qt>Do you really want to delete '<b>%1</b>'?</qt>").arg( pageItem->text(0) ), "", KGuiItem( i18n("&Delete"), "editdelete") )
         == KMessageBox::Continue)
      KIO::NetAccess::del( pageItem->path(), 0 );
  }
  // The actual view refresh will be done automagically by the slots connected to kdirwatch
}

void KCMKabCustomFields::importFile()
{
  KURL src = KFileDialog::getOpenFileName( QDir::homeDirPath(), i18n("*.ui|Designer files"),
                                              this, i18n("Import Page") );
  KURL dest = kabLocalDir() + "contacteditorpages/";
  dest.setFileName(src.fileName());
  KIO::NetAccess::file_copy( src, dest, -1, true, false, this );
  // The actual view refresh will be done automagically by the slots connected to kdirwatch
}


void KCMKabCustomFields::loadUiFiles()
{
  QStringList list = KGlobal::dirs()->findAllResources( "data", "kaddressbook/contacteditorpages/*.ui", true, true );
  for ( QStringList::iterator it = list.begin(); it != list.end(); ++it ) {
    new PageItem( mPageView, *it );
  }
}

void KCMKabCustomFields::rebuildList()
{
  QStringList ai = saveActivePages();
  updatePreview( 0 );
  mPageView->clear();
  loadUiFiles();
  loadActivePages(ai);
}

void KCMKabCustomFields::loadActivePages(const QStringList& ai)
{
  QListViewItemIterator it( mPageView );
  while ( it.current() ) {
    if ( it.current()->parent() == 0 ) {
      PageItem *item = static_cast<PageItem*>( it.current() );
      if ( ai.find( item->name() ) != ai.end() ) {
        item->setOn( true );
        item->setIsActive( true );
      }
    }

    ++it;
  }
}

void KCMKabCustomFields::load()
{
  loadActivePages(KABPrefs::instance()->mAdvancedCustomFields);
}

QStringList KCMKabCustomFields::saveActivePages()
{
  QListViewItemIterator it( mPageView, QListViewItemIterator::Checked |
                            QListViewItemIterator::Selectable );

  QStringList activePages;
  while ( it.current() ) {
    if ( it.current()->parent() == 0 ) {
      PageItem *item = static_cast<PageItem*>( it.current() );
      activePages.append( item->name() );
    }

    ++it;
  }

  return activePages;
}

void KCMKabCustomFields::save()
{
  KABPrefs::instance()->mAdvancedCustomFields =  saveActivePages();
  KABPrefs::instance()->writeConfig();
}

void KCMKabCustomFields::defaults()
{
}

void KCMKabCustomFields::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  bool noDesigner = KStandardDirs::findExe("designer").isEmpty();

  if ( noDesigner )
  {
    QString txt =
      i18n("<qt><b>Warning:</b> Qt Designer could not be found. It is probably not "
         "installed. You will only be able to import existing designer files!</qt>");
    QLabel *lbl = new QLabel( txt, this );
    layout->addWidget( lbl );
  }

  QHBoxLayout *hbox = new QHBoxLayout( layout, KDialog::spacingHint() );

  mPageView = new KListView( this );
  mPageView->addColumn( i18n( "Available Pages" ) );
  mPageView->setRootIsDecorated( true );
  mPageView->setAllColumnsShowFocus( true );
  mPageView->setFullWidth( true );
  hbox->addWidget( mPageView );

  QGroupBox *box = new QGroupBox(1, Qt::Horizontal, i18n("Preview of Selected Page"), this );

  mPagePreview = new QLabel( box );
  mPagePreview->setMinimumWidth( 300 );

  mPageDetails = new QLabel( box );

  hbox->addWidget( box );

  loadUiFiles();

  hbox = new QHBoxLayout( layout, KDialog::spacingHint() );

  QString cwHowto = i18n("<qt><p>This section allows you to add your own GUI"
                         "  Elements ('<i>Widgets</i>') to store your own values"
                         " into the address book. Proceed as described below:</p>"
                         "<ol>"
                         "<li>Click on '<i>Edit with Qt Designer</i>'"
                         "<li>In the dialog, select '<i>Widget</i>', then click <i>OK</i>"
                         "<li>Add your widgets to the form"
                         "<li>Save the file in the directory proposed by Qt Designer"
                         "<li>Close Qt Designer"
                         "</ol>"
                         "<p>In case you already have a designer file (*.ui) located"
                         " somewhere on your hard disk, simply choose '<i>Import Page</i>'</p>"
                         "<p><b>Important:</b> The name of each input widget you place within"
                         " the form must start with '<i>X_</i>'; so if you want the widget to"
                         " correspond to your custom entry '<i>X-Foo</i>', set the widget's"
                         " <i>name</i> property to '<i>X_Foo</i>'.</p></qt>"
                         "<p><b>Important:</b> The widget will edit custom fields with an"
                         " application name of KADDRESSBOOK.  To change the application name"
                         " to be edited, set the widget name in Qt Designer.</p>" );

  KActiveLabel *activeLabel = new KActiveLabel(
      i18n( "<a href=\"whatsthis:%1\">How does this work?</a>" ).arg(cwHowto), this );
  hbox->addWidget( activeLabel );

  // ### why is this needed? Looks like a KActiveLabel bug...
  activeLabel->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );

  hbox->addStretch( 1 );

  mDeleteButton = new QPushButton( i18n( "Delete Page" ), this);
  mDeleteButton->setEnabled( false );
  hbox->addWidget( mDeleteButton );
  mImportButton = new QPushButton( i18n( "Import Page" ), this);
  hbox->addWidget( mImportButton );
  mDesignerButton = new QPushButton( i18n( "Edit with Qt Designer..." ), this );
  hbox->addWidget( mDesignerButton );

  if ( noDesigner )
    mDesignerButton->setEnabled( false );
}

void KCMKabCustomFields::updatePreview( QListViewItem *item )
{
  bool widgetItemSelected = false;

  if ( item ) {
    if ( item->parent() ) {
      QString details = QString( "<qt><table>"
                                 "<tr><td align=\"right\"><b>%1</b></td><td>%2</td></tr>"
                                 "<tr><td align=\"right\"><b>%3</b></td><td>%4</td></tr>"
                                 "<tr><td align=\"right\"><b>%5</b></td><td>%6</td></tr>"
                                 "<tr><td align=\"right\"><b>%7</b></td><td>%8</td></tr>"
                                 "</table></qt>" )
                                .arg( i18n( "vCard Key:" ) )
                                .arg( item->text( 0 ).replace("X_","X-") )
                                .arg( i18n( "Type:" ) )
                                .arg( item->text( 1 ) )
                                .arg( i18n( "Classname:" ) )
                                .arg( item->text( 2 ) )
                                .arg( i18n( "Description:" ) )
                                .arg( item->text( 3 ) );

      mPageDetails->setText( details );

      PageItem *pageItem = static_cast<PageItem*>( item->parent() );
      mPagePreview->setPixmap( pageItem->preview() );
    } else {
      mPageDetails->setText( QString::null );

      PageItem *pageItem = static_cast<PageItem*>( item );
      mPagePreview->setPixmap( pageItem->preview() );

      widgetItemSelected = true;
    }

    mPagePreview->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  } else {
    mPagePreview->setPixmap( QPixmap() );
    mPagePreview->setFrameStyle( 0 );
    mPageDetails->setText( QString::null );
  }

  mDeleteButton->setEnabled( widgetItemSelected );
}

void KCMKabCustomFields::itemClicked( QListViewItem *item )
{
  if ( !item || item->parent() != 0 )
    return;

  PageItem *pageItem = static_cast<PageItem*>( item );

  if ( pageItem->isOn() != pageItem->isActive() ) {
    emit changed( true );
    pageItem->setIsActive( pageItem->isOn() );
  }
}

QString KCMKabCustomFields::kabLocalDir()
{
  QStringList kabdirs = locateLocal("data", "kaddressbook/");
  return kabdirs.grep( QRegExp( "^"+KGlobal::dirs()->localkdedir() ) ).first();
}

void KCMKabCustomFields::startDesigner()
{
  QString cmdLine = "designer";

  // check if path exists and create one if not.
  QString cepPath = kabLocalDir() +"contacteditorpages";
  if( !KGlobal::dirs()->exists(cepPath) ) {
    KIO::NetAccess::mkdir( cepPath, this );
  }

  // finnally jump there
  chdir(cepPath.local8Bit());

  QListViewItem *item = mPageView->selectedItem();
  if ( item ) {
    PageItem *pageItem = static_cast<PageItem*>( item->parent() ? item->parent() : item );
    cmdLine += " " + pageItem->path();
  }

  KRun::runCommand( cmdLine );
}

const KAboutData* KCMKabCustomFields::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkabcustomfields" ),
                                      I18N_NOOP( "KAddressBook Custom Fields Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c), 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  return about;
}

#include "kcmkabcustomfields.moc"
