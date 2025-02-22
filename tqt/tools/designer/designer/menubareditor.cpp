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

#include <ntqaction.h>
#include <ntqapplication.h>
#include <ntqbitmap.h>
#include <ntqdragobject.h>
#include <ntqlineedit.h>
#include <ntqmainwindow.h>
#include <ntqpainter.h>
#include <ntqstyle.h>
#include "command.h"
#include "formwindow.h"
#include "menubareditor.h"
#include "popupmenueditor.h"

extern void find_accel( const TQString &txt, TQMap<TQChar, TQWidgetList > &accels, TQWidget *w );

// Drag Object Declaration -------------------------------------------

class MenuBarEditorItemPtrDrag : public TQStoredDrag
{
public:
    MenuBarEditorItemPtrDrag( MenuBarEditorItem * item,
			      TQWidget * parent = 0,
			      const char * name = 0 );
    ~MenuBarEditorItemPtrDrag() {};
    static bool canDecode( TQDragMoveEvent * e );
    static bool decode( TQDropEvent * e, MenuBarEditorItem ** i );
};

// Drag Object Implementation ---------------------------------------

MenuBarEditorItemPtrDrag::MenuBarEditorItemPtrDrag( MenuBarEditorItem * item,
						    TQWidget * parent,
						    const char * name )
    : TQStoredDrag( "qt/menubareditoritemptr", parent, name )
{
    TQByteArray data( sizeof( TQ_LONG ) );
    TQDataStream stream( data, IO_WriteOnly );
    stream << ( TQ_LONG ) item;
    setEncodedData( data );
}

bool MenuBarEditorItemPtrDrag::canDecode( TQDragMoveEvent * e )
{
    return e->provides( "qt/menubareditoritemptr" );
}

bool MenuBarEditorItemPtrDrag::decode( TQDropEvent * e, MenuBarEditorItem ** i )
{
    TQByteArray data = e->encodedData( "qt/menubareditoritemptr" );
    TQDataStream stream( data, IO_ReadOnly );

    if ( !data.size() )
	return FALSE;

    TQ_LONG p = 0;
    stream >> p;
    *i = ( MenuBarEditorItem *) p;

    return TRUE;
}

// MenuBarEditorItem ---------------------------------------------------

MenuBarEditorItem::MenuBarEditorItem( MenuBarEditor * bar, TQObject * parent, const char * name )
    : TQObject( parent, name ),
      menuBar( bar ),
      popupMenu( 0 ),
      visible( TRUE ),
      separator( FALSE ),
      removable( FALSE )
{ }

MenuBarEditorItem::MenuBarEditorItem( PopupMenuEditor * menu, MenuBarEditor * bar,
				      TQObject * parent, const char * name )
    : TQObject( parent, name ),
      menuBar( bar ),
      popupMenu( menu ),
      visible( TRUE ),
      separator( FALSE ),
      removable( TRUE )
{
    text = menu->name();
}

MenuBarEditorItem::MenuBarEditorItem( TQActionGroup * actionGroup, MenuBarEditor * bar,
				      TQObject * parent, const char * name )
    : TQObject( parent, name ),
      menuBar( bar ),
      popupMenu( 0 ),
      visible( TRUE ),
      separator( FALSE ),
      removable( TRUE )
{
    text = actionGroup->menuText();
    popupMenu = new PopupMenuEditor( menuBar->formWindow(), menuBar );
    popupMenu->insert( actionGroup );
}

MenuBarEditorItem::MenuBarEditorItem( MenuBarEditorItem * item, TQObject * parent, const char * name )
    : TQObject( parent, name ),
      menuBar( item->menuBar ),
      popupMenu( 0 ),
      text( item->text ),
      visible( item->visible ),
      separator( item->separator ),
      removable( item->removable )
{
    popupMenu = new PopupMenuEditor( menuBar->formWindow(), item->popupMenu, menuBar );
}

