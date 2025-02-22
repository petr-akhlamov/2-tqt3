/****************************************************************************
**
** Implementation of TQTabWidget class
**
** Created : 990318
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the TQt GUI Toolkit.
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
** This file may be used under the terms of the Q Public License as
** defined by Trolltech ASA and appearing in the file LICENSE.TQPL
** included in the packaging of this file.  Licensees holding valid TQt
** Commercial licenses may use this file in accordance with the TQt
** Commercial License Agreement provided with the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#include "ntqtabwidget.h"
#ifndef TQT_NO_TABWIDGET
#include "ntqobjectlist.h"
#include "ntqtabbar.h"
#include "ntqapplication.h"
#include "ntqwidgetstack.h"
#include "ntqbitmap.h"
#include "ntqaccel.h"
#include "ntqstyle.h"
#include "ntqpainter.h"
#include "ntqtoolbutton.h"

#ifdef Q_OS_MACX
#include <qmacstyle_mac.h>
#endif

/*!
    \class TQTabWidget ntqtabwidget.h
    \brief The TQTabWidget class provides a stack of tabbed widgets.

    \ingroup organizers
    \ingroup advanced
    \mainclass

    A tab widget provides a tab bar of tabs and a `page area' below
    (or above, see \l{TabPosition}) the tabs. Each tab is associated
    with a different widget (called a `page'). Only the current tab's
    page is shown in the page area; all the other tabs' pages are
    hidden. The user can show a different page by clicking on its tab
    or by pressing its Alt+\e{letter} accelerator if it has one.

    The normal way to use TQTabWidget is to do the following in the
    constructor:
    \list 1
    \i Create a TQTabWidget.
    \i Create a TQWidget for each of the pages in the tab dialog,
    insert children into it, set up geometry management for it and use
    addTab() (or insertTab()) to set up a tab and keyboard accelerator
    for it.
    \i Connect to the signals and slots.
    \endlist

    The position of the tabs is set with setTabPosition(), their shape
    with setTabShape(), and their margin with setMargin().

    If you don't call addTab() and the TQTabWidget is already visible,
    then the page you have created will not be visible. Don't
    confuse the object name you supply to the TQWidget constructor and
    the tab label you supply to addTab(). addTab() takes a name which
    indicates an accelerator and is meaningful and descriptive to the
    user, whereas the widget name is used primarily for debugging.

    The signal currentChanged() is emitted when the user selects a
    page.

    The current page is available as an index position with
    currentPageIndex() or as a wiget pointer with currentPage(). You
    can retrieve a pointer to a page with a given index using page(),
    and can find the index position of a page with indexOf(). Use
    setCurrentPage() to show a particular page by index, or showPage()
    to show a page by widget pointer.

    You can change a tab's label and iconset using changeTab() or
    setTabLabel() and setTabIconSet(). A tab page can be removed with
    removePage().

    Each tab is either enabled or disabled at any given time (see
    setTabEnabled()). If a tab is enabled, the tab text is drawn
    normally and the user can select that tab. If it is disabled, the
    tab is drawn in a different way and the user cannot select that
    tab. Note that even if a tab is disabled, the page can still be
    visible, for example if all of the tabs happen to be disabled.

    Although tab widgets can be a very good way to split up a complex
    dialog, it's also very easy to get into a mess. See TQTabDialog for
    some design hints. An alternative is to use a TQWidgetStack for
    which you provide some means of navigating between pages, for
    example, a TQToolBar or a TQListBox.

    Most of the functionality in TQTabWidget is provided by a TQTabBar
    (at the top, providing the tabs) and a TQWidgetStack (most of the
    area, organizing the individual pages).

    <img src=qtabwidget-m.png> <img src=qtabwidget-w.png>

    \sa TQTabDialog, TQToolBox
*/

/*!
  \enum TQt::Corner
  This enum type specifies a corner in a rectangle:
  \value TopLeft top left corner
  \value TopRight top right corner
  \value BottomLeft bottom left corner
  \value BottomRight bottom right corner
*/

