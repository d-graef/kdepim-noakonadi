/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001,2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <knumvalidator.h>

#include <libkcal/event.h>

#include <libkdepim/kdateedit.h>

#include "koprefs.h"

#include "koeditorrecurrence.h"
#include "koeditorrecurrence.moc"

/////////////////////////// RecurBase ///////////////////////////////

RecurBase::RecurBase( QWidget *parent, const char *name ) :
  QWidget( parent, name )
{
  mFrequencyEdit = new QSpinBox( 1, 9999, 1, this );
  mFrequencyEdit->setValue( 1 );
}

QWidget *RecurBase::frequencyEdit()
{
  return mFrequencyEdit;
}

void RecurBase::setFrequency( int f )
{
  if ( f < 1 ) f = 1;

  mFrequencyEdit->setValue( f );
}

int RecurBase::frequency()
{
  return mFrequencyEdit->value();
}

/////////////////////////// RecurDaily ///////////////////////////////

RecurDaily::RecurDaily( QWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  QLabel *preLabel = new QLabel( i18n("Recur every"), this );
  topLayout->addWidget( preLabel );

  topLayout->addWidget( frequencyEdit() );

  QLabel *postLabel = new QLabel( i18n("day(s)"), this );
  topLayout->addWidget( postLabel );
}


/////////////////////////// RecurWeekly ///////////////////////////////

RecurWeekly::RecurWeekly( QWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  topLayout->addStretch( 1 );

  QBoxLayout *weeksLayout = new QHBoxLayout( topLayout );

  QLabel *preLabel = new QLabel( i18n("Recur every"), this );
  weeksLayout->addWidget( preLabel );

  weeksLayout->addWidget( frequencyEdit() );

  QLabel *postLabel = new QLabel( i18n("week(s) on:"), this );
  weeksLayout->addWidget( postLabel );

  QHBox *dayBox = new QHBox( this );
  topLayout->addWidget( dayBox, 1, AlignVCenter );
  // TODO: Create day checks boxes as array
  // TODO: Respect start of week setting
  mSundayBox    = new QCheckBox(i18n("Sun"), dayBox );
  mMondayBox    = new QCheckBox(i18n("Mon"), dayBox );
  mTuesdayBox   = new QCheckBox(i18n("Tue"), dayBox );
  mWednesdayBox = new QCheckBox(i18n("Wed"), dayBox );
  mThursdayBox  = new QCheckBox(i18n("Thu"), dayBox );
  mFridayBox    = new QCheckBox(i18n("Fri"), dayBox );
  mSaturdayBox  = new QCheckBox(i18n("Sat"), dayBox );

  topLayout->addStretch( 1 );
}

void RecurWeekly::setDays( const QBitArray &days )
{
  mMondayBox->setChecked   ( days.testBit( 0 ) );
  mTuesdayBox->setChecked  ( days.testBit( 1 ) );
  mWednesdayBox->setChecked( days.testBit( 2 ) );
  mThursdayBox->setChecked ( days.testBit( 3 ) );
  mFridayBox->setChecked   ( days.testBit( 4 ) );
  mSaturdayBox->setChecked ( days.testBit( 5 ) );
  mSundayBox->setChecked   ( days.testBit( 6 ) );
}

QBitArray RecurWeekly::days()
{
  QBitArray days( 7 );
  
  days.setBit( 0, mMondayBox->isChecked() );
  days.setBit( 1, mTuesdayBox->isChecked() );
  days.setBit( 2, mWednesdayBox->isChecked() );
  days.setBit( 3, mThursdayBox->isChecked() );
  days.setBit( 4, mFridayBox->isChecked() );
  days.setBit( 5, mSaturdayBox->isChecked() );
  days.setBit( 6, mSundayBox->isChecked() );

  return days;
}

/////////////////////////// RecurMonthly ///////////////////////////////

