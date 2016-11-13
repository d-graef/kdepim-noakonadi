/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "knarticle.h"

#include "knhdrviewitem.h"
#include "kngroup.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "settings.h"
#include "utilities.h"
#include "utils/locale.h"

#include <klocale.h>
#include <kcodecs.h>
#include <kmimetype.h>

#include <QByteArray>


using namespace KNode::Utilities;
using namespace KMime;


KNArticle::KNArticle(KNArticleCollection *c) : i_d(-1), c_ol(c), i_tem(0)
{
}


KNArticle::~KNArticle()
{
  delete i_tem;
}


void KNArticle::clear()
{
  f_lags.clear();
}


void KNArticle::setListItem(KNHdrViewItem *it)
{
  i_tem=it;
  if(i_tem) i_tem->art=this;
}


void KNArticle::setLocked(bool b)
{
  f_lags.set(0, b);
  if(c_ol) {  // local articles may have c_ol==0 !
    if(b)
      c_ol->articleLocked();
    else
      c_ol->articleUnlocked();
  }
}


QByteArray KNArticle::assembleHeaders()
{
  // filter out internal headers
  for ( Headers::Base::List::Iterator it = h_eaders.begin(); it != h_eaders.end(); ) {
    if ( (*it)->isXHeader() && ( strncasecmp( (*it)->type(), "X-KNode", 7 ) == 0 ) ) {
      delete *it;
      it = h_eaders.erase( it );
    }
    else
      ++it;
  }

  return KMime::NewsArticle::assembleHeaders();
}


//=========================================================================================


KNRemoteArticle::KNRemoteArticle(KNGroup *g)
 : KNArticle(g), a_rticleNumber(-1), i_dRef(-1), d_ref(0), t_hrLevel(0), s_core(0),
   c_olor(knGlobals.settings()->unreadThreadColor()),
   u_nreadFups(0), n_ewFups(0), s_ubThreadChangeDate(0)
{
  setDefaultCharset( Locale::defaultCharset( g ) );

  // Remote article are read-only
  setFrozen( true );
}


KNRemoteArticle::~KNRemoteArticle()
{}


void KNRemoteArticle::parse()
{
  KNArticle::parse();
  QByteArray raw;

  raw = KMime::extractHeader( head(), from()->type() );
  if ( !raw.isEmpty() ) {
    QByteArray f;
    Locale::encodeTo7Bit( raw, defaultCharset(), f );
    from()->from7BitString( f );
  }

  raw = KMime::extractHeader( head(), subject()->type() );
  if( !raw.isEmpty() ) {
    QByteArray subjectStr;
    Locale::encodeTo7Bit( raw, defaultCharset(), subjectStr );
    subject()->from7BitString( subjectStr );
  }
}


void KNRemoteArticle::initListItem()
{
  if(!i_tem) return;

  KNHdrViewItem *hdrVItem = static_cast<KNHdrViewItem*>(i_tem);       //  cast: overloaded text() function from KNHdrViewItem is called

  KMime::Types::Mailbox mbox;
  if ( !from()->isEmpty() ) {
    mbox = from()->mailboxes().first();

  }
  if ( mbox.hasName() )
    i_tem->setText( 1, mbox.name() );
  else
    i_tem->setText( 1, QString::fromLatin1( mbox.address() ) );

  QString dateTimeStringS(hdrVItem->text(4));
  QString dateTimeStringD;
  QString tempString;

  tempString = dateTimeStringS.mid(6, 4);  // year
  dateTimeStringD.append(tempString);
  dateTimeStringD.append("-");
  tempString = dateTimeStringS.mid(3, 2);  // month
  dateTimeStringD.append(tempString);
  dateTimeStringD.append("-");
  tempString = dateTimeStringS.mid(0, 2);  // day
  dateTimeStringD.append(tempString);
  dateTimeStringD.append("T");
  tempString = dateTimeStringS.mid(11, 2);  // hour
  dateTimeStringD.append(tempString);
  dateTimeStringD.append(":");
  tempString = dateTimeStringS.mid(14, 2);  // minute
  dateTimeStringD.append(tempString);
  dateTimeStringD.append(":");
  dateTimeStringD.append("00");             // second

  QVariant dateTime(dateTimeStringD);
  bool convert_rc = dateTime.convert(QVariant::DateTime);

  i_tem->setText(0, hdrVItem->text(0));     //   subject
  i_tem->setText(2, hdrVItem->text(2));     //
  i_tem->setText(3, hdrVItem->text(3));     //
  i_tem->setData(4, Qt::DisplayRole, dateTime);     //    else sorting as string

  updateListItem();
}