/*!
    \enum TQTabWidget::TabPosition

    This enum type defines where TQTabWidget draws the tab row:
    \value Top  above the pages
    \value Bottom  below the pages
*/

/*!
    \enum TQTabWidget::TabShape

    This enum type defines the shape of the tabs:
    \value Rounded  rounded look (normal)
    \value Triangular  triangular look (very unusual, included for completeness)
*/

/* undocumented now
  \obsolete

  \fn void TQTabWidget::selected( const TQString &tabLabel );

  This signal is emitted whenever a tab is selected (raised),
  including during the first show().

  \sa raise()
*/


/*!
    \fn void TQTabWidget::currentChanged( TQWidget* );

    This signal is emitted whenever the current page changes. The
    parameter is the new current page.

    \sa currentPage(), showPage(), tabLabel()
*/

class TQTabBarBase : public TQWidget
{
public:
    TQTabBarBase( TQTabWidget* parent=0, const char* name=0 )
        : TQWidget( parent, name ) {};
protected:
    void paintEvent( TQPaintEvent * )
    {
        TQObject * obj = parent();
        if( obj ){
            TQTabWidget * t = (TQTabWidget *) obj;
            TQPainter p( this );
	    TQStyle::SFlags flags = TQStyle::Style_Default;

	    if ( t->tabPosition() == TQTabWidget::Top )
		flags |= TQStyle::Style_Top;
	    if ( t->tabPosition() == TQTabWidget::Bottom )
		flags |= TQStyle::Style_Bottom;
	    if(parentWidget()->isEnabled())
		flags |= TQStyle::Style_Enabled;

	    style().drawPrimitive( TQStyle::PE_TabBarBase, &p, rect(),
				   colorGroup(), flags );
        }
    }
};

class TQTabWidgetData
{
public:
    TQTabWidgetData()
        : tabs(0), tabBase(0), stack(0), dirty( TRUE ),
          pos( TQTabWidget::Top ), shape( TQTabWidget::Rounded ),
	  leftCornerWidget(0), rightCornerWidget(0) {};
    ~TQTabWidgetData(){};
    TQTabBar* tabs;
    TQTabBarBase* tabBase;
    TQWidgetStack* stack;
    bool dirty;
    TQTabWidget::TabPosition pos;
    TQTabWidget::TabShape shape;
    int alignment;
    TQWidget* leftCornerWidget;
    TQWidget* rightCornerWidget;
};

/*!
    Constructs a tabbed widget called \a name with parent \a parent,
    and widget flags \a f.
*/
TQTabWidget::TQTabWidget( TQWidget *parent, const char *name, WFlags f )
    : TQWidget( parent, name, f )
{
    d = new TQTabWidgetData;
    d->stack = new TQWidgetStack( this, "tab pages" );
    d->stack->installEventFilter( this );
    d->tabBase = new TQTabBarBase( this, "tab base" );
    d->tabBase->resize( 1, 1 );
    setTabBar( new TQTabBar( this, "tab control" ) );

    d->stack->setFrameStyle( TQFrame::TabWidgetPanel | TQFrame::Raised );
#ifdef Q_OS_TEMP
    d->pos = Bottom;
#endif

    setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Expanding ) );
    setFocusPolicy( TabFocus );
    setFocusProxy( d->tabs );

    installEventFilter( this );
#ifdef Q_OS_MACX
    if (::tqt_cast<TQMacStyle*>(&style()))
        setMargin(10); // According to HIGuidelines at least.
#endif
}

/*!
    \reimp
*/
TQTabWidget::~TQTabWidget()
{
    delete d;
}

/*!
    Adds another tab and page to the tab view.

    The new page is \a child; the tab's label is \a label. Note the
    difference between the widget name (which you supply to widget
    constructors and to setTabEnabled(), for example) and the tab
    label. The name is internal to the program and invariant, whereas
    the label is shown on-screen and may vary according to language
    and other factors.

    If the tab's \a label contains an ampersand, the letter following
    the ampersand is used as an accelerator for the tab, e.g. if the
    label is "Bro\&wse" then Alt+W becomes an accelerator which will
    move the focus to this tab.

    If you call addTab() after show() the screen will flicker and the
    user may be confused.

    Adding the same child twice will have undefined behavior.

    \sa insertTab()
*/
void TQTabWidget::addTab( TQWidget *child, const TQString &label)
{
    insertTab( child, label );
}