RecurMonthly::RecurMonthly( QWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  QButtonGroup *buttonGroup = new QButtonGroup( this );
  buttonGroup->setFrameStyle( QFrame::NoFrame );
  topLayout->addWidget( buttonGroup, 1, AlignVCenter );

  QGridLayout *buttonLayout = new QGridLayout( buttonGroup, 3, 2 );
  buttonLayout->setSpacing( KDialog::spacingHint() );

  mByDayRadio = new QRadioButton( i18n("Recur on the"), buttonGroup );
  buttonLayout->addWidget( mByDayRadio, 0, 0 );

  mByDayCombo = new QComboBox( buttonGroup );
  mByDayCombo->setSizeLimit( 7 );
  mByDayCombo->insertItem( i18n("1st") );
  mByDayCombo->insertItem( i18n("2nd") );
  mByDayCombo->insertItem( i18n("3rd") );
  mByDayCombo->insertItem( i18n("4th") );
  mByDayCombo->insertItem( i18n("5th") );
  mByDayCombo->insertItem( i18n("6th") );
  mByDayCombo->insertItem( i18n("7th") );
  mByDayCombo->insertItem( i18n("8th") );
  mByDayCombo->insertItem( i18n("9th") );
  mByDayCombo->insertItem( i18n("10th") );
  mByDayCombo->insertItem( i18n("11th") );
  mByDayCombo->insertItem( i18n("12th") );
  mByDayCombo->insertItem( i18n("13th") );
  mByDayCombo->insertItem( i18n("14th") );
  mByDayCombo->insertItem( i18n("15th") );
  mByDayCombo->insertItem( i18n("16th") );
  mByDayCombo->insertItem( i18n("17th") );
  mByDayCombo->insertItem( i18n("18th") );
  mByDayCombo->insertItem( i18n("19th") );
  mByDayCombo->insertItem( i18n("20th") );
  mByDayCombo->insertItem( i18n("21st") );
  mByDayCombo->insertItem( i18n("22nd") );
  mByDayCombo->insertItem( i18n("23rd") );
  mByDayCombo->insertItem( i18n("24th") );
  mByDayCombo->insertItem( i18n("25th") );
  mByDayCombo->insertItem( i18n("26th") );
  mByDayCombo->insertItem( i18n("27th") );
  mByDayCombo->insertItem( i18n("28th") );
  mByDayCombo->insertItem( i18n("29th") );
  mByDayCombo->insertItem( i18n("30th") );
  mByDayCombo->insertItem( i18n("31st") );
  buttonLayout->addWidget( mByDayCombo, 0, 1 );

  QLabel *byDayLabel = new QLabel( i18n("day"), buttonGroup );
  buttonLayout->addWidget( byDayLabel, 0, 2 );


  mByPosRadio = new QRadioButton( i18n("Recur on the" ), buttonGroup);
  buttonLayout->addWidget( mByPosRadio, 1, 0 );

  mByPosCountCombo = new QComboBox( buttonGroup );
  mByPosCountCombo->insertItem( i18n("1st") );
  mByPosCountCombo->insertItem( i18n("2nd") );
  mByPosCountCombo->insertItem( i18n("3rd") );
  mByPosCountCombo->insertItem( i18n("4th") );
  mByPosCountCombo->insertItem( i18n("5th") );
  buttonLayout->addWidget( mByPosCountCombo, 1, 1 );

  mByPosWeekdayCombo = new QComboBox( buttonGroup );
  mByPosWeekdayCombo->insertItem( i18n("Monday") );
  mByPosWeekdayCombo->insertItem( i18n("Tuesday") );
  mByPosWeekdayCombo->insertItem( i18n("Wednesday") );
  mByPosWeekdayCombo->insertItem( i18n("Thursday") );
  mByPosWeekdayCombo->insertItem( i18n("Friday") );
  mByPosWeekdayCombo->insertItem( i18n("Saturday") );
  mByPosWeekdayCombo->insertItem( i18n("Sunday") );
  buttonLayout->addWidget( mByPosWeekdayCombo, 1, 2 );

  QLabel *preLabel = new QLabel( i18n("every"), this );
  topLayout->addWidget( preLabel );

  topLayout->addWidget( frequencyEdit() );

  QLabel *postLabel = new QLabel( i18n("month(s)"), this );
  topLayout->addWidget( postLabel );
}