// MenuBarEditor --------------------------------------------------------

int MenuBarEditor::clipboardOperation = 0;
MenuBarEditorItem * MenuBarEditor::clipboardItem = 0;

MenuBarEditor::MenuBarEditor( FormWindow * fw, TQWidget * parent, const char * name )
    : TQMenuBar( parent, name ),
      formWnd( fw ),
      draggedItem( 0 ),
      currentIndex( 0 ),
      itemHeight( 0 ),
      separatorWidth( 32 ),
      hideWhenEmpty( TRUE ),
      hasSeparator( FALSE )
{
    setAcceptDrops( TRUE );
    setFocusPolicy( StrongFocus );

    addItem.setMenuText( tr("new menu") );
    addSeparator.setMenuText( tr("new separator") );

    lineEdit = new TQLineEdit( this, "menubar lineedit" );
    lineEdit->hide();
    lineEdit->setFrameStyle(TQFrame::Plain | TQFrame::NoFrame);
    lineEdit->polish();
    lineEdit->setBackgroundMode(PaletteButton);
    lineEdit->setBackgroundOrigin(ParentOrigin);
    lineEdit->installEventFilter( this );

    dropLine = new TQWidget( this, "menubar dropline", TQt::WStyle_NoBorder | WStyle_StaysOnTop );
    dropLine->setBackgroundColor( TQt::red );
    dropLine->hide();

    setMinimumHeight( fontMetrics().height() + 2 * borderSize() );
}

MenuBarEditor::~MenuBarEditor()
{
    itemList.setAutoDelete( TRUE );
}

FormWindow * MenuBarEditor::formWindow()
{
    return formWnd;
}

MenuBarEditorItem * MenuBarEditor::createItem( int index, bool addToCmdStack )
{
    MenuBarEditorItem * i =
	new MenuBarEditorItem( new PopupMenuEditor( formWnd, ( TQWidget * ) parent() ), this );
    if ( addToCmdStack ) {
	AddMenuCommand * cmd = new AddMenuCommand( "Add Menu", formWnd, this, i, index );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    } else {
	AddMenuCommand cmd( "Add Menu", formWnd, this, i, index );
	cmd.execute();
    }
    return i;
}

void MenuBarEditor::insertItem( MenuBarEditorItem * item, int index )
{
    item->menu()->parentMenu = this;

    if ( index != -1 )
	itemList.insert( index, item );
    else
	itemList.append( item );

    if ( hideWhenEmpty && itemList.count() == 1 )
	show(); // calls resizeInternals();
    else
	resizeInternals();

    if ( isVisible() )
	update();
}

void MenuBarEditor::insertItem( TQString text, PopupMenuEditor * menu, int index )
{
    MenuBarEditorItem * item = new MenuBarEditorItem( menu, this );
    if ( !text.isNull() )
	item->setMenuText( text );
    insertItem( item, index );
}

void MenuBarEditor::insertItem( TQString text, TQActionGroup * group, int index )
{
    MenuBarEditorItem * item = new MenuBarEditorItem( group, this );
    if ( !text.isNull() )
	item->setMenuText( text );
    insertItem( item, index );
}


void MenuBarEditor::insertSeparator( int index )
{
    if ( hasSeparator )
	return;

    MenuBarEditorItem * i = createItem( index );
    i->setSeparator( TRUE );
    i->setMenuText( "separator" );
    hasSeparator = TRUE;
}

void MenuBarEditor::removeItemAt( int index )
{
    removeItem( item( index ) );
}

void MenuBarEditor::removeItem( MenuBarEditorItem * item )
{
    if ( item &&
	 item->isRemovable() &&
	 itemList.removeRef( item ) ) {

	if ( item->isSeparator() )
	    hasSeparator = FALSE;

	if ( hideWhenEmpty && itemList.count() == 0 )
	    hide();
	else
	    resizeInternals();

	int n = count() + 1;
	if ( currentIndex >=  n )
	    currentIndex = n;

	if ( isVisible() )
	    update();
    }
}

