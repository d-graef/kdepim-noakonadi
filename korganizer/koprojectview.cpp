// $Id$

#include <qlayout.h>
#include <qheader.h>
#include <qpushbutton.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "vcaldrag.h"
#include "calprinter.h"
#include "KGantt.h"

#include "koprojectview.h"
#include "koprojectview.moc"

KOProjectViewItem::KOProjectViewItem(Todo *event,KGanttItem* parentTask,
                                     const QString& text, 
	                             const QDateTime& start,
                                     const QDateTime& end) :
  KGanttItem(parentTask,text,start,end)
{
  mEvent = event;
}

KOProjectViewItem::~KOProjectViewItem()
{
}

Todo *KOProjectViewItem::event()
{
  return mEvent;
}


KOProjectView::KOProjectView(CalObject *calendar,QWidget* parent,
                             const char* name) :
  KOBaseView(calendar,parent,name)
{
  QBoxLayout *topLayout = new QVBoxLayout(this);

  QBoxLayout *topBar = new QHBoxLayout;
  topLayout->addLayout(topBar);
  
  QLabel *title = new QLabel(i18n("Project View"),this);
  title->setFrameStyle(QFrame::Panel|QFrame::Raised);
  topBar->addWidget(title,1);

  QPushButton *zoomIn = new QPushButton(i18n("Zoom In"),this);
  topBar->addWidget(zoomIn,0);
  connect(zoomIn,SIGNAL(clicked()),SLOT(zoomIn()));

  QPushButton *zoomOut = new QPushButton(i18n("Zoom Out"),this);
  topBar->addWidget(zoomOut,0);
  connect(zoomOut,SIGNAL(clicked()),SLOT(zoomOut()));

  // Externally controlled zooming is not possible with the current API of
  // KGantt.
  zoomIn->hide();
  zoomOut->hide();

  QPushButton *menuButton = new QPushButton(i18n("Select Mode"),this);
  topBar->addWidget(menuButton,0);
  connect(menuButton,SIGNAL(clicked()),SLOT(showModeMenu()));

  createMainTask();

  mGantt = new KGantt(mMainTask,this);
  topLayout->addWidget(mGantt,1);

#if 0
  mGantt->addHoliday(2000, 10, 3);
  mGantt->addHoliday(2001, 10, 3);
  mGantt->addHoliday(2000, 12, 24);

  for(int i=1; i<7; i++)
    mGantt->addHoliday(2001, 1, i);
#endif


/*  
  mTodoListView = new KOTodoListView(mCalendar,this);
  topLayout->addWidget(mTodoListView);

  mTodoListView->setRootIsDecorated(true);
  mTodoListView->setAllColumnsShowFocus(true);
  
  mTodoListView->addColumn(i18n("Summary"));
  mTodoListView->addColumn(i18n("Priority"));
  mTodoListView->setColumnAlignment(1,AlignHCenter);
  mTodoListView->addColumn(i18n("Due Date"));
  mTodoListView->addColumn(i18n("Due Time"));
  mTodoListView->addColumn(i18n("Sort Id"));
  mTodoListView->setColumnAlignment(4,AlignHCenter);
  
  mItemPopupMenu = new QPopupMenu;
  mItemPopupMenu->insertItem(i18n("Show..."), this,
                             SLOT (showTodo()));
  mItemPopupMenu->insertItem(i18n("Edit..."), this,
                             SLOT (editTodo()));
  mItemPopupMenu->insertItem(SmallIconSet("delete"), i18n("Delete"), this,
                             SLOT (deleteTodo()));
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(SmallIconSet("todo"), i18n("New To-Do..."), this,
                             SLOT (newTodo()));
  mItemPopupMenu->insertItem(i18n("New Sub-To-Do..."), this,
                             SLOT (newSubTodo()));
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(i18n("Purge Completed"), this,
                             SLOT(purgeCompleted()));
                       
  mPopupMenu = new QPopupMenu;
  mPopupMenu->insertItem(SmallIconSet("todo"), i18n("New To-Do"), this,
                         SLOT (newTodo()));
  mPopupMenu->insertItem(i18n("Purge Completed"), this,
                         SLOT(purgeCompleted()));
  
  // Double clicking conflicts with opening/closing the subtree                   
  QObject::connect(mTodoListView,SIGNAL(doubleClicked(QListViewItem *)),
                   this,SLOT(showItem(QListViewItem *)));
  QObject::connect(mTodoListView,SIGNAL(rightButtonClicked ( QListViewItem *,
                   const QPoint &, int )),
                   this,SLOT(popupMenu(QListViewItem *,const QPoint &,int)));
  QObject::connect(mTodoListView,SIGNAL(clicked(QListViewItem *)),
                   this,SLOT(itemClicked(QListViewItem *)));
  connect(mTodoListView,SIGNAL(todoDropped(Event *)),SLOT(updateView()));
*/
}