void RecurMonthly::setByDay( int day )
{
  mByDayRadio->setChecked( true );
  mByDayCombo->setCurrentItem( day );
}

void RecurMonthly::setByPos( int count, int weekday )
{
  mByPosRadio->setChecked( true );
  mByPosCountCombo->setCurrentItem( count );
  mByPosWeekdayCombo->setCurrentItem( weekday );
}

bool RecurMonthly::byDay()
{
  return mByDayRadio->isChecked();
}

bool RecurMonthly::byPos()
{
  return mByPosRadio->isChecked();
}

int RecurMonthly::day()
{
  return mByDayCombo->currentItem() + 1;
}

int RecurMonthly::count()
{
  return mByPosCountCombo->currentItem() + 1;
}

int RecurMonthly::weekday()
{
  return mByPosWeekdayCombo->currentItem();
}

/////////////////////////// RecurYearly ///////////////////////////////

RecurYearly::RecurYearly( QWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  QButtonGroup *buttonGroup = new QButtonGroup( this );
  buttonGroup->setFrameStyle( QFrame::NoFrame );
  topLayout->addWidget( buttonGroup, 1, AlignVCenter );

  QGridLayout *buttonLayout = new QGridLayout( buttonGroup, 3, 2 );

  mByMonthRadio = new QRadioButton( i18n("Recur in the month of"),
                                    buttonGroup);
  buttonLayout->addWidget( mByMonthRadio, 0, 0 );

  mByMonthCombo = new QComboBox( buttonGroup );
  mByMonthCombo->insertItem( i18n("January") );
  mByMonthCombo->insertItem( i18n("February") );
  mByMonthCombo->insertItem( i18n("March") );
  mByMonthCombo->insertItem( i18n("April") );
  mByMonthCombo->insertItem( i18n("May") );
  mByMonthCombo->insertItem( i18n("June") );
  mByMonthCombo->insertItem( i18n("July") );
  mByMonthCombo->insertItem( i18n("August") );
  mByMonthCombo->insertItem( i18n("September") );
  mByMonthCombo->insertItem( i18n("October") );
  mByMonthCombo->insertItem( i18n("November") );
  mByMonthCombo->insertItem( i18n("December") );
  buttonLayout->addWidget( mByMonthCombo, 0, 1 );

  
  buttonLayout->setRowStretch( 1, 1 );


  mByDayRadio = new QRadioButton( i18n("Recur on this day"),
                                  buttonGroup);
  buttonLayout->addMultiCellWidget( mByDayRadio, 2, 2, 0, 1 );


  QLabel *preLabel = new QLabel( i18n("every"), this );
  topLayout->addWidget( preLabel );

  topLayout->addWidget( frequencyEdit() );

  QLabel *postLabel = new QLabel( i18n("year(s)"), this );
  topLayout->addWidget( postLabel );
}

void RecurYearly::setByDay()
{
  mByDayRadio->setChecked( true );
}

void RecurYearly::setByMonth( int month )
{
  mByMonthRadio->setChecked( true );
  mByMonthCombo->setCurrentItem( month - 1 );
}

bool RecurYearly::byMonth()
{
  return mByMonthRadio->isChecked();
}

bool RecurYearly::byDay()
{
  return mByDayRadio->isChecked();
}

int RecurYearly::month()
{
  return mByMonthCombo->currentItem() + 1;
}

//////////////////////////// ExceptionsWidget //////////////////////////

