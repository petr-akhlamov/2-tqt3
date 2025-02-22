/**********************************************************************
** Copyright (C) 2005-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of TQt Designer.
**
** This file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the files LICENSE.GPL2
** and LICENSE.GPL3 included in the packaging of this file.
** Alternatively you may (at your option) use any later version
** of the GNU General Public License if such license has been
** publicly approved by Trolltech ASA (or its successors, if any)
** and the KDE Free TQt Foundation.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/.
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** Licensees holding valid TQt Commercial licenses may use this file in
** accordance with the TQt Commercial License Agreement provided with
** the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#include <ntqvariant.h>  // HP-UX compiler needs this here

#include "sizehandle.h"
#include "formwindow.h"
#include "widgetfactory.h"

#include <ntqwidget.h>
#include <ntqapplication.h>
#include <ntqlabel.h>

SizeHandle::SizeHandle( FormWindow *parent, Direction d, WidgetSelection *s )
    : TQWidget( parent )
{
    active = TRUE;
    setBackgroundMode( active ? PaletteText : PaletteDark );
    setFixedSize( 6, 6 );
    widget = 0;
    dir =d ;
    setMouseTracking( FALSE );
    formWindow = parent;
    sel = s;
    updateCursor();
}

void SizeHandle::updateCursor()
{
    if ( !active ) {
	setCursor( arrowCursor );
	return;
    }

    switch ( dir ) {
    case LeftTop:
	setCursor( sizeFDiagCursor );
	break;
    case Top:
	setCursor( sizeVerCursor );
	break;
    case RightTop:
	setCursor( sizeBDiagCursor );
	break;
    case Right:
	setCursor( sizeHorCursor );
	break;
    case RightBottom:
	setCursor( sizeFDiagCursor );
	break;
    case Bottom:
	setCursor( sizeVerCursor );
	break;
    case LeftBottom:
	setCursor( sizeBDiagCursor );
	break;
    case Left:
	setCursor( sizeHorCursor );
	break;
    }
}

void SizeHandle::setActive( bool a )
{
    active = a;
    setBackgroundMode( active ? PaletteText : PaletteDark );
    updateCursor();
}

void SizeHandle::setWidget( TQWidget *w )
{
    widget = w;
}

void SizeHandle::paintEvent( TQPaintEvent * )
{
    if ( ( (FormWindow*)parentWidget() )->currentWidget() != widget )
	return;
    TQPainter p( this );
    p.setPen( blue );
    p.drawRect( 0, 0, width(), height() );
}

void SizeHandle::mousePressEvent( TQMouseEvent *e )
{
    if ( !widget || e->button() != LeftButton || !active )
	return;
    oldPressPos = e->pos();
    geom = origGeom = TQRect( widget->pos(), widget->size() );
}

void SizeHandle::mouseMoveEvent( TQMouseEvent *e )
{
    if ( !widget || ( e->state() & LeftButton ) != LeftButton || !active )
	return;
    TQPoint rp = mapFromGlobal( e->globalPos() );
    TQPoint d = oldPressPos - rp;
    oldPressPos = rp;
    TQPoint checkPos = widget->parentWidget()->mapFromGlobal( e->globalPos() );
    TQRect pr = widget->parentWidget()->rect();

    // ##### move code around a bit to reduce duplicated code here
    switch ( dir ) {
    case LeftTop: {
	if ( checkPos.x() > pr.width() - 2 * width() || checkPos.y() > pr.height() - 2 * height() )
	    return;
	int w = geom.width() + d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	int h = geom.height() + d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	int dx = widget->width() - w;
	int dy = widget->height() - h;
	trySetGeometry( widget, widget->x() + dx, widget->y() + dy, w, h );
    } break;
    case Top: {
	if ( checkPos.y() > pr.height() - 2 * height() )
	    return;
	int h = geom.height() + d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	int dy = widget->height() - h;
	trySetGeometry( widget, widget->x(), widget->y() + dy, widget->width(), h );
    } break;
    case RightTop: {
	if ( checkPos.x() < 2 * width() || checkPos.y() > pr.height() - 2 * height() )
	    return;
	int h = geom.height() + d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	int dy = widget->height() - h;
	int w = geom.width() - d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	trySetGeometry( widget, widget->x(), widget->y() + dy, w, h );
    } break;
    case Right: {
	if ( checkPos.x() < 2 * width() )
	    return;
	int w = geom.width() - d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	tryResize( widget, w, widget->height() );
    } break;
    case RightBottom: {
	if ( checkPos.x() < 2 * width() || checkPos.y() < 2 * height() )
	    return;
	int w = geom.width() - d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	int h = geom.height() - d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	tryResize( widget, w, h );
    } break;
    case Bottom: {
	if ( checkPos.y() < 2 * height() )
	    return;
	int h = geom.height() - d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	tryResize( widget, widget->width(), h );
    } break;
    case LeftBottom: {
	if ( checkPos.x() > pr.width() - 2 * width() || checkPos.y() < 2 * height() )
	    return;
	int w = geom.width() + d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	int dx = widget->width() - w;
	int h = geom.height() - d.y();
	geom.setHeight( h );
	h = ( h / formWindow->grid().y() ) * formWindow->grid().y();
	trySetGeometry( widget, widget->x() + dx, widget->y(), w, h );
    } break;
    case Left: {
	if ( checkPos.x() > pr.width() - 2 * width() )
	    return;
	int w = geom.width() + d.x();
	geom.setWidth( w );
	w = ( w / formWindow->grid().x() ) * formWindow->grid().x();
	int dx = widget->width() - w;
	trySetGeometry( widget, widget->x() + dx, widget->y(), w, widget->height() );
    } break;
    }

    TQPoint p = pos();
    sel->updateGeometry();
    oldPressPos += ( p - pos() );

    formWindow->sizePreview()->setText( tr( "%1/%2" ).arg( widget->width() ).arg( widget->height() ) );
    formWindow->sizePreview()->adjustSize();
    TQRect lg( formWindow->mapFromGlobal( e->globalPos() ) + TQPoint( 16, 16 ),
	      formWindow->sizePreview()->size() );
    formWindow->checkPreviewGeometry( lg );
    formWindow->sizePreview()->setGeometry( lg );
    formWindow->sizePreview()->show();
    formWindow->sizePreview()->raise();
    if ( WidgetFactory::layoutType( widget ) != WidgetFactory::NoLayout )
	formWindow->updateChildSelections( widget );
}

void SizeHandle::mouseReleaseEvent( TQMouseEvent *e )
{
    if ( e->button() != LeftButton || !active )
	return;

    formWindow->sizePreview()->hide();
    if ( geom != widget->geometry() )
	formWindow->commandHistory()->addCommand( new ResizeCommand( tr( "Resize" ),
								     formWindow,
								     widget, origGeom,
								     widget->geometry() ) );
    formWindow->emitUpdateProperties( widget );
}

void SizeHandle::trySetGeometry( TQWidget *w, int x, int y, int width, int height )
{
    int minw = TQMAX( w->minimumSizeHint().width(), w->minimumSize().width() );
    minw = TQMAX( minw, 2 * formWindow->grid().x() );
    int minh = TQMAX( w->minimumSizeHint().height(), w->minimumSize().height() );
    minh = TQMAX( minh, 2 * formWindow->grid().y() );
    if ( TQMAX( minw, width ) > w->maximumWidth() ||
	 TQMAX( minh, height ) > w->maximumHeight() )
	return;
    if ( width < minw && x != w->x() )
	x -= minw - width;
    if ( height < minh && y != w->y() )
	y -= minh - height;
    w->setGeometry( x, y, TQMAX( minw, width ), TQMAX( minh, height ) );
}

void SizeHandle::tryResize( TQWidget *w, int width, int height )
{
    int minw = TQMAX( w->minimumSizeHint().width(), w->minimumSize().width() );
    minw = TQMAX( minw, 16 );
    int minh = TQMAX( w->minimumSizeHint().height(), w->minimumSize().height() );
    minh = TQMAX( minh, 16 );
    w->resize( TQMAX( minw, width ), TQMAX( minh, height ) );
}

// ------------------------------------------------------------------------

WidgetSelection::WidgetSelection( FormWindow *parent, TQPtrDict<WidgetSelection> *selDict )
    : selectionDict( selDict )
{
    formWindow = parent;
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	handles.insert( i, new SizeHandle( formWindow, (SizeHandle::Direction)i, this ) );
    }
    hide();
}

void WidgetSelection::setWidget( TQWidget *w, bool updateDict )
{
    if ( !w ) {
	hide();
	if ( updateDict )
	    selectionDict->remove( wid );
	wid = 0;
	return;
    }

    wid = w;
    bool active = !wid->parentWidget() || WidgetFactory::layoutType( wid->parentWidget() ) == WidgetFactory::NoLayout;
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *h = handles[ i ];
	if ( h ) {
	    h->setWidget( wid );
	    h->setActive( active );
	}
    }
    updateGeometry();
    show();
    if ( updateDict )
	selectionDict->insert( w, this );
}

bool WidgetSelection::isUsed() const
{
    return wid != 0;
}

void WidgetSelection::updateGeometry()
{
    if ( !wid || !wid->parentWidget() )
	return;

    TQPoint p = wid->parentWidget()->mapToGlobal( wid->pos() );
    p = formWindow->mapFromGlobal( p );
    TQRect r( p, wid->size() );

    int w = 6;
    int h = 6;

    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *hndl = handles[ i ];
	if ( !hndl )
	    continue;
	switch ( i ) {
	case SizeHandle::LeftTop:
	    hndl->move( r.x() - w / 2, r.y() - h / 2 );
	    break;
	case SizeHandle::Top:
	    hndl->move( r.x() + r.width() / 2 - w / 2, r.y() - h / 2 );
	    break;
	case SizeHandle::RightTop:
	    hndl->move( r.x() + r.width() - w / 2, r.y() - h / 2 );
	    break;
	case SizeHandle::Right:
	    hndl->move( r.x() + r.width() - w / 2, r.y() + r.height() / 2 - h / 2 );
	    break;
	case SizeHandle::RightBottom:
	    hndl->move( r.x() + r.width() - w / 2, r.y() + r.height() - h / 2 );
	    break;
	case SizeHandle::Bottom:
	    hndl->move( r.x() + r.width() / 2 - w / 2, r.y() + r.height() - h / 2 );
	    break;
	case SizeHandle::LeftBottom:
	    hndl->move( r.x() - w / 2, r.y() + r.height() - h / 2 );
	    break;
	case SizeHandle::Left:
	    hndl->move( r.x() - w / 2, r.y() + r.height() / 2 - h / 2 );
	    break;
	default:
	    break;
	}
    }
}

void WidgetSelection::hide()
{
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *h = handles[ i ];
	if ( h )
	    h->hide();
    }
}

void WidgetSelection::show()
{
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *h = handles[ i ];
	if ( h ) {
	    h->show();
	    h->raise();
	}
    }
}

void WidgetSelection::update()
{
    for ( int i = SizeHandle::LeftTop; i <= SizeHandle::Left; ++i ) {
	SizeHandle *h = handles[ i ];
	if ( h )
	    h->update();
    }
}

TQWidget *WidgetSelection::widget() const
{
    return wid;
}