int MenuBarEditor::findItem( MenuBarEditorItem * item )
{
    return itemList.findRef( item );
}

int MenuBarEditor::findItem( PopupMenuEditor * menu )
{
    MenuBarEditorItem * i = itemList.first();

    while ( i ) {
	if ( i->menu() == menu )
	    return itemList.at();
	i = itemList.next();
    }

    return -1;
}

int MenuBarEditor::findItem( TQPoint & pos )
{
    int x = borderSize();
    int dx = 0;
    int y = 0;
    int w = width();
    TQSize s;
    TQRect r;

    MenuBarEditorItem * i = itemList.first();

    while ( i ) {

	if ( i->isVisible() ) {

	    s = itemSize( i );
	    dx = s.width();

	    if ( x + dx > w && x > borderSize() ) {
		y += itemHeight;
		x = borderSize();
	    }

	    r = TQRect( x, y, s.width(), s.height() );

	    if ( r.contains( pos ) )
		return itemList.at();

	    addItemSizeToCoords( i, x, y, w );
	}

	i = itemList.next();
    }

    // check add item
    s = itemSize( &addItem );
    dx = s.width();

    if ( x + dx > w && x > borderSize() ) {
	y += itemHeight;
	x = borderSize();
    }

    r = TQRect( x, y, s.width(), s.height() );

    if ( r.contains( pos ) )
	return itemList.count();

    return itemList.count() + 1;
}

MenuBarEditorItem * MenuBarEditor::item( int index )
{
    if ( index == -1 )
	return itemList.at( currentIndex );

    int c = itemList.count();
    if ( index == c )
	return &addItem;
    else if ( index > c )
	return &addSeparator;

    return itemList.at( index );
}

int MenuBarEditor::count()
{
    return itemList.count();
}

int MenuBarEditor::current()
{
    return currentIndex;
}

void MenuBarEditor::cut( int index )
{
    if ( clipboardItem && clipboardOperation == Cut )
	delete clipboardItem;

    clipboardOperation = Cut;
    clipboardItem = itemList.at( index );

    if ( clipboardItem == &addItem || clipboardItem == &addSeparator ) {
	clipboardOperation = None;
	clipboardItem = 0;
	return; // do nothing
    }

    RemoveMenuCommand * cmd = new RemoveMenuCommand( "Cut Menu", formWnd, this, index );
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
}

void MenuBarEditor::copy( int index )
{
    if ( clipboardItem && clipboardOperation == Cut )
	delete clipboardItem;

    clipboardOperation = Copy;
    clipboardItem = itemList.at( index );

    if ( clipboardItem == &addItem || clipboardItem == &addSeparator ) {
	clipboardOperation = None;
	clipboardItem = 0;
    }
}