/*!
    \overload

    Adds another tab and page to the tab view.

    This function is the same as addTab(), but with an additional \a
    iconset.
*/
void TQTabWidget::addTab( TQWidget *child, const TQIconSet& iconset, const TQString &label )
{
    insertTab( child, iconset, label );
}

/*!
    \overload

    This is a low-level function for adding tabs. It is useful if you
    are using setTabBar() to set a TQTabBar subclass with an overridden
    TQTabBar::paint() function for a subclass of TQTab. The \a child is
    the new page and \a tab is the tab to put the \a child on.
*/
void TQTabWidget::addTab( TQWidget *child, TQTab* tab )
{
    insertTab( child, tab );
}



/*!
    Inserts another tab and page to the tab view.

    The new page is \a child; the tab's label is \a label. Note the
    difference between the widget name (which you supply to widget
    constructors and to setTabEnabled(), for example) and the tab
    label. The name is internal to the program and invariant, whereas
    the label is shown on-screen and may vary according to language
    and other factors.

    If the tab's \a label contains an ampersand, the letter following
    the ampersand is used as an accelerator for the tab, e.g. if the
    label is "Bro\&wse" then Alt+W becomes an accelerator which will
    move the focus to this tab.

    If \a index is not specified, the tab is simply appended.
    Otherwise it is inserted at the specified position.

    If you call insertTab() after show(), the screen will flicker and
    the user may be confused.

    \sa addTab()
*/
void TQTabWidget::insertTab( TQWidget *child, const TQString &label, int index)
{
    TQTab * t = new TQTab();
    TQ_CHECK_PTR( t );
    t->label = label;
    insertTab( child, t, index );
}


/*!
    \overload

    Inserts another tab and page to the tab view.

    This function is the same as insertTab(), but with an additional
    \a iconset.
*/
void TQTabWidget::insertTab( TQWidget *child, const TQIconSet& iconset, const TQString &label, int index )
{
    TQTab * t = new TQTab();
    TQ_CHECK_PTR( t );
    t->label = label;
    t->iconset = new TQIconSet( iconset );
    insertTab( child, t, index );
}

/*!
    \overload

    This is a lower-level method for inserting tabs, similar to the
    other insertTab() method. It is useful if you are using
    setTabBar() to set a TQTabBar subclass with an overridden
    TQTabBar::paint() function for a subclass of TQTab. The \a child is
    the new page, \a tab is the tab to put the \a child on and \a
    index is the position in the tab bar that this page should occupy.
*/
void TQTabWidget::insertTab( TQWidget *child, TQTab* tab, int index)
{
    tab->enabled = TRUE;
    int id = d->tabs->insertTab( tab, index );
    d->stack->addWidget( child, id );
    if ( d->stack->frameStyle() != ( TQFrame::TabWidgetPanel | TQFrame::Raised ) )
        d->stack->setFrameStyle( TQFrame::TabWidgetPanel | TQFrame::Raised );
    setUpLayout();
}


/*!
    Defines a new \a label for page \a{w}'s tab.
*/
void TQTabWidget::changeTab( TQWidget *w, const TQString &label)
{
    int id = d->stack->id( w );
    if ( id < 0 )
        return;
    TQTab* t = d->tabs->tab( id );
    if ( !t )
        return;
    // this will update the accelerators
    t->setText( label );

    d->tabs->layoutTabs();
    d->tabs->update();
    setUpLayout();
}