void KNRemoteArticle::updateListItem()
{

  if(!i_tem) return;

  KNode::Appearance *app=knGlobals.configManager()->appearance();

  QIcon greyBallChkd(app->icon(KNode::Appearance::greyBallChkd));
  QIcon greyBall(app->icon(KNode::Appearance::greyBall));
  QIcon redBallChkd(app->icon(KNode::Appearance::redBallChkd));
  QIcon redBall(app->icon(KNode::Appearance::redBall));
  QIcon newFups(app->icon(KNode::Appearance::newFups));
  QIcon null(app->icon(KNode::Appearance::null));
  QIcon eyes(app->icon(KNode::Appearance::eyes));
  QIcon ignore(app->icon(KNode::Appearance::ignore));

  if(isRead()) {
    if(hasContent())
      i_tem->setIcon(0, greyBallChkd);
    else
      i_tem->setIcon(0, greyBall);
  }
  else {
    if(hasContent())
      i_tem->setIcon(0, redBallChkd);
    else
      i_tem->setIcon(0, redBall);
  }

  if(hasNewFollowUps())
    i_tem->setIcon(1, newFups);
  else
    i_tem->setIcon(1, null);

  if(isWatched())
    i_tem->setIcon(2, eyes);
  else {
    if(isIgnored())
      i_tem->setIcon(2, ignore);
    else
      i_tem->setIcon(2, null);
  }

//  i_tem->setExpandable( (threadMode() && hasVisibleFollowUps()) );
//  i_tem->repaint(); //force repaint
}


void KNRemoteArticle::thread(KNRemoteArticle::List &l)
{
  KNRemoteArticle *tmp=0, *ref=this;
  KNGroup *g=static_cast<KNGroup*>(c_ol);
  int idRef=i_dRef, topID=-1;

  while(idRef!=0) {
    ref=g->byId(idRef);
    if(!ref)
      return; // sh#t !!
    idRef=ref->idRef();
  }

  topID=ref->id();
  l.append(ref);

  for(int i=0; i<g->length(); i++) {
    tmp=g->at(i);
    if(tmp->idRef()!=0) {
      idRef=tmp->idRef();
      while(idRef!=0) {
        ref=g->byId(idRef);
        idRef=ref->idRef();
      }
      if(ref->id()==topID)
        l.append(tmp);
    }
  }
}


void KNRemoteArticle::setForceDefaultCharset(bool b)
{
  if (!b) { // restore default
    KNGroup *g=static_cast<KNGroup*>(c_ol);
    setDefaultCharset( Locale::defaultCharset( g ) );
  }
  KNArticle::setForceDefaultCharset( b );
  initListItem();
}


void KNRemoteArticle::propagateThreadChangedDate()
{
  KNRemoteArticle *ref=this;
  KNGroup *g=static_cast<KNGroup*>(c_ol);
  int idRef=i_dRef;

  while (idRef!=0) {
    ref=g->byId(idRef);
    if(!ref)
      return; // sh#t !!
    idRef=ref->idRef();
  }

  if (date()->dateTime() > ref->date()->dateTime()) {
    ref->setSubThreadChangeDate(date()->dateTime().toTime_t());
  }
}


//=========================================================================================


KNLocalArticle::KNLocalArticle(KNArticleCollection *c)
  : KNArticle(c), s_Offset(0), e_Offset(0), s_erverId(-1)
{
  setDefaultCharset( Locale::defaultCharset() );
}


KNLocalArticle::~KNLocalArticle()
{}


void KNLocalArticle::updateListItem()
{
  if(!i_tem)
    return;

  QString tmp;
  int idx=0;
  KNode::Appearance *app=knGlobals.configManager()->appearance();

  QIcon savedRemote(app->icon(KNode::Appearance::savedRemote));    //  @dg
  QIcon canceledPosting(app->icon(KNode::Appearance::canceledPosting));
  QIcon posting(app->icon(KNode::Appearance::posting));
  QIcon mail(app->icon(KNode::Appearance::mail));

  if(isSavedRemoteArticle()) {
    i_tem->setIcon(0, savedRemote);   //    @dg
    Headers::Newsgroups *hdrNewsgroup = newsgroups( false );
    if ( hdrNewsgroup && !hdrNewsgroup->isEmpty() ) {
      tmp = hdrNewsgroup->asUnicodeString();
    } else {
      Headers::To *hdrTo = to( false );
      if ( hdrTo && !hdrTo->isEmpty() ) {
        tmp = hdrTo->asUnicodeString();
      }
    }
  } else {
    if(doPost()) {
      tmp += newsgroups()->asUnicodeString();
      if(canceled())
        i_tem->setIcon(idx++, canceledPosting);
      else
        i_tem->setIcon(idx++, posting);
    }

    if(doMail()) {
      i_tem->setIcon(idx++, mail);         //     @dg
      if(doPost())
        tmp+=" / ";
      tmp += to()->asUnicodeString();
    }
  }

  i_tem->setText(1, tmp);
}