void MenuBarEditor::paste( int index )
{
    if ( clipboardItem && clipboardOperation ) {
	MenuBarEditorItem * i = new MenuBarEditorItem( clipboardItem );
	AddMenuCommand * cmd = new AddMenuCommand( "Paste Menu", formWnd, this, i, index );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

void MenuBarEditor::exchange( int a, int b )
{
    MenuBarEditorItem * ia = itemList.at( a );
    MenuBarEditorItem * ib = itemList.at( b );
    if ( !ia || !ib ||
	 ia == &addItem || ia == &addSeparator ||
	 ib == &addItem || ib == &addSeparator )
	return; // do nothing
    itemList.replace( b, ia );
    itemList.replace( a, ib );
}

void MenuBarEditor::showLineEdit( int index )
{
    if ( index == -1 )
	index = currentIndex;

    MenuBarEditorItem * i = 0;

    if ( (uint) index >= itemList.count() )
	i = &addItem;
    else
	i = itemList.at( index );

    if ( i && i->isSeparator() )
	return;

    // open edit field for item name
    lineEdit->setText( i->menuText() );
    lineEdit->selectAll();
    TQPoint pos = itemPos( index );
    lineEdit->move( pos.x() + borderSize(), pos.y() - ( borderSize() / 2 ) );
    lineEdit->resize( itemSize( i ) );
    lineEdit->show();
    lineEdit->setFocus();
}

void MenuBarEditor::showItem( int index )
{
    if ( index == -1 )
	index = currentIndex;

    if ( (uint)index < itemList.count() ) {
	MenuBarEditorItem * i = itemList.at( index );
	if ( i->isSeparator() || draggedItem )
	    return;
	PopupMenuEditor * m = i->menu();
	TQPoint pos = itemPos( index );
	m->move( pos.x(), pos.y() + itemHeight - 1 );
	m->raise();
	m->show();
	setFocus();
    }
}

void MenuBarEditor::hideItem( int index )
{
    if ( index == -1 )
	index = currentIndex;

    if ( (uint)index < itemList.count() ) {
	PopupMenuEditor * m = itemList.at( index )->menu();
	m->hideSubMenu();
	m->hide();
    }
}

void MenuBarEditor::focusItem( int index )
{
    if ( index == -1 )
	index = currentIndex;

    if ( (uint)index < itemList.count() ) {
	PopupMenuEditor * m = itemList.at( index )->menu();
	m->setFocus();
	m->update();
	update();
    }
}

void MenuBarEditor::deleteItem( int index )
{
    if ( index == -1 )
	index = currentIndex;

    if ( (uint)index < itemList.count() ) {
	RemoveMenuCommand * cmd = new RemoveMenuCommand( "Delete Menu",
							 formWnd,
							 this,
							 currentIndex );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
}

TQSize MenuBarEditor::sizeHint() const
{
    return TQSize( parentWidget()->width(), heightForWidth( parentWidget()->width() ) );
}

int MenuBarEditor::heightForWidth( int max_width ) const
{
    MenuBarEditor * that = ( MenuBarEditor * ) this;
    int x = borderSize();
    int y = 0;

    TQPainter p( this );
    that->itemHeight = that->itemSize( &(that->addItem) ).height();

    MenuBarEditorItem * i = that->itemList.first();
    while ( i ) {
	if ( i->isVisible() )
	    that->addItemSizeToCoords( i, x, y, max_width );
	i = that->itemList.next();
    }

    that->addItemSizeToCoords( &(that->addItem), x, y, max_width );
    that->addItemSizeToCoords( &(that->addSeparator), x, y, max_width );

    return y + itemHeight;
}

void MenuBarEditor::show()
{
    TQWidget::show();
    resizeInternals();

    TQResizeEvent e( parentWidget()->size(), parentWidget()->size() );
    TQApplication::sendEvent( parentWidget(), &e );
}

void MenuBarEditor::checkAccels( TQMap<TQChar, TQWidgetList > &accels )
{
    TQString t;
    MenuBarEditorItem * i = itemList.first();
    while ( i ) {
	t = i->menuText();
	find_accel( t, accels, this );
	// do not check the accelerators in the popup menus
	i = itemList.next();
    }
}

// public slots

void MenuBarEditor::cut()
{
    cut( currentIndex );
}

void MenuBarEditor::copy()
{
    copy( currentIndex );
}

void MenuBarEditor::paste()
{
    paste( currentIndex );
}

// protected

bool MenuBarEditor::eventFilter( TQObject * o, TQEvent * e )
{
    if ( o == lineEdit && e->type() == TQEvent::FocusOut ) {
	leaveEditMode();
	lineEdit->hide();
	update();
    } else if ( e->type() == TQEvent::LayoutHint ) {
	resize( sizeHint() );
     }
    return TQMenuBar::eventFilter( o, e );
}

void MenuBarEditor::paintEvent( TQPaintEvent * )
{
    TQPainter p( this );
    TQRect r = rect();
    style().drawPrimitive( TQStyle::PE_PanelMenuBar, &p,
			   r, colorGroup() );
    drawItems( p );
}

void MenuBarEditor::mousePressEvent( TQMouseEvent * e )
{
    mousePressPos = e->pos();
    hideItem();
    lineEdit->hide();
    currentIndex = findItem( mousePressPos );
    showItem();
    update();
    e->accept();
}

void MenuBarEditor::mouseDoubleClickEvent( TQMouseEvent * e )
{
    mousePressPos = e->pos();
    currentIndex = findItem( mousePressPos );
    lineEdit->hide();
    if ( currentIndex > (int)itemList.count() ) {
	insertSeparator();
	update();
    } else {
	showLineEdit();
    }
}

void MenuBarEditor::mouseMoveEvent( TQMouseEvent * e )
{
    if ( e->state() & TQt::LeftButton ) {
	if ( ( e->pos() - mousePressPos ).manhattanLength() > 3 ) {
	    bool itemCreated = FALSE;
	    bool isSeparator = FALSE;
	    draggedItem = item( findItem( mousePressPos ) );
	    if ( draggedItem == &addItem ) {
		draggedItem = createItem();
		itemCreated = TRUE;
	    } else if ( draggedItem == &addSeparator ) {
                if (hasSeparator) // we can only have one separator
                    return;
		draggedItem = createItem();
		draggedItem->setSeparator( TRUE );
		draggedItem->setMenuText( "separator" );
		isSeparator = TRUE;
		itemCreated = TRUE;
	    } else {
		isSeparator = draggedItem->isSeparator();
	    }

	    MenuBarEditorItemPtrDrag * d =
		new MenuBarEditorItemPtrDrag( draggedItem, this );
	    d->setPixmap( createTextPixmap( draggedItem->menuText() ) );
	    hideItem();
	    draggedItem->setVisible( FALSE );
	    update();

	    // If the item is dropped in the same list,
	    //  we will have two instances of the same pointer
	    // in the list.
	    itemList.find( draggedItem );
	    TQLNode * node = itemList.currentNode();
	    dropConfirmed = FALSE;
	    d->dragCopy(); // dragevents and stuff happens
	    if ( draggedItem ) { // item was not dropped
		if ( itemCreated ) {
		    removeItem( draggedItem );
		} else {
		    hideItem();
		    draggedItem->setVisible( TRUE );
		    draggedItem = 0;
		    showItem();
		}
	    } else if ( dropConfirmed ) { // item was dropped
		dropConfirmed = FALSE;
		hideItem();
		itemList.takeNode( node )->setVisible( TRUE );
		hasSeparator = isSeparator || hasSeparator;
		showItem();
	    } else {
		hasSeparator = isSeparator || hasSeparator;
            }
	    update();
	}
    }
}

void MenuBarEditor::dragEnterEvent( TQDragEnterEvent * e )
{
    if ( MenuBarEditorItemPtrDrag::canDecode( e ) ) {
	e->accept();
	dropLine->show();
    }
}

void MenuBarEditor::dragLeaveEvent( TQDragLeaveEvent * )
{
    dropLine->hide();
}

void MenuBarEditor::dragMoveEvent( TQDragMoveEvent * e )
{

    TQPoint pos = e->pos();
    dropLine->move( snapToItem( pos ) );

    int idx = findItem( pos );
    if ( currentIndex != idx ) {
	hideItem();
	currentIndex = idx;
	showItem();
    }
}

void MenuBarEditor::dropEvent( TQDropEvent * e )
{
    MenuBarEditorItem * i = 0;

    if ( MenuBarEditorItemPtrDrag::decode( e, &i ) ) {
	draggedItem = 0;
	hideItem();
	dropInPlace( i, e->pos() );
	e->accept();
    }

    dropLine->hide();
}

void MenuBarEditor::keyPressEvent( TQKeyEvent * e )
{
    if ( lineEdit->isHidden() ) { // In navigation mode
	switch ( e->key() ) {

	case TQt::Key_Delete:
	    hideItem();
	    deleteItem();
	    showItem();
	    break;

	case TQt::Key_Left:
	    e->accept();
	    navigateLeft( e->state() & TQt::ControlButton );
	    return;

	case TQt::Key_Right:
	    e->accept();
	    navigateRight( e->state() & TQt::ControlButton );
	    return; // no update

	case TQt::Key_Down:
	    e->accept();
	    focusItem();
	    return; // no update

	case TQt::Key_PageUp:
	    currentIndex = 0;
	    break;

	case TQt::Key_PageDown:
	    currentIndex = itemList.count();
	    break;

	case TQt::Key_Enter:
	case TQt::Key_Return:
	case TQt::Key_F2:
	    e->accept();
	    enterEditMode();
	    return; // no update

	case TQt::Key_Up:
	case TQt::Key_Alt:
	case TQt::Key_Shift:
	case TQt::Key_Control:
	case TQt::Key_Escape:
	    e->ignore();
	    setFocus(); // FIXME: this is because some other widget get the focus when CTRL is pressed
	    return; // no update

	case TQt::Key_C:
	    if ( e->state() & TQt::ControlButton && currentIndex < (int)itemList.count() ) {
		copy( currentIndex );
		break;
	    }

	case TQt::Key_X:
	    if ( e->state() & TQt::ControlButton && currentIndex < (int)itemList.count() ) {
		hideItem();
		cut( currentIndex );
		showItem();
		break;
	    }

	case TQt::Key_V:
	    if ( e->state() & TQt::ControlButton ) {
		hideItem();
		paste( currentIndex < (int)itemList.count() ? currentIndex + 1: itemList.count() );
		showItem();
		break;
	    }

	default:
	    if ( e->ascii() >= 32 || e->ascii() == 0 ) {
		showLineEdit();
		TQApplication::sendEvent( lineEdit, e );
		e->accept();
	    } else {
		e->ignore();
	    }
	    return;
	}
    } else { // In edit mode

	switch ( e->key() ) {
	case TQt::Key_Control:
	    e->ignore();
	    return;
	case TQt::Key_Enter:
	case TQt::Key_Return:
	    leaveEditMode();
	case TQt::Key_Escape:
	    lineEdit->hide();
	    setFocus();
	    break;
	}
    }
    e->accept();
    update();
}

void MenuBarEditor::focusOutEvent( TQFocusEvent * e )
{
    TQWidget * fw = tqApp->focusWidget();
    if ( e->lostFocus() && !::tqt_cast<PopupMenuEditor*>(fw) )
	hideItem();
    update();
}

void MenuBarEditor::resizeInternals()
{
    dropLine->resize( 2, itemHeight );
    updateGeometry();
}

void MenuBarEditor::drawItems( TQPainter & p )
{
    TQPoint pos( borderSize(), 0 );
    uint c = 0;

    p.setPen( colorGroup().buttonText() );

    MenuBarEditorItem * i = itemList.first();
    while ( i ) {
	if ( i->isVisible() )
	    drawItem( p, i, c++, pos ); // updates x y
	i = itemList.next();
    }

    p.setPen( darkBlue );
    drawItem( p, &addItem, c++, pos );
    if ( !hasSeparator )
	drawItem( p, &addSeparator, c, pos );
}

void MenuBarEditor::drawItem( TQPainter & p,
			      MenuBarEditorItem * i,
			      int idx,
			      TQPoint & pos )
{
    int w = itemSize( i ).width();

    // If the item passes the right border, and it is not the first item on the line
    if ( pos.x() + w > width() && pos.x() > borderSize() ) { // wrap
	pos.ry() += itemHeight;
	pos.setX( borderSize() );
    }

    if ( i->isSeparator() ) {
	drawSeparator( p, pos );
    } else {
	int flags = TQPainter::AlignLeft | TQPainter::AlignVCenter |
	    TQt::ShowPrefix | TQt::SingleLine;
	p.drawText( pos.x() + borderSize(), pos.y(), w - borderSize(), itemHeight,
		    flags, i->menuText() );
    }

    if ( hasFocus() && idx == currentIndex && !draggedItem )
	p.drawWinFocusRect( pos.x(), pos.y() + 1, w, itemHeight - 2 );

    pos.rx() += w;
}

void MenuBarEditor::drawSeparator( TQPainter & p, TQPoint & pos )
{
    p.save();
    p.setPen( darkBlue );

    int left = pos.x();
    int top = pos.y() + 2;
    int right = left + separatorWidth - 1;
    int bottom = pos.y() + itemHeight - 4;

    p.drawLine( left, top, left, bottom );
    p.drawLine( right, top, right, bottom );

    p.fillRect( left, pos.y() + borderSize() * 2,
		separatorWidth - 1, itemHeight - borderSize() * 4,
		TQBrush( darkBlue, TQt::Dense5Pattern ) );

    p.restore();
}

TQSize MenuBarEditor::itemSize( MenuBarEditorItem * i )
{
    if ( i->isSeparator() )
	return TQSize( separatorWidth, itemHeight );
    TQRect r = fontMetrics().boundingRect( i->menuText().remove( "&") );
    return TQSize( r.width() + borderSize() * 2, r.height() + borderSize() * 4 );
}

void MenuBarEditor::addItemSizeToCoords( MenuBarEditorItem * i, int & x, int & y, int w )
{
    int dx = itemSize( i ).width();
    if ( x + dx > w && x > borderSize() ) {
	y += itemHeight;
	x = borderSize();
    }
    x += dx;
}

TQPoint MenuBarEditor::itemPos( int index )
{
    int x = borderSize();
    int y = 0;
    int w = width();
    int dx = 0;
    int c = 0;

    MenuBarEditorItem * i = itemList.first();

    while ( i ) {
	if ( i->isVisible() ) {
	    dx = itemSize( i ).width();
	    if ( x + dx > w && x > borderSize() ) {
		y += itemHeight;
		x = borderSize();
	    }
	    if ( c == index )
		return TQPoint( x, y );
	    x += dx;
	    c++;
	}
	i = itemList.next();
    }
    dx = itemSize( &addItem ).width();
    if ( x + dx > width() && x > borderSize() ) {
	y += itemHeight;
	x = borderSize();
    }

    return TQPoint( x, y );
}

TQPoint MenuBarEditor::snapToItem( const TQPoint & pos )
{
    int x = borderSize();
    int y = 0;
    int dx = 0;

    MenuBarEditorItem * n = itemList.first();

    while ( n ) {
	if ( n->isVisible() ) {
	    dx = itemSize( n ).width();
	    if ( x + dx > width() && x > borderSize() ) {
		y += itemHeight;
		x = borderSize();
	    }
	    if ( pos.y() > y &&
		 pos.y() < y + itemHeight &&
		 pos.x() < x + dx / 2 ) {
		return TQPoint( x, y );
	    }
	    x += dx;
	}
	n = itemList.next();
    }

    return TQPoint( x, y );
}

void MenuBarEditor::dropInPlace( MenuBarEditorItem * i, const TQPoint & pos )
{
    int x = borderSize();
    int y = 0;
    int dx = 0;
    int idx = 0;

    MenuBarEditorItem * n = itemList.first();

    while ( n ) {
	if ( n->isVisible() ) {
	    dx = itemSize( n ).width();
	    if ( x + dx > width() && x > borderSize() ) {
		y += itemHeight;
		x = borderSize();
	    }
	    if ( pos.y() > y &&
		 pos.y() < y + itemHeight &&
		 pos.x() < x + dx / 2 )
		break;
	    x += dx;
	}
	n = itemList.next();
	idx++;
    }

    hideItem();
    Command * cmd = 0;
    int iidx = itemList.findRef( i );
    if ( iidx != -1 ) { // internal dnd
	cmd = new MoveMenuCommand( "Item Dragged", formWnd, this, iidx, idx );
	item( iidx )->setVisible( TRUE );
    } else {
	cmd = new AddMenuCommand( "Add Menu", formWnd, this, i, idx );
	dropConfirmed = TRUE; // let mouseMoveEvent set the item visible
    }
    formWnd->commandHistory()->addCommand( cmd );
    cmd->execute();
    currentIndex = ( iidx >= 0 && iidx < idx ) ? idx - 1 : idx;
    showItem();
}


void MenuBarEditor::safeDec()
{
    do {
	currentIndex--;
    } while ( currentIndex > 0 && !( item( currentIndex )->isVisible() ) );
}

void MenuBarEditor::safeInc()
{
    int max = (int)itemList.count();
    if ( !hasSeparator )
	max += 1;
    if ( currentIndex < max ) {
	do {
	    currentIndex++;
	    // skip invisible items
	} while ( currentIndex < max && !( item( currentIndex )->isVisible() ) );
    }
}

void MenuBarEditor::navigateLeft( bool ctrl )
{
    // FIXME: handle invisible items
    if ( currentIndex > 0 ) {
	hideItem();
	if ( ctrl ) {
	    ExchangeMenuCommand * cmd = new ExchangeMenuCommand( "Move Menu Left",
								 formWnd,
								 this,
								 currentIndex,
								 currentIndex - 1 );
	    formWnd->commandHistory()->addCommand( cmd );
	    cmd->execute();
	    safeDec();
	} else {
	    safeDec();
	}
	showItem();
    }
    update();
}

void MenuBarEditor::navigateRight( bool ctrl )
{
// FIXME: handle invisible items
    hideItem();
    if ( ctrl ) {
	if ( currentIndex < ( (int)itemList.count() - 1 ) ) {
	    ExchangeMenuCommand * cmd =	new ExchangeMenuCommand( "Move Menu Right",
								 formWnd,
								 this,
								 currentIndex,
								 currentIndex + 1 );
	    formWnd->commandHistory()->addCommand( cmd );
	    cmd->execute();
	    safeInc();
	}
    } else {
	safeInc();
    }
    showItem();
    update();
}

void MenuBarEditor::enterEditMode()
{
    if ( currentIndex > (int)itemList.count() ) {
	insertSeparator();
    } else {
	showLineEdit();
    }
}

void MenuBarEditor::leaveEditMode()
{
    MenuBarEditorItem * i = 0;
    if ( currentIndex >= (int)itemList.count() ) {
	i = createItem();
	// do not put rename on cmd stack
	RenameMenuCommand rename( "Rename Menu", formWnd, this, lineEdit->text(), i );
	rename.execute();
    } else {
	i = itemList.at( currentIndex );
	RenameMenuCommand * cmd =
	    new RenameMenuCommand( "Rename Menu", formWnd, this, lineEdit->text(), i );
	formWnd->commandHistory()->addCommand( cmd );
	cmd->execute();
    }
    showItem();
}

TQPixmap MenuBarEditor::createTextPixmap( const TQString &text )
{
    TQSize sz( fontMetrics().boundingRect( text ).size() );
    TQPixmap pix( sz.width() + 20, sz.height() * 2 );
    pix.fill( white );
    TQPainter p( &pix, this );
    p.drawText( 2, 0, pix.width(), pix.height(), 0, text );
    p.end();
    TQBitmap bm( pix.size() );
    bm.fill( color0 );
    p.begin( &bm );
    p.setPen( color1 );
    p.drawText( 2, 0, pix.width(), pix.height(), 0, text );
    p.end();
    pix.setMask( bm );
    return pix;
}