/*!
    \overload

    Defines a new \a iconset and a new \a label for page \a{w}'s tab.
*/
void TQTabWidget::changeTab( TQWidget *w, const TQIconSet& iconset, const TQString &label)
{
    int id = d->stack->id( w );
    if ( id < 0 )
        return;
    TQTab* t = d->tabs->tab( id );
    if ( !t )
        return;
    if ( t->iconset ) {
        delete t->iconset;
        t->iconset = 0;
    }
    // this will update the accelerators
    t->iconset = new TQIconSet( iconset );
    t->setText( label );

    d->tabs->layoutTabs();
    d->tabs->update();
    setUpLayout();
}

/*!
    Returns TRUE if the page \a w is enabled; otherwise returns FALSE.

    \sa setTabEnabled(), TQWidget::isEnabled()
*/

bool TQTabWidget::isTabEnabled( TQWidget* w ) const
{
    int id = d->stack->id( w );
    if ( id >= 0 )
        return w->isEnabled();
    else
        return FALSE;
}

/*!
    If \a enable is TRUE, page \a w is enabled; otherwise page \a w is
    disabled. The page's tab is redrawn appropriately.

    TQTabWidget uses TQWidget::setEnabled() internally, rather than
    keeping a separate flag.

    Note that even a disabled tab/page may be visible. If the page is
    visible already, TQTabWidget will not hide it; if all the pages are
    disabled, TQTabWidget will show one of them.

    \sa isTabEnabled(), TQWidget::setEnabled()
*/

void TQTabWidget::setTabEnabled( TQWidget* w, bool enable)
{
    int id = d->stack->id( w );
    if ( id >= 0 ) {
        w->setEnabled( enable );
        d->tabs->setTabEnabled( id, enable );
    }
}

/*!
  Sets widget \a w to be the shown in the specified \a corner of the
  tab widget.

  Only the horizontal element of the \a corner will be used.

  \sa cornerWidget(), setTabPosition()
*/
void TQTabWidget::setCornerWidget( TQWidget * w, TQt::Corner corner )
{
    if ( !w )
	return;
    if ( (uint)corner & 1 )
	d->rightCornerWidget = w;
    else
	d->leftCornerWidget = w;
}

/*!
    Returns the widget shown in the \a corner of the tab widget or 0.
*/
TQWidget * TQTabWidget::cornerWidget( TQt::Corner corner ) const
{
    if ( (uint)corner & 1 )
	return d->rightCornerWidget;
    return d->leftCornerWidget;
}

/*!
    Ensures that page \a w is shown. This is useful mainly for
    accelerators.

    \warning Used carelessly, this function can easily surprise or
    confuse the user.

    \sa TQTabBar::setCurrentTab()
*/
void TQTabWidget::showPage( TQWidget * w)
{
    int id = d->stack->id( w );
    if ( id >= 0 ) {
        d->stack->raiseWidget( w );
        d->tabs->setCurrentTab( id );
        // ### why overwrite the frame style?
        if ( d->stack->frameStyle() != ( TQFrame::TabWidgetPanel |TQFrame::Raised ) )
            d->stack->setFrameStyle( TQFrame::TabWidgetPanel | TQFrame::Raised );
    }
}

/*!
    Removes page \a w from this stack of widgets. Does not delete \a
    w.

    \sa addTab(), showPage(), TQWidgetStack::removeWidget()
*/
void TQTabWidget::removePage( TQWidget * w )
{
    int id = d->stack->id( w );
    if ( id >= 0 ) {
	int oldId = d->stack->id(currentPage());
	bool fixCurrentTab = oldId == id;
	//switches to the next enabled tab
	d->tabs->setTabEnabled( id, FALSE );
	//if no next enabled page we fix the current page
	fixCurrentTab = fixCurrentTab && oldId == d->stack->id(currentPage());

	d->stack->removeWidget( w );
	d->tabs->removeTab( d->tabs->tab(id) );
	if ( fixCurrentTab )
	    showTab( d->tabs->currentTab() );
	setUpLayout();

	if ( d->tabs->count() == 0 )
	    d->stack->setFrameStyle( TQFrame::NoFrame );
    }
}

/*!
    Returns the label text for the tab on page \a w.
*/