ExceptionsWidget::ExceptionsWidget( QWidget *parent, const char *name ) :
  QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  QGroupBox *groupBox = new QGroupBox( 1, Horizontal, i18n("Exceptions"),
                                       this );
  topLayout->addWidget( groupBox );

  QWidget *box = new QWidget( groupBox );

  QGridLayout *boxLayout = new QGridLayout( box );

  mExceptionDateEdit = new KDateEdit( box );
  mExceptionDateEdit->setDate( QDate::currentDate() );
  boxLayout->addWidget( mExceptionDateEdit, 0, 0 );

  QPushButton *addExceptionButton = new QPushButton( i18n("Add"), box );
  boxLayout->addWidget( addExceptionButton, 1, 0 );
  QPushButton *changeExceptionButton = new QPushButton( i18n("Change"), box );
  boxLayout->addWidget( changeExceptionButton, 2, 0 );
  QPushButton *deleteExceptionButton = new QPushButton( i18n("Delete"), box );
  boxLayout->addWidget( deleteExceptionButton, 3, 0 );

  mExceptionList = new QListBox( box );
  boxLayout->addMultiCellWidget( mExceptionList, 0, 3, 1, 1 );

  boxLayout->setRowStretch( 4, 1 );
  boxLayout->setColStretch( 1, 3 );

  connect( addExceptionButton, SIGNAL( clicked() ),
	   SLOT( addException() ) );
  connect( changeExceptionButton, SIGNAL( clicked() ),
	   SLOT( changeException() ) );
  connect( deleteExceptionButton, SIGNAL( clicked() ),
	   SLOT( deleteException() ) );
}

void ExceptionsWidget::addException()
{
  QDate date = mExceptionDateEdit->date();
  QString dateStr = KGlobal::locale()->formatDate( date );
  if( !mExceptionList->findItem( dateStr ) ) {
    mExceptionDates.append( date );
    mExceptionList->insertItem( dateStr );
  }
}

void ExceptionsWidget::changeException()
{
  int pos = mExceptionList->currentItem();
  if ( pos < 0 ) return;

  QDate date = mExceptionDateEdit->date();
  mExceptionDates[ pos ] = date;
  mExceptionList->changeItem( KGlobal::locale()->formatDate( date ), pos );
}

void ExceptionsWidget::deleteException()
{
  int pos = mExceptionList->currentItem();
  if ( pos < 0 ) return;

  mExceptionDates.remove( mExceptionDates.at( pos ) );
  mExceptionList->removeItem( pos );
}

void ExceptionsWidget::setDates( const DateList &dates )
{
  mExceptionList->clear();
  mExceptionDates.clear();
  DateList::ConstIterator dit;
  for ( dit = dates.begin(); dit != dates.end(); ++dit ) {
    mExceptionList->insertItem( KGlobal::locale()->formatDate(* dit ) );
    mExceptionDates.append( *dit );
  }
}

DateList ExceptionsWidget::dates()
{
  return mExceptionDates;
}

/////////////////////////////// Main Widget /////////////////////////////

KOEditorRecurrence::KOEditorRecurrence( QWidget* parent, const char *name ) :
  QWidget( parent, name )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  mEnabledCheck = new QCheckBox( i18n("Enable Recurrence"), this );
  connect( mEnabledCheck, SIGNAL( toggled( bool ) ),
           SLOT( setEnabled( bool ) ) );
  topLayout->addMultiCellWidget( mEnabledCheck, 0, 0, 0, 1 );


  mTimeGroupBox = new QGroupBox( 1, Horizontal, i18n("Appointment Time "),
                                 this );
  topLayout->addMultiCellWidget( mTimeGroupBox, 1, 1 , 0 , 1 );

//  QFrame *timeFrame = new QFrame( mTimeGroupBox );
//  QBoxLayout *layoutTimeFrame = new QHBoxLayout( timeFrame );
//  layoutTimeFrame->setSpacing( KDialog::spacingHint() );

  mDateTimeLabel = new QLabel( mTimeGroupBox );