void KOProjectView::createMainTask()
{
  mMainTask = new KGanttItem(0,"main task",
                         QDateTime::currentDateTime(),
                         QDateTime::currentDateTime());
  mMainTask->setMode(KGanttItem::Rubberband);
  mMainTask->setStyle(KGanttItem::DrawBorder | KGanttItem::DrawText |
                      KGanttItem::DrawHandle);
}

void KOProjectView::updateView()
{
  kdDebug() << "KOProjectView::updateView()" << endl;

  // Clear Gantt view
  QList<KGanttItem> subs = mMainTask->getSubItems();
  KGanttItem *t=subs.first();
  while(t) {
    KGanttItem *nt=subs.next();
    delete t;
    t = nt;
  }

#if 0
  KGanttItem* t1 = new KGanttItem(mGantt->getMainTask(), "task 1, no subtasks", 
                             QDateTime::currentDateTime().addDays(10),
                             QDateTime::currentDateTime().addDays(20) );

  KGanttItem* t2 = new KGanttItem(mGantt->getMainTask(), "task 2, subtasks, no rubberband", 
                             QDateTime(QDate(2000,10,1)),
                             QDateTime(QDate(2000,10,31)) );
#endif

  QList<Todo> todoList = mCalendar->getTodoList();

/*
  kdDebug() << "KOProjectView::updateView(): Todo List:" << endl;
  Event *t;
  for(t = todoList.first(); t; t = todoList.next()) {
    kdDebug() << "  " << t->getSummary() << endl;

    if (t->getRelatedTo()) {
      kdDebug() << "      (related to " << t->getRelatedTo()->getSummary() << ")" << endl;
    }

    QList<Event> l = t->getRelations();
    Event *c;
    for(c=l.first();c;c=l.next()) {
      kdDebug() << "    - relation: " << c->getSummary() << endl;
    }
  }
*/

  // Put for each Event a KOProjectViewItem in the list view. Don't rely on a
  // specific order of events. That means that we have to generate parent items
  // recursively for proper hierarchical display of Todos.
  mTodoMap.clear();
  Todo *todo;
  for(todo = todoList.first(); todo; todo = todoList.next()) {
    if (!mTodoMap.contains(todo)) {
      insertTodoItem(todo);
    }
  }
}

QMap<Todo *,KGanttItem *>::ConstIterator
    KOProjectView::insertTodoItem(Todo *todo)
{
//  kdDebug() << "KOProjectView::insertTodoItem(): " << todo->getSummary() << endl;
  Todo *relatedTodo = dynamic_cast<Todo *>(todo->relatedTo());
  if (relatedTodo) {
//    kdDebug() << "  has Related" << endl;
    QMap<Todo *,KGanttItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
//      kdDebug() << "    related not yet in list" << endl;
      itemIterator = insertTodoItem (relatedTodo);
    }
    KGanttItem *task = createTask(*itemIterator,todo);
    return mTodoMap.insert(todo,task);
  } else {
//    kdDebug() << "  no Related" << endl;
    KGanttItem *task = createTask(mMainTask,todo);
    return mTodoMap.insert(todo,task);
  }
}

KGanttItem *KOProjectView::createTask(KGanttItem *parent,Todo *todo)
{
  QDateTime startDt;
  QDateTime endDt;

  if (todo->hasStartDate() && !todo->hasDueDate()) {
    // start date but no due date
    startDt = todo->dtStart();
    endDt = QDateTime::currentDateTime();
  } else if (!todo->hasStartDate() && todo->hasDueDate()) {
    // due date but no start date
    startDt = todo->dtDue();
    endDt = todo->dtDue();
  } else if (!todo->hasStartDate() || !todo->hasDueDate()) {
    startDt = QDateTime::currentDateTime();
    endDt = QDateTime::currentDateTime();
  } else {
    startDt = todo->dtStart();
    endDt = todo->dtDue();
  }

  KGanttItem *task = new KOProjectViewItem(todo,parent,todo->summary(),startDt,
                                       endDt);
  connect(task,SIGNAL(changed(KGanttItem*, KGanttItem::Change)),
          SLOT(taskChanged(KGanttItem*,KGanttItem::Change)));
  if (todo->relations().count() > 0) {
    task->setBrush(QBrush(QColor(240,240,240), QBrush::Dense4Pattern));
  }

  return task;
}