TQString TQTabWidget::tabLabel( TQWidget * w ) const
{
    TQTab * t = d->tabs->tab( d->stack->id( w ) );
    return t ? t->label : TQString::null;
}

/*!
    Sets the tab label for page \a w to \a l
*/

void TQTabWidget::setTabLabel( TQWidget * w, const TQString &l )
{
    TQTab * t = d->tabs->tab( d->stack->id( w ) );
    if ( t )
        t->label = l;
    d->tabs->layoutTabs();
    d->tabs->update();
    setUpLayout();
}

/*!
    Returns a pointer to the page currently being displayed by the tab
    dialog. The tab dialog does its best to make sure that this value
    is never 0 (but if you try hard enough, it can be).
*/

TQWidget * TQTabWidget::currentPage() const
{
    return page( currentPageIndex() );
}

/*!
    \property TQTabWidget::autoMask
    \brief whether the tab widget is automatically masked

    \sa TQWidget::setAutoMask()
*/

/*!
    \property TQTabWidget::currentPage
    \brief the index position of the current tab page

  \sa TQTabBar::currentTab()
*/

int TQTabWidget::currentPageIndex() const
{
    return d->tabs->indexOf( d->tabs->currentTab() );
}

void TQTabWidget::setCurrentPage( int index )
{
    d->tabs->setCurrentTab( d->tabs->tabAt( index ) );
    showTab( d->tabs->currentTab() );
}


/*!
    Returns the index position of page \a w, or -1 if the widget
    cannot be found.
*/
int TQTabWidget::indexOf( TQWidget* w ) const
{
    return d->tabs->indexOf( d->stack->id( w ) );
}


/*!
    \reimp
*/
void TQTabWidget::resizeEvent( TQResizeEvent *e )
{
    TQWidget::resizeEvent( e );
    setUpLayout();
}

/*!
    Replaces the dialog's TQTabBar heading with the tab bar \a tb. Note
    that this must be called \e before any tabs have been added, or
    the behavior is undefined.

    \sa tabBar()
*/
void TQTabWidget::setTabBar( TQTabBar* tb)
{
    if ( tb->parentWidget() != this )
        tb->reparent( this, TQPoint(0,0), TRUE );
    delete d->tabs;
    d->tabs = tb;
    setFocusProxy( d->tabs );
    connect( d->tabs, TQ_SIGNAL(selected(int)),
             this,    TQ_SLOT(showTab(int)) );
    setUpLayout();
}


/*!
    Returns the current TQTabBar.

    \sa setTabBar()
*/
TQTabBar* TQTabWidget::tabBar() const
{
    return d->tabs;
}

/*!
    Ensures that the selected tab's page is visible and appropriately
    sized.
*/

void TQTabWidget::showTab( int i )
{
    if ( d->stack->widget( i ) ) {
	d->stack->raiseWidget( i );
	emit selected( d->tabs->tab( i )->label );
	emit currentChanged( d->stack->widget( i ) );
    }
}