//  mDateTimeLabel = new QLabel( timeFrame );
//  layoutTimeFrame->addWidget( mDateTimeLabel );


  mRuleBox = new QGroupBox( 1, Vertical, i18n("Recurrence Rule"), this );
  topLayout->addMultiCellWidget( mRuleBox, 2, 2, 0, 1 );

  QButtonGroup *ruleButtonGroup = new QButtonGroup( 1, Horizontal, mRuleBox );
  ruleButtonGroup->setFrameStyle( QFrame::NoFrame );

  mDailyButton = new QRadioButton( i18n("Daily"), ruleButtonGroup );
  mWeeklyButton = new QRadioButton( i18n("Weekly"), ruleButtonGroup );
  mMonthlyButton = new QRadioButton( i18n("Monthly"), ruleButtonGroup );
  mYearlyButton = new QRadioButton( i18n("Yearly"), ruleButtonGroup );

  QFrame *ruleSepFrame = new QFrame( mRuleBox );
  ruleSepFrame->setFrameStyle( QFrame::VLine | QFrame::Sunken );

  mRuleStack = new QWidgetStack( mRuleBox );

  mDaily = new RecurDaily( mRuleStack );
  mRuleStack->addWidget( mDaily, 0 );

  mWeekly = new RecurWeekly( mRuleStack );
  mRuleStack->addWidget( mWeekly, 0 );

  mMonthly = new RecurMonthly( mRuleStack );
  mRuleStack->addWidget( mMonthly, 0 );

  mYearly = new RecurYearly( mRuleStack );
  mRuleStack->addWidget( mYearly, 0 );

  connect( mDailyButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRule() ) );
  connect( mWeeklyButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRule() ) );
  connect( mMonthlyButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRule() ) );
  connect( mYearlyButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRule() ) );


  mRangeGroupBox = new QGroupBox( 1, Horizontal, i18n("Recurrence Range"),
                                  this );
  topLayout->addWidget( mRangeGroupBox, 3, 0 );

  QWidget *rangeBox = new QWidget( mRangeGroupBox );
  QVBoxLayout *rangeLayout = new QVBoxLayout( rangeBox );
  rangeLayout->setSpacing( KDialog::spacingHint() );

  mStartDateLabel = new QLabel( i18n("Begin on:"), rangeBox );
  rangeLayout->addWidget( mStartDateLabel );

  QButtonGroup *rangeButtonGroup = new QButtonGroup;

  mNoEndDateButton = new QRadioButton( i18n("No ending date"), rangeBox );
  rangeButtonGroup->insert( mNoEndDateButton );
  rangeLayout->addWidget( mNoEndDateButton );

  QBoxLayout *durationLayout = new QHBoxLayout( rangeLayout );
  durationLayout->setSpacing( KDialog::spacingHint() );

  mEndDurationButton = new QRadioButton( i18n("End after"), rangeBox );
  rangeButtonGroup->insert( mEndDurationButton );
  durationLayout->addWidget( mEndDurationButton );
  
  mEndDurationEdit = new QSpinBox( 1, 9999, 1, rangeBox );
  durationLayout->addWidget( mEndDurationEdit );

  QLabel *endDurationLabel = new QLabel( i18n("occurrence(s)"), rangeBox );
  durationLayout ->addWidget( endDurationLabel );

  QBoxLayout *endDateLayout = new QHBoxLayout( rangeLayout );
  endDateLayout->setSpacing( KDialog::spacingHint() );
  
  mEndDateButton = new QRadioButton( i18n("End by:"), rangeBox );
  rangeButtonGroup->insert( mEndDateButton );
  endDateLayout->addWidget( mEndDateButton );
  
  mEndDateEdit = new KDateEdit( rangeBox );
  endDateLayout->addWidget( mEndDateEdit );

  endDateLayout->addStretch( 1 );

  connect( mNoEndDateButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRange() ) );
  connect( mEndDurationButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRange() ) );
  connect( mEndDateButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRange() ) );


  mExceptions = new ExceptionsWidget( this );
  topLayout->addWidget( mExceptions, 3, 1 );
}

