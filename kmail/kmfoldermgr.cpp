// kmfoldermgr.cpp

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <qdir.h>
#include <assert.h>
#include "kmfiltermgr.h"
#include "kmfoldermgr.h"
#include "kmfolder.h"
#include "kmglobal.h"
#include <kapp.h>
#include <klocale.h>


//-----------------------------------------------------------------------------
KMFolderMgr::KMFolderMgr(const QCString& aBasePath):
  KMFolderMgrInherited(), mDir()
{
  initMetaObject();
  setBasePath(aBasePath);
}


//-----------------------------------------------------------------------------
KMFolderMgr::~KMFolderMgr()
{
  mBasePath = 0;
}


//-----------------------------------------------------------------------------
void KMFolderMgr::setBasePath(const QCString& aBasePath)
{
  QDir dir;

  assert(!aBasePath.isEmpty());

  if (aBasePath[0] == '~')
  {
    mBasePath = QDir::homeDirPath();
    mBasePath.append("/");
    mBasePath.append(((const char *)aBasePath)+1);
  }
  else
  {
    mBasePath = "";
    mBasePath.append(aBasePath);
  }


  dir.setPath(mBasePath);
  if (!dir.exists())
  {
    KMFolder fld(&mDir);

    warning("Directory\n"+mBasePath+"\ndoes not exist.\n\n"
	    "KMail will create it now.");
    // dir.mkdir(mBasePath, TRUE);
    mkdir(mBasePath.data(), 0700);
    mDir.setPath(mBasePath);

    fld.setName("inbox");
    fld.create();
    fld.close();

    fld.setName("outbox");
    fld.create();
    fld.close();

    fld.setName("sent-mail");
    fld.create();
    fld.close();

    fld.setName("trash");
    fld.create();
    fld.close();
  }

  mDir.setPath(mBasePath);
  mDir.reload();
  emit changed();
}


//-----------------------------------------------------------------------------
KMFolder* KMFolderMgr::createFolder(const char* fName, bool sysFldr)
{
  KMFolder* fld;

  fld = mDir.createFolder(fName, sysFldr);
  if (fld) emit changed();

  return fld;
}


//-----------------------------------------------------------------------------
bool KMFolderMgr::createDirectory(const char* fName)
{
  bool rc = mDir.createDirectory(fName);
  if (rc) emit changed();
  return rc;
}


//-----------------------------------------------------------------------------
KMFolder* KMFolderMgr::find(const char* folderName, bool foldersOnly)
{
  KMFolderNode* node;

  for (node=mDir.first(); node; node=mDir.next())
  {
    if (node->isDir() && foldersOnly) continue;
    if (node->name()==folderName) return (KMFolder*)node;
  }
  return NULL;
}


//-----------------------------------------------------------------------------
KMFolder* KMFolderMgr::findOrCreate(const char* aFolderName)
{
  KMFolder* folder = find(aFolderName);

  if (!folder)
  {
    warning(i18n("Creating missing folder\n`%s'"), aFolderName);

    folder = createFolder(aFolderName, TRUE);
    if (!folder) fatal(i18n("Cannot create folder `%s'\nin %s"),
		       aFolderName, (const char*)mBasePath);
  }
  return folder;
}


//-----------------------------------------------------------------------------
void KMFolderMgr::folderListRecursive(KMCStringList* aList, KMFolderDir* aDir,
				      const QCString& aPath)
{
  KMFolderNode* node;
  QCString path;

  assert(aList!=NULL);
  assert(aDir!=NULL);

  for (node=aDir->first(); node; node=aDir->next())
  {
    if (aPath.isEmpty()) path = node->name();
    else path = aPath + '/' + node->name();
    if (!node->isDir()) aList->append(path);
  }
}


//-----------------------------------------------------------------------------
KMCStringList& KMFolderMgr::folderList()
{
  static KMCStringList lst;

  lst.clear();
  folderListRecursive(&lst, &mDir, 0);
  return lst;
}


//-----------------------------------------------------------------------------
void KMFolderMgr::remove(KMFolder* aFolder)
{
  assert(aFolder != NULL);

  aFolder->remove();
  mDir.remove(aFolder);
  //mDir.reload();
  if (filterMgr) filterMgr->folderRemoved(aFolder,NULL);

  emit changed();
}


//-----------------------------------------------------------------------------
KMFolderRootDir& KMFolderMgr::dir(void)
{
  return mDir;
}


//-----------------------------------------------------------------------------
void KMFolderMgr::contentsChanged(void)
{
  emit changed();
}


//-----------------------------------------------------------------------------
void KMFolderMgr::reload(void)
{
}


//-----------------------------------------------------------------------------
#include "kmfoldermgr.moc"