/*
    Set up the layout.
*/
void TQTabWidget::setUpLayout( bool onlyCheck )
{
    if ( onlyCheck && !d->dirty )
	return; // nothing to do

    if ( !isVisible() ) {
	d->dirty = TRUE;
	return; // we'll do it later
    }

    TQSize t( 0, d->stack->frameWidth() );
    if ( d->tabs->isVisibleTo(this) )
	t = d->tabs->sizeHint();
    int lcw = 0;
    if ( d->leftCornerWidget && d->leftCornerWidget->isVisible()  ) {
	TQSize sz = d->leftCornerWidget->sizeHint();
	d->leftCornerWidget->resize(sz);
	lcw = sz.width();
	if ( t.height() > lcw )
	    lcw = t.height();
     }
    int rcw = 0;
    if ( d->rightCornerWidget && d->rightCornerWidget->isVisible() ) {
	TQSize sz = d->rightCornerWidget->sizeHint();
	d->rightCornerWidget->resize(sz);
	rcw = sz.width();
	if ( t.height() > rcw )
	    rcw = t.height();
    }
    int tw = width() - lcw - rcw;
    if ( t.width() > tw )
	t.setWidth( tw );
    int lw = d->stack->lineWidth();
    bool reverse = TQApplication::reverseLayout();
    int tabx, taby, stacky, exty, exth, overlap;

    exth = style().pixelMetric( TQStyle::PM_TabBarBaseHeight, this );
    overlap = style().pixelMetric( TQStyle::PM_TabBarBaseOverlap, this );

    if ( d->pos == Bottom ) {
	taby = height() - t.height() - lw;
	stacky = 0;
	exty = taby - (exth - overlap);
    } else { // Top
	taby = 0;
	stacky = t.height()-lw + (exth - overlap);
	exty = taby + t.height() - overlap;
    }

    int lhs = (TQMAX( 0, lw - 2 ) + lcw);
    int alignment = style().styleHint( TQStyle::SH_TabBar_Alignment, this );
    if ( alignment == AlignHCenter  && t.width() < width() )
        tabx = lhs + ((width()-(lcw+rcw))/2 - t.width()/2);
    else if(reverse || alignment == AlignRight)
        tabx = TQMIN( width() - t.width(), width() - t.width() - lw + 2 ) - lcw;
    else
        tabx = lhs;

    d->tabs->setGeometry( tabx, taby, t.width(), t.height() );
    d->tabBase->setGeometry( 0, exty, width(), exth );
    if ( exth == 0 )
	d->tabBase->hide();
    else
	d->tabBase->show();

    d->stack->setGeometry( 0, stacky, width(), height() - (exth-overlap) -
			   t.height()+TQMAX(0, lw-2));

    d->dirty = FALSE;

    // move cornerwidgets
    if ( d->leftCornerWidget ) {
	int y = ( t.height() / 2 ) - ( d->leftCornerWidget->height() / 2 );
	int x = ( reverse ? width() - lcw + y : y );
	d->leftCornerWidget->move( x, y + taby );
    }
    if ( d->rightCornerWidget ) {
	int y = ( t.height() / 2 ) - ( d->rightCornerWidget->height() / 2 );
	int x = ( reverse ? y : width() - rcw + y );
	d->rightCornerWidget->move( x, y + taby );
    }
    if ( !onlyCheck )
	update();
    updateGeometry();
    if ( autoMask() )
	updateMask();
}

/*!
    \reimp
*/
TQSize TQTabWidget::sizeHint() const
{
    TQSize lc(0, 0), rc(0, 0);

    if (d->leftCornerWidget)
	lc = d->leftCornerWidget->sizeHint();
    if(d->rightCornerWidget)
	rc = d->rightCornerWidget->sizeHint();
    if ( !d->dirty ) {
	TQTabWidget *that = (TQTabWidget*)this;
	that->setUpLayout( TRUE );
    }
    TQSize s( d->stack->sizeHint() );
    TQSize t( d->tabs->sizeHint() );
    if(!style().styleHint(TQStyle::SH_TabBar_PreferNoArrows, d->tabs))
	t = t.boundedTo( TQSize(200,200) );
    else
        t = t.boundedTo( TQApplication::desktop()->size() );

    TQSize sz( TQMAX( s.width(), t.width() + rc.width() + lc.width() ),
	      s.height() + (TQMAX( rc.height(), TQMAX(lc.height(), t.height()))) + ( d->tabBase->isVisible() ? d->tabBase->height() : 0 ) );
    return style().sizeFromContents(TQStyle::CT_TabWidget, this, sz).expandedTo(TQApplication::globalStrut());
}