KOEditorRecurrence::~KOEditorRecurrence()
{
}

void KOEditorRecurrence::setEnabled( bool enabled )
{
//  kdDebug() << "KOEditorRecurrence::setEnabled(): " << (enabled ? "on" : "off") << endl;

  mTimeGroupBox->setEnabled( enabled );
  mRuleBox->setEnabled( enabled );
  mRangeGroupBox->setEnabled( enabled );
  mExceptions->setEnabled( enabled );
}

void KOEditorRecurrence::showCurrentRule()
{
  if ( mDailyButton->isChecked() ) mRuleStack->raiseWidget( mDaily );
  else if ( mWeeklyButton->isChecked() ) mRuleStack->raiseWidget( mWeekly );
  else if ( mMonthlyButton->isChecked() ) mRuleStack->raiseWidget( mMonthly );
  else mRuleStack->raiseWidget( mYearly );
}

void KOEditorRecurrence::showCurrentRange()
{
  mEndDurationEdit->setEnabled( mEndDurationButton->isChecked() );
  mEndDateEdit->setEnabled( mEndDateButton->isChecked() );
}

void KOEditorRecurrence::setDateTimes( QDateTime start, QDateTime )
{
//  kdDebug() << "KOEditorRecurrence::setDateTimes" << endl;

  mStartDateLabel->setText( i18n("Begins on: %1")
      .arg( KGlobal::locale()->formatDate( start.date() ) ) );
}

void KOEditorRecurrence::setDefaults( QDateTime from, QDateTime to, bool )
{
  setDateTimes( from, to );

  mStartDateLabel->setText( i18n("Begins on: %1")
      .arg( KGlobal::locale()->formatDate(from.date() ) ) );

  bool enabled = false;
  mEnabledCheck->setChecked( enabled );
  setEnabled( enabled );

  mNoEndDateButton->setChecked( true );
  mWeeklyButton->setChecked( true );

  mDaily->setFrequency( 1 );

  mWeekly->setFrequency( 1 );
  QBitArray days( 7 );
  days.fill( 0 );
  mWeekly->setDays( days );

  mMonthly->setFrequency( 1 );
  mMonthly->setByDay( from.date().day()-1 );

  mYearly->setFrequency( 1 );
  mYearly->setByDay();
}

void KOEditorRecurrence::readEvent(Event *event)
{
  QBitArray rDays( 7 );
  QPtrList<Recurrence::rMonthPos> rmp;
  QPtrList<int> rmd;
  int day = 0;
  int count = 0;
  int month = 0;

  setDateTimes( event->dtStart(), event->dtEnd() );

  Recurrence *r = event->recurrence();
  int f = r->frequency();

  int recurs = r->doesRecur();

  mEnabledCheck->setChecked( recurs );
  setEnabled( recurs );

  switch ( recurs ) {
    case Recurrence::rNone:
      break;
    case Recurrence::rDaily:
      mDailyButton->setChecked( true );
      mDaily->setFrequency( f );
      break;
    case Recurrence::rWeekly:
      mWeeklyButton->setChecked( true );
      mWeekly->setFrequency( f );
      mWeekly->setDays( r->days() );
      break;
    case Recurrence::rMonthlyPos:
      // we only handle one possibility in the list right now,
      // so I have hardcoded calls with first().  If we make the GUI
      // more extended, this can be changed.
      mMonthlyButton->setChecked( true );

      rmp = r->monthPositions();
      if ( rmp.first()->negative )
        count = 5 - rmp.first()->rPos - 1;
      else
        count = rmp.first()->rPos - 1;
      day = 0;
      while ( !rmp.first()->rDays.testBit( day ) ) ++day;
      mMonthly->setByPos( count, day );

      mMonthly->setFrequency( f );

      break;
    case Recurrence::rMonthlyDay:
      mMonthlyButton->setChecked( true );

      rmd = r->monthDays();
      day = *rmd.first() - 1;
      mMonthly->setByDay( day );

      mMonthly->setFrequency( f );

      break;
    case Recurrence::rYearlyMonth:
    case Recurrence::rYearlyDay:
      mYearlyButton->setChecked( true );

      rmd = r->yearNums();
      month = *rmd.first(); 
      if ( month == event->dtStart().date().month() ) {
        mYearly->setByDay();
      } else {
        mYearly->setByMonth( month );
      }

      mYearly->setFrequency( f );
      break;
    default:
      break;
  }

  mStartDateLabel->setText( i18n("Begins on: %1")
      .arg( KGlobal::locale()->formatDate( event->dtStart().date() ) ) );

  if ( r->doesRecur() ) {
    if ( r->duration() == -1 ) {
      mNoEndDateButton->setChecked( true );
    } else if ( r->duration() == 0 ) {
      mEndDateButton->setChecked( true );
      mEndDateEdit->setDate( r->endDate() );
    } else {
      mEndDurationButton->setChecked( true );
      mEndDurationEdit->setValue( r->duration( ) );
    }
  }

  mExceptions->setDates( event->exDates() );
}