void KOProjectView::updateConfig()
{
  // TODO: to be implemented.
}

QList<Incidence> KOProjectView::getSelected()
{
  QList<Incidence> selected;

/*
  KOProjectViewItem *item = (KOProjectViewItem *)(mTodoListView->selectedItem());
  if (item) selected.append(item->event());
*/

  return selected;
}

void KOProjectView::changeEventDisplay(Event *, int)
{
  updateView();
}

void KOProjectView::selectDates(const QDateList)
{
  updateView();
}
 
void KOProjectView::selectEvents(QList<Event>)
{
  kdDebug() << "KOProjectView::selectEvents(): not yet implemented" << endl;
}

void KOProjectView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                              const QDate &td)
{
  calPrinter->preview(CalPrinter::Todolist, fd, td);
}

#if 0
void KOProjectView::editItem(QListViewItem *item)
{
  emit editEventSignal(((KOProjectViewItem *)item)->event());
}

void KOProjectView::showItem(QListViewItem *item)
{
  emit showTodoSignal(((KOProjectViewItem *)item)->event());
}

void KOProjectView::popupMenu(QListViewItem *item,const QPoint &,int)
{
  mActiveItem = (KOProjectViewItem *)item;
  if (item) mItemPopupMenu->popup(QCursor::pos());
  else mPopupMenu->popup(QCursor::pos());
}

void KOProjectView::newTodo()
{
  emit newTodoSignal();
}

void KOProjectView::newSubTodo()
{
  if (mActiveItem) {
    emit newSubTodoSignal(mActiveItem->event());
  }
}

void KOProjectView::editTodo()
{
  if (mActiveItem) {
    emit editEventSignal(mActiveItem->event());
  }
}

void KOProjectView::showTodo()
{
  if (mActiveItem) {
    emit showTodoSignal(mActiveItem->event());
  }
}

void KOProjectView::deleteTodo()
{
  if (mActiveItem) {
    if (mActiveItem->childCount()) {
      KMessageBox::sorry(this,i18n("Cannot delete To-Do which has children."),
                         i18n("Delete To-Do"));
    } else {
      emit deleteEventSignal(mActiveItem->event());
    }
  }
}

void KOProjectView::purgeCompleted()
{
  int result = KMessageBox::warningContinueCancel(this,
      i18n("Delete all completed todos?"),i18n("Purge Todos"),i18n("Purge"));

  if (result == KMessageBox::Continue) {
    QList<Todo> todoCal = mCalendar->getTodoList();

    Todo *aTodo;
    for (aTodo = todoCal.first(); aTodo; aTodo = todoCal.next()) {
    if (aTodo->isCompleted())
      mCalendar->deleteTodo(aTodo);
    }
    updateView();
  }
}

void KOProjectView::itemClicked(QListViewItem *item)
{
  if (!item) return;

  KOProjectViewItem *todoItem = (KOProjectViewItem *)item;
  int completed = todoItem->event()->isCompleted();  // Completed or not?
  
  if (todoItem->isOn()) {
    if (!completed) {
      todoItem->event()->setCompleted(true);
    }
  } else {
    if (completed) {
      todoItem->event()->setCompleted(false);
    }
  }
}
#endif

void KOProjectView::showModeMenu()
{
  mGantt->menu()->popup(QCursor::pos());  
}

void KOProjectView::taskChanged(KGanttItem *task,KGanttItem::Change change)
{
  if (task == mMainTask) return;

  KOProjectViewItem *item = (KOProjectViewItem *)task;

  if (change == KGanttItem::StartChanged) {
    item->event()->setDtStart(task->getStart());
  } else if (change == KGanttItem::EndChanged) {
    item->event()->setDtDue(task->getEnd());
  }
}

void KOProjectView::zoomIn()
{
}

void KOProjectView::zoomOut()
{
}