/*!
    \reimp

    Returns a suitable minimum size for the tab widget.
*/
TQSize TQTabWidget::minimumSizeHint() const
{
    TQSize lc(0, 0), rc(0, 0);

    if(d->leftCornerWidget)
	lc = d->leftCornerWidget->minimumSizeHint();
    if(d->rightCornerWidget)
	rc = d->rightCornerWidget->minimumSizeHint();
    if ( !d->dirty ) {
	TQTabWidget *that = (TQTabWidget*)this;
	that->setUpLayout( TRUE );
    }
    TQSize s( d->stack->minimumSizeHint() );
    TQSize t( d->tabs->minimumSizeHint() );

    TQSize sz( TQMAX( s.width(), t.width() + rc.width() + lc.width() ),
	      s.height() + (TQMAX( rc.height(), TQMAX(lc.height(), t.height()))) + ( d->tabBase->isVisible() ? d->tabBase->height() : 0 ) );
    return style().sizeFromContents(TQStyle::CT_TabWidget, this, sz).expandedTo(TQApplication::globalStrut());
}

/*!
    \reimp
 */
void TQTabWidget::showEvent( TQShowEvent * )
{
    setUpLayout();
}


/*!
    \property TQTabWidget::tabPosition
    \brief the position of the tabs in this tab widget

    Possible values for this property are \c TQTabWidget::Top and \c
    TQTabWidget::Bottom.

    \sa TabPosition
*/
TQTabWidget::TabPosition TQTabWidget::tabPosition() const
{
    return d->pos;
}

void TQTabWidget::setTabPosition( TabPosition pos)
{
    if (d->pos == pos)
        return;
    d->pos = pos;
    if (d->tabs->shape() == TQTabBar::TriangularAbove || d->tabs->shape() == TQTabBar::TriangularBelow ) {
        if ( pos == Bottom )
            d->tabs->setShape( TQTabBar::TriangularBelow );
        else
            d->tabs->setShape( TQTabBar::TriangularAbove );
    }
    else {
        if ( pos == Bottom )
            d->tabs->setShape( TQTabBar::RoundedBelow );
        else
            d->tabs->setShape( TQTabBar::RoundedAbove );
    }
    d->tabs->layoutTabs();
    setUpLayout();
}

/*!
    \property TQTabWidget::tabShape
    \brief the shape of the tabs in this tab widget

    Possible values for this property are \c TQTabWidget::Rounded
    (default) or \c TQTabWidget::Triangular.

    \sa TabShape
*/

TQTabWidget::TabShape TQTabWidget::tabShape() const
{
    return d->shape;
}

void TQTabWidget::setTabShape( TabShape s )
{
    if ( d->shape == s )
        return;
    d->shape = s;
    if ( d->pos == Top ) {
        if ( s == Rounded )
            d->tabs->setShape( TQTabBar::RoundedAbove );
        else
            d->tabs->setShape( TQTabBar::TriangularAbove );
    } else {
        if ( s == Rounded )
            d->tabs->setShape( TQTabBar::RoundedBelow );
        else
            d->tabs->setShape( TQTabBar::TriangularBelow );
    }
    d->tabs->layoutTabs();
    setUpLayout();
}


/*!
    \property TQTabWidget::margin
    \brief the margin in this tab widget

    The margin is the distance between the innermost pixel of the
    frame and the outermost pixel of the pages.
*/
int TQTabWidget::margin() const
{
    return d->stack->margin();
}

void TQTabWidget::setMargin( int w )
{
    d->stack->setMargin( w );
    setUpLayout();
}


/*!
    \reimp
 */
void TQTabWidget::styleChange( TQStyle& old )
{
    TQWidget::styleChange( old );
    setUpLayout();
}


/*!
    \reimp
 */
void TQTabWidget::updateMask()
{
    if ( !autoMask() )
        return;

    TQRect r;
    TQRegion reg( r );
    reg += TQRegion( d->tabs->geometry() );
    reg += TQRegion( d->stack->geometry() );
    setMask( reg );
}


/*!
    \reimp
 */