void KOEditorRecurrence::writeEvent( Event *event )
{
  Recurrence *r = event->recurrence();
    
  // clear out any old settings;
  r->unsetRecurs();

  if ( mEnabledCheck->isChecked() ) {
    int duration;
    QDate endDate;

    if ( mNoEndDateButton->isChecked() ) {
      duration = -1;
    } else if ( mEndDurationButton->isChecked() ) {
      duration = mEndDurationEdit->value();
    } else {
      duration = 0;
      endDate = mEndDateEdit->date();
    }

    if ( mDailyButton->isChecked() ) {
      int freq = mDaily->frequency();
      if ( duration != 0 ) r->setDaily( freq, duration );
      else  r->setDaily( freq, endDate );
    } else if ( mWeeklyButton->isChecked() ) {
      int freq = mWeekly->frequency();
      QBitArray days = mWeekly->days();
      if ( duration != 0 ) r->setWeekly( freq, days, duration );
      else r->setWeekly( freq, days, endDate );
    } else if ( mMonthlyButton->isChecked() ) {
      int freq = mMonthly->frequency();
      if ( mMonthly->byPos() ) {
        int pos = mMonthly->count();

        QBitArray days( 7 );
        days.fill( false );

        days.setBit( mMonthly->weekday() );
        if ( duration != 0 )
	  r->setMonthly( Recurrence::rMonthlyPos, freq, duration );
	else
	  r->setMonthly( Recurrence::rMonthlyPos, freq, endDate );
        r->addMonthlyPos( pos, days );
      } else {
	// it's by day
        int day = mMonthly->day();

        if ( duration != 0 ) {
          r->setMonthly( Recurrence::rMonthlyDay, freq, duration );
        } else {
          r->setMonthly( Recurrence::rMonthlyDay, freq, endDate );
        }
        r->addMonthlyDay( day );
      }
    } else if ( mYearlyButton->isChecked() ) {
      int freq = mYearly->frequency();

      int month;
      if ( mYearly->byMonth() ) {
        month = mYearly->month();
      } else {
        month = event->dtStart().date().month();
      }
      if ( duration != 0 ) {
	r->setYearly( Recurrence::rYearlyMonth, freq, duration );
      } else {
	r->setYearly( Recurrence::rYearlyMonth, freq, endDate );
      }

      r->addYearlyNum( month );
    }

    event->setExDates( mExceptions->dates() );
  }
}

void KOEditorRecurrence::setDateTimeStr( const QString &str )
{
  mDateTimeLabel->setText( str );
}

bool KOEditorRecurrence::validateInput()
{
  // Check input here

  return true;
}