void KNLocalArticle::setForceDefaultCharset( bool b )
{
  if (!b)  // restore default
    setDefaultCharset( Locale::defaultCharset() );
  KNArticle::setForceDefaultCharset( b );
  updateListItem();
}


//=========================================================================================


KNAttachment::KNAttachment(Content *c)
  : c_ontent(c), l_oadHelper(0), f_ile(0), i_sAttached(true)
{
  Headers::ContentType  *t=c->contentType();
  Headers::ContentTransferEncoding   *e=c->contentTransferEncoding();
  Headers::ContentDescription *d=c->contentDescription(false);

  n_ame=t->name();

  if(d)
    d_escription=d->asUnicodeString();


  setMimeType(t->mimeType());

  if(e->encoding()==Headers::CEuuenc) {
    setCte( Headers::CEbase64 );
    updateContentInfo();
  }
  else
    e_ncoding.setEncoding( e->encoding() );


  h_asChanged=false; // has been set to "true" in setMimeType()
}


KNAttachment::KNAttachment(KNLoadHelper *helper)
  : c_ontent(0), l_oadHelper(helper), f_ile(helper->getFile()), i_sAttached(false), h_asChanged(true)
{
  setMimeType(KMimeType::findByPath(f_ile->fileName())->name());
  n_ame=helper->getURL().fileName();
}


KNAttachment::~KNAttachment()
{
  if(!i_sAttached && c_ontent)
    delete c_ontent;
  delete l_oadHelper;
}


void KNAttachment::setMimeType(const QString &s)
{
  mMimeType = s;
  h_asChanged=true;

  if ( !mMimeType.contains( "text/", Qt::CaseInsensitive ) ) {
    f_b64=true;
    e_ncoding.setEncoding(Headers::CEbase64);
  }
  else {
    f_b64=false;
    if ( knGlobals.settings()->allow8BitBody() )
      setCte(Headers::CE8Bit);
    else
      setCte(Headers::CEquPr);
  }
}


QString KNAttachment::contentSize() const
{
  QString ret;
  int s=0;

  if(c_ontent && c_ontent->hasContent())
    s=c_ontent->size();
  else {
    if (f_ile)
      s=f_ile->size();
  }

  if(s > 1023) {
    s=s/1024;
    ret.setNum(s);
    ret+=" kB";
  }
  else {
    ret.setNum(s);
    ret+=" Bytes";
  }

  return ret;
}


void KNAttachment::updateContentInfo()
{
  if(!h_asChanged || !c_ontent)
    return;

  //Content-Type
  Headers::ContentType *t=c_ontent->contentType();
  t->setMimeType( mMimeType.toLatin1() );
  t->setName(n_ame, "UTF-8");
  t->setCategory(Headers::CCmixedPart);

  //Content-Description
  if(d_escription.isEmpty())
    c_ontent->removeHeader("Content-Description");
  else
    c_ontent->contentDescription()->fromUnicodeString(d_escription, "UTF-8");

  //Content-Disposition
  Headers::ContentDisposition *d=c_ontent->contentDisposition();
  d->setDisposition(Headers::CDattachment);
  d->setFilename(n_ame);

  //Content-Transfer-Encoding
  if(i_sAttached)
    c_ontent->changeEncoding(e_ncoding.encoding());
  else
    c_ontent->contentTransferEncoding()->setEncoding(e_ncoding.encoding());

  c_ontent->assemble();

  h_asChanged=false;
}



void KNAttachment::attach(Content *c)
{
  if(i_sAttached || !f_ile)
    return;

  c_ontent=new Content();
  updateContentInfo();
  Headers::ContentType *type=c_ontent->contentType();
  Headers::ContentTransferEncoding *e=c_ontent->contentTransferEncoding();
  QByteArray data;

  data = f_ile->readAll();

  if ( data.size() < f_ile->size() && f_ile->error() != QFile::NoError ) {
    KNHelper::displayExternalFileError();
    delete c_ontent;
    c_ontent=0;
  } else {
    if (e_ncoding.encoding()==Headers::CEbase64 || !type->isText()) { //encode base64
      c_ontent->setBody( KCodecs::base64Encode(data, true) + '\n' );
      //      c_ontent->b_ody += '\n';
      e->setEncoding(Headers::CEbase64);
      e->setDecoded(false);
    } else  {
      c_ontent->setBody( data + '\n' );
      //      c_ontent->b_ody += '\n';
      e->setDecoded(true);
    }
  }

  if(c_ontent) {
    c->addContent(c_ontent);
    i_sAttached=true;
  }
}


void KNAttachment::detach(Content *c)
{
  if(i_sAttached) {
    c->removeContent(c_ontent, false);
    i_sAttached=false;
  }
}