bool TQTabWidget::eventFilter( TQObject *o, TQEvent * e)
{
    if ( o == this ) {
	if ( e->type() == TQEvent::LanguageChange || e->type() == TQEvent::LayoutHint ) {
	    d->dirty = TRUE;
	    setUpLayout();
	    updateGeometry();
	} else if ( e->type() == TQEvent::KeyPress ) {
	    TQKeyEvent *ke = (TQKeyEvent*) e;
	    if ( ( ke->key() == TQt::Key_Tab || ke->key() == TQt::Key_Backtab ) &&
		 count() > 1 &&
		 ke->state() & TQt::ControlButton ) {
		int page = currentPageIndex();
		if ( ke->key() == TQt::Key_Backtab || ke->state() & TQt::ShiftButton ) {
		    page--;
		    if ( page < 0 )
			page = count() - 1;
		} else {
		    page++;
		    if ( page >= count() )
			page = 0;
		}
		setCurrentPage( page );
		if ( !tqApp->focusWidget() )
		    d->tabs->setFocus();
		return TRUE;
	    }
	}

    } else if ( o == d->stack ) {
	if ( e->type() == TQEvent::ChildRemoved
	     && ( (TQChildEvent*)e )->child()->isWidgetType() ) {
	    removePage( (TQWidget*)  ( (TQChildEvent*)e )->child() );
	    return TRUE;
	} else if ( e->type() == TQEvent::LayoutHint ) {
	    updateGeometry();
	}
    }
    return FALSE;
}

/*!
    Returns the tab page at index position \a index or 0 if the \a
    index is out of range.
*/
TQWidget *TQTabWidget::page( int index ) const
{
    TQTab *t = d->tabs->tabAt(index);
    if ( t )
	return d->stack->widget( t->id );
    // else
    return 0;
}

/*!
    Returns the label of the tab at index position \a index or
    TQString::null if the \a index is out of range.
*/
TQString TQTabWidget::label( int index ) const
{
    TQTab *t = d->tabs->tabAt( index );
    if ( t )
 	return t->label;
    // else
    return TQString::null;
}

/*!
    \property TQTabWidget::count
    \brief the number of tabs in the tab bar
*/
int TQTabWidget::count() const
{
    return d->tabs->count();
}

/*!
    Returns the iconset of page \a w or a \link TQIconSet::TQIconSet()
    null iconset\endlink if \a w is not a tab page or does not have an
    iconset.
*/
TQIconSet TQTabWidget::tabIconSet( TQWidget * w ) const
{
    int id = d->stack->id( w );
    if ( id < 0 )
        return TQIconSet();
    TQTab* t = d->tabs->tab( id );
    if ( !t )
        return TQIconSet();
    if ( t->iconset )
	return TQIconSet( *t->iconset );
    else
	return TQIconSet();
}

/*!
    Sets the iconset for page \a w to \a iconset.
*/
void TQTabWidget::setTabIconSet( TQWidget * w, const TQIconSet & iconset )
{
    int id = d->stack->id( w );
    if ( id < 0 )
        return;
    TQTab* t = d->tabs->tab( id );
    if ( !t )
        return;
    if ( t->iconset )
        delete t->iconset;
    t->iconset = new TQIconSet( iconset );

    d->tabs->layoutTabs();
    d->tabs->update();
    setUpLayout();
}

/*!
    Sets the tab tool tip for page \a w to \a tip.

    \sa removeTabToolTip(), tabToolTip()
*/
void TQTabWidget::setTabToolTip( TQWidget * w, const TQString & tip )
{
    int index = d->tabs->indexOf( d->stack->id( w ) );
    if ( index < 0 )
        return;
    d->tabs->setToolTip( index, tip );
}

/*!
    Returns the tab tool tip for page \a w or TQString::null if no tool
    tip has been set.

    \sa setTabToolTip(), removeTabToolTip()
*/
TQString TQTabWidget::tabToolTip( TQWidget * w ) const
{
    int index = d->tabs->indexOf( d->stack->id( w ) );
    if ( index < 0 )
        return TQString();
    return d->tabs->toolTip( index );
}

/*!
    Removes the tab tool tip for page \a w. If the page does not have
    a tip, nothing happens.

    \sa setTabToolTip(), tabToolTip()
*/
void TQTabWidget::removeTabToolTip( TQWidget * w )
{
    int index = d->tabs->indexOf( d->stack->id( w ) );
    if ( index < 0 )
        return;
    d->tabs->removeToolTip( index );
}

#endif
