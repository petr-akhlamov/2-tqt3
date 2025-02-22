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

#include "formwindow.h"
#include "layout.h"
#include <widgetdatabase.h>
#include "widgetfactory.h"

#include <ntqlayout.h>
#include <ntqevent.h>
#include <ntqpainter.h>
#include <ntqpen.h>
#include <ntqbitmap.h>
#include <ntqsplitter.h>
#include <ntqvaluevector.h>
#include <ntqmainwindow.h>

bool operator<( const TQGuardedPtr<TQWidget> &p1, const TQGuardedPtr<TQWidget> &p2 )
{
    return p1.operator->() < p2.operator->();
}

/*!
  \class Layout layout.h
  \brief Baseclass for layouting widgets in the Designer

  Classes derived from this abstract base class are used for layouting
  operations in the Designer.

*/

/*!  \a p specifies the parent of the layoutBase \a lb. The parent
  might be changed in setup(). If the layoutBase is a
  container, the parent and the layoutBase are the same. Also they
  always have to be a widget known to the designer (e.g. in the case
  of the tabwidget parent and layoutBase are the tabwidget and not the
  page which actually gets laid out. For actual usage the correct
  widget is found later by Layout.)
 */

Layout::Layout( const TQWidgetList &wl, TQWidget *p, FormWindow *fw, TQWidget *lb, bool doSetup, bool splitter )
    : widgets( wl ), parent( p ), formWindow( fw ), isBreak( !doSetup ), useSplitter( splitter )
{
    widgets.setAutoDelete( FALSE );
    layoutBase = lb;
    if ( !doSetup && layoutBase )
	oldGeometry = layoutBase->geometry();
}

/*!  The widget list we got in the constructor might contain too much
  widgets (like widgets with different parents, already laid out
  widgets, etc.). Here we set up the list and so the only the "best"
  widgets get laid out.
*/

void Layout::setup()
{
    startPoint = TQPoint( 32767, 32767 );
    TQValueList<TQWidgetList> lists;
    TQWidget *lastParent = 0;
    TQWidgetList *lastList = 0;
    TQWidget *w = 0;

    // Go through all widgets of the list we got. As we can only
    // layout widgets which have the same parent, we first do some
    // sorting which means create a list for each parent containing
    // its child here. After that we keep working on the list of
    // childs which has the most entries.
    // Widgets which are already laid out are thrown away here too
    for ( w = widgets.first(); w; w = widgets.next() ) {
	if ( w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout )
	    continue;
	if ( lastParent != w->parentWidget() ) {
	    lastList = 0;
	    lastParent = w->parentWidget();
	    TQValueList<TQWidgetList>::Iterator it = lists.begin();
	    for ( ; it != lists.end(); ++it ) {
		if ( ( *it ).first()->parentWidget() == w->parentWidget() )
		    lastList = &( *it );
	    }
	    if ( !lastList ) {
		TQWidgetList l;
		l.setAutoDelete( FALSE );
		lists.append( l );
		lastList = &lists.last();
	    }
	}
	lastList->append( w );
    }

    // So, now find the list with the most entries
    lastList = 0;
    TQValueList<TQWidgetList>::Iterator it = lists.begin();
    for ( ; it != lists.end(); ++it ) {
	if ( !lastList || ( *it ).count() > lastList->count() )
	    lastList = &( *it );
    }

    // If we found no list (because no widget did fit at all) or the
    // best list has only one entry and we do not layout a container,
    // we leave here.
    if ( !lastList || ( lastList->count() < 2 &&
			( !layoutBase ||
			  ( !WidgetDatabase::isContainer( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( layoutBase ) ) ) &&
			    layoutBase != formWindow->mainContainer() ) )
			) ) {
	widgets.clear();
	startPoint = TQPoint( 0, 0 );
	return;
    }

    // Now we have a new and clean widget list, which makes sense
    // to layout
    widgets = *lastList;
    // Also use the only correct parent later, so store it
    parent = WidgetFactory::widgetOfContainer( widgets.first()->parentWidget() );
    // Now calculate the position where the layout-meta-widget should
    // be placed and connect to widgetDestroyed() signals of the
    // widgets to get informed if one gets deleted to be able to
    // handle that and do not crash in this case
    for ( w = widgets.first(); w; w = widgets.next() ) {
	connect( w, TQ_SIGNAL( destroyed() ),
		 this, TQ_SLOT( widgetDestroyed() ) );
	startPoint = TQPoint( TQMIN( startPoint.x(), w->x() ),
			     TQMIN( startPoint.y(), w->y() ) );
	geometries.insert( w, TQRect( w->pos(), w->size() ) );
	// Change the Z-order, as saving/loading uses the Z-order for
	// writing/creating widgets and this has to be the same as in
	// the layout. Else saving + loading will give different results
	w->raise();
    }
}

void Layout::widgetDestroyed()
{
     if ( sender() && sender()->isWidgetType() )
	widgets.removeRef( (TQWidget*)sender() );
}

bool Layout::prepareLayout( bool &needMove, bool &needReparent )
{
    if ( !widgets.count() )
	return FALSE;
    for ( TQWidget *w = widgets.first(); w; w = widgets.next() )
	w->raise();
    needMove = !layoutBase;
    needReparent = needMove || ::tqt_cast<TQLayoutWidget*>(layoutBase) || ::tqt_cast<TQSplitter*>(layoutBase);
    if ( !layoutBase ) {
	if ( !useSplitter )
	    layoutBase = WidgetFactory::create( WidgetDatabase::idFromClassName( "TQLayoutWidget" ),
						WidgetFactory::containerOfWidget( parent ) );
	else
	    layoutBase = WidgetFactory::create( WidgetDatabase::idFromClassName( "TQSplitter" ),
						WidgetFactory::containerOfWidget( parent ) );
    } else {
	WidgetFactory::deleteLayout( layoutBase );
    }

    return TRUE;
}

void Layout::finishLayout( bool needMove, TQLayout *layout )
{
    if ( needMove )
	layoutBase->move( startPoint );
    TQRect g( TQRect( layoutBase->pos(), layoutBase->size() ) );
    if ( WidgetFactory::layoutType( layoutBase->parentWidget() ) == WidgetFactory::NoLayout && !isBreak )
	layoutBase->adjustSize();
    else if ( isBreak )
	layoutBase->setGeometry( oldGeometry );
    oldGeometry = g;
    layoutBase->show();
    layout->activate();
    formWindow->insertWidget( layoutBase );
    formWindow->selectWidget( layoutBase );
    TQString n = layoutBase->name();
    if ( n.find( "qt_dead_widget_" ) != -1 ) {
	n.remove( 0, TQString( "qt_dead_widget_" ).length() );
	layoutBase->setName( n );
    }
}

void Layout::undoLayout()
{
    if ( !widgets.count() )
	return;
    TQMap<TQGuardedPtr<TQWidget>, TQRect>::Iterator it = geometries.begin();
    for ( ; it != geometries.end(); ++it ) {
	if ( !it.key() )
	    continue;
	it.key()->reparent( WidgetFactory::containerOfWidget( parent ), 0, ( *it ).topLeft(), it.key()->isVisibleTo( formWindow ) );
	it.key()->resize( ( *it ).size() );
    }
    formWindow->selectWidget( layoutBase, FALSE );
    WidgetFactory::deleteLayout( layoutBase );
    if ( parent != layoutBase && !::tqt_cast<TQMainWindow*>(layoutBase) ) {
	layoutBase->hide();
	TQString n = layoutBase->name();
	n.prepend( "qt_dead_widget_" );
	layoutBase->setName( n );
    } else {
	layoutBase->setGeometry( oldGeometry );
    }
    if ( widgets.first() )
	formWindow->selectWidget( widgets.first() );
    else
	formWindow->selectWidget( formWindow );
}

void Layout::breakLayout()
{
    TQMap<TQWidget*, TQRect> rects;
    if ( !widgets.isEmpty() ) {
	TQWidget *w;
	for ( w = widgets.first(); w; w = widgets.next() )
	    rects.insert( w, w->geometry() );
    }
    WidgetFactory::deleteLayout( layoutBase );
    bool needReparent = qstrcmp( layoutBase->className(), "TQLayoutWidget" ) == 0 ||
			qstrcmp( layoutBase->className(), "TQSplitter" ) == 0 ||
			( !WidgetDatabase::isContainer( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( layoutBase ) ) ) &&
			  layoutBase != formWindow->mainContainer() );
    bool needResize = qstrcmp( layoutBase->className(), "TQSplitter" ) == 0;
    bool add = geometries.isEmpty();
    for ( TQWidget *w = widgets.first(); w; w = widgets.next() ) {
	if ( needReparent )
	    w->reparent( layoutBase->parentWidget(), 0,
			 layoutBase->pos() + w->pos(), TRUE );
	if ( needResize ) {
	    TQMap<TQWidget*, TQRect>::Iterator it = rects.find( w );
	    if ( it != rects.end() )
		w->setGeometry( TQRect( layoutBase->pos() + (*it).topLeft(), (*it).size() ) );
	}
	if ( add )
	    geometries.insert( w, TQRect( w->pos(), w->size() ) );
    }
    if ( needReparent ) {
	layoutBase->hide();
	parent = layoutBase->parentWidget();
	TQString n = layoutBase->name();
	n.prepend( "qt_dead_widget_" );
	layoutBase->setName( n );
    } else {
	parent = layoutBase;
    }
    if ( widgets.first() && widgets.first()->isVisibleTo( formWindow ) )
	formWindow->selectWidget( widgets.first() );
    else
	formWindow->selectWidget( formWindow );
}

class HorizontalLayoutList : public TQWidgetList
{
public:
    HorizontalLayoutList( const TQWidgetList &l )
	: TQWidgetList( l ) {}

    int compareItems( TQPtrCollection::Item item1, TQPtrCollection::Item item2 ) {
	TQWidget *w1 = (TQWidget*)item1;
	TQWidget *w2 = (TQWidget*)item2;
	if ( w1->x() == w2->x() )
	    return 0;
	if ( w1->x() > w2->x() )
	    return 1;
	return -1;
    }

};

HorizontalLayout::HorizontalLayout( const TQWidgetList &wl, TQWidget *p, FormWindow *fw, TQWidget *lb, bool doSetup, bool splitter )
    : Layout( wl, p, fw, lb, doSetup, splitter )
{
    if ( doSetup )
	setup();
}

void HorizontalLayout::setup()
{
    HorizontalLayoutList l( widgets );
    l.sort();
    widgets = l;
    Layout::setup();
}

void HorizontalLayout::doLayout()
{
    bool needMove, needReparent;
    if ( !prepareLayout( needMove, needReparent ) )
	return;

    TQHBoxLayout *layout = (TQHBoxLayout*)WidgetFactory::createLayout( layoutBase, 0, WidgetFactory::HBox );

    for ( TQWidget *w = widgets.first(); w; w = widgets.next() ) {
	if ( needReparent && w->parent() != layoutBase )
	    w->reparent( layoutBase, 0, TQPoint( 0, 0 ), FALSE );
	if ( !useSplitter ) {
	    if ( qstrcmp( w->className(), "Spacer" ) == 0 )
		layout->addWidget( w, 0, ( (Spacer*)w )->alignment() );
	    else
		layout->addWidget( w );
	    if ( ::tqt_cast<TQLayoutWidget*>(w) )
		( (TQLayoutWidget*)w )->updateSizePolicy();
	}
	w->show();
    }

    if ( ::tqt_cast<TQSplitter*>(layoutBase) )
	( (TQSplitter*)layoutBase )->setOrientation( TQt::Horizontal );

    finishLayout( needMove, layout );
}




class VerticalLayoutList : public TQWidgetList
{
public:
    VerticalLayoutList( const TQWidgetList &l )
	: TQWidgetList( l ) {}

    int compareItems( TQPtrCollection::Item item1, TQPtrCollection::Item item2 ) {
	TQWidget *w1 = (TQWidget*)item1;
	TQWidget *w2 = (TQWidget*)item2;
	if ( w1->y() == w2->y() )
	    return 0;
	if ( w1->y() > w2->y() )
	    return 1;
	return -1;
    }

};

VerticalLayout::VerticalLayout( const TQWidgetList &wl, TQWidget *p, FormWindow *fw, TQWidget *lb, bool doSetup, bool splitter )
    : Layout( wl, p, fw, lb, doSetup, splitter )
{
    if ( doSetup )
	setup();
}

void VerticalLayout::setup()
{
    VerticalLayoutList l( widgets );
    l.sort();
    widgets = l;
    Layout::setup();
}

void VerticalLayout::doLayout()
{
    bool needMove, needReparent;
    if ( !prepareLayout( needMove, needReparent ) )
	return;

    TQVBoxLayout *layout = (TQVBoxLayout*)WidgetFactory::createLayout( layoutBase, 0, WidgetFactory::VBox );

    for ( TQWidget *w = widgets.first(); w; w = widgets.next() ) {
	if ( needReparent && w->parent() != layoutBase )
	    w->reparent( layoutBase, 0, TQPoint( 0, 0 ), FALSE );
	if ( !useSplitter ) {
	    if ( qstrcmp( w->className(), "Spacer" ) == 0 )
		layout->addWidget( w, 0, ( (Spacer*)w )->alignment() );
	    else
		layout->addWidget( w );
	    if ( ::tqt_cast<TQLayoutWidget*>(w) )
		( (TQLayoutWidget*)w )->updateSizePolicy();
	}
	w->show();
    }

    if ( ::tqt_cast<TQSplitter*>(layoutBase) )
	( (TQSplitter*)layoutBase )->setOrientation( TQt::Vertical );

    finishLayout( needMove, layout );
}





class Grid
{
public:
    Grid( int rows, int cols );
    ~Grid();

    TQWidget* cell( int row, int col ) const { return cells[ row * ncols + col]; }
    void setCell( int row, int col, TQWidget* w ) { cells[ row*ncols + col] = w; }
    void setCells( TQRect c, TQWidget* w ) {
	for ( int rows = c.bottom()-c.top(); rows >= 0; rows--)
	    for ( int cols = c.right()-c.left(); cols >= 0; cols--) {
		setCell(c.top()+rows, c.left()+cols, w);
	    }
    }
    int numRows() const { return nrows; }
    int numCols() const { return ncols; }

    void simplify();
    bool locateWidget( TQWidget* w, int& row, int& col, int& rowspan, int& colspan );

private:
    void merge();
    int countRow( int r, int c ) const;
    int countCol( int r, int c ) const;
    void setRow( int r, int c, TQWidget* w, int count );
    void setCol( int r, int c, TQWidget* w, int count );
    bool isWidgetStartCol( int c ) const;
    bool isWidgetEndCol( int c ) const;
    bool isWidgetStartRow( int r ) const;
    bool isWidgetEndRow( int r ) const;
    bool isWidgetTopLeft( int r, int c ) const;
    void extendLeft();
    void extendRight();
    void extendUp();
    void extendDown();
    TQWidget** cells;
    bool* cols;
    bool* rows;
    int nrows, ncols;

};

Grid::Grid( int r, int c )
    : nrows( r ), ncols( c )
{
    cells = new TQWidget*[ r * c ];
    memset( cells, 0, sizeof( cells ) * r * c );
    rows = new bool[ r ];
    cols = new bool[ c ];

}

Grid::~Grid()
{
    delete [] cells;
    delete [] cols;
    delete [] rows;
}

int Grid::countRow( int r, int c ) const
{
    TQWidget* w = cell( r, c );
    int i = c + 1;
    while ( i < ncols && cell( r, i ) == w )
	i++;
    return i - c;
}

int Grid::countCol( int r, int c ) const
{
    TQWidget* w = cell( r, c );
    int i = r + 1;
    while ( i < nrows && cell( i, c ) == w )
	i++;
    return i - r;
}

void Grid::setCol( int r, int c, TQWidget* w, int count )
{
    for (int i = 0; i < count; i++ )
	setCell( r + i, c, w );
}

void Grid::setRow( int r, int c, TQWidget* w, int count )
{
    for (int i = 0; i < count; i++ )
	setCell( r, c + i, w );
}

bool Grid::isWidgetStartCol( int c ) const
{
    int r;
    for ( r = 0; r < nrows; r++ ) {
	if ( cell( r, c ) && ( (c==0) || (cell( r, c)  != cell( r, c-1) )) ) {
	    return TRUE;
	}
    }
    return FALSE;
}

bool Grid::isWidgetEndCol( int c ) const
{
    int r;
    for ( r = 0; r < nrows; r++ ) {
	if ( cell( r, c ) && ((c == ncols-1) || (cell( r, c) != cell( r, c+1) )) )
	    return TRUE;
    }
    return FALSE;
}

bool Grid::isWidgetStartRow( int r ) const
{
    int c;
    for ( c = 0; c < ncols; c++ ) {
	if ( cell( r, c ) && ( (r==0) || (cell( r, c) != cell( r-1, c) )) )
	    return TRUE;
    }
    return FALSE;
}

bool Grid::isWidgetEndRow( int r ) const
{
    int c;
    for ( c = 0; c < ncols; c++ ) {
	if ( cell( r, c ) && ((r == nrows-1) || (cell( r, c) != cell( r+1, c) )) )
	    return TRUE;
    }
    return FALSE;
}


bool Grid::isWidgetTopLeft( int r, int c ) const
{
    TQWidget* w = cell( r, c );
    if ( !w )
	return FALSE;
    return ( !r || cell( r-1, c) != w ) && (!c || cell( r, c-1) != w);
}

void Grid::extendLeft()
{
    int r,c,i;
    for ( c = 1; c < ncols; c++ ) {
	for ( r = 0; r < nrows; r++ ) {
	    TQWidget* w = cell( r, c );
	    if ( !w )
		continue;
	    int cc = countCol( r, c);
	    int stretch = 0;
	    for ( i = c-1; i >= 0; i-- ) {
		if ( cell( r, i ) )
		    break;
		if ( countCol( r, i ) < cc )
		    break;
		if ( isWidgetEndCol( i ) )
		    break;
		if ( isWidgetStartCol( i ) ) {
		    stretch = c - i;
		    break;
		}
	    }
	    if ( stretch ) {
		for ( i = 0; i < stretch; i++ )
		    setCol( r, c-i-1, w, cc );
	    }
	}
    }
}


void Grid::extendRight()
{
    int r,c,i;
    for ( c = ncols - 2; c >= 0; c-- ) {
	for ( r = 0; r < nrows; r++ ) {
	    TQWidget* w = cell( r, c );
	    if ( !w )
		continue;
	    int cc = countCol( r, c);
	    int stretch = 0;
	    for ( i = c+1; i < ncols; i++ ) {
		if ( cell( r, i ) )
		    break;
		if ( countCol( r, i ) < cc )
		    break;
		if ( isWidgetStartCol( i ) )
		    break;
		if ( isWidgetEndCol( i ) ) {
		    stretch = i - c;
		    break;
		}
	    }
	    if ( stretch ) {
		for ( i = 0; i < stretch; i++ )
		    setCol( r, c+i+1, w, cc );
	    }
	}
    }

}

void Grid::extendUp()
{
    int r,c,i;
    for ( r = 1; r < nrows; r++ ) {
	for ( c = 0; c < ncols; c++ ) {
	    TQWidget* w = cell( r, c );
	    if ( !w )
		continue;
	    int cr = countRow( r, c);
	    int stretch = 0;
	    for ( i = r-1; i >= 0; i-- ) {
		if ( cell( i, c ) )
		    break;
		if ( countRow( i, c ) < cr )
		    break;
		if ( isWidgetEndRow( i ) )
		    break;
		if ( isWidgetStartRow( i ) ) {
		    stretch = r - i;
		    break;
		}
	    }
	    if ( stretch ) {
		for ( i = 0; i < stretch; i++ )
		    setRow( r-i-1, c, w, cr );
	    }
	}
    }
}

void Grid::extendDown()
{
    int r,c,i;
    for ( r = nrows - 2; r >= 0; r-- ) {
	for ( c = 0; c < ncols; c++ ) {
	    TQWidget* w = cell( r, c );
	    if ( !w )
		continue;
	    int cr = countRow( r, c);
	    int stretch = 0;
	    for ( i = r+1; i < nrows; i++ ) {
		if ( cell( i, c ) )
		    break;
		if ( countRow( i, c ) < cr )
		    break;
		if ( isWidgetStartRow( i ) )
		    break;
		if ( isWidgetEndRow( i ) ) {
		    stretch = i - r;
		    break;
		}
	    }
	    if ( stretch ) {
		for ( i = 0; i < stretch; i++ )
		    setRow( r+i+1, c, w, cr );
	    }
	}
    }

}

void Grid::simplify()
{
    extendLeft();
    extendRight();
    extendUp();
    extendDown();
    merge();
}


void Grid::merge()
{
    int r,c;
    for ( c = 0; c < ncols; c++ )
	cols[c] = FALSE;

    for ( r = 0; r < nrows; r++ )
	rows[r] = FALSE;

    for ( c = 0; c < ncols; c++ ) {
	for ( r = 0; r < nrows; r++ ) {
	    if ( isWidgetTopLeft( r, c ) ) {
		rows[r] = TRUE;
		cols[c] = TRUE;
	    }
	}
    }
}

bool Grid::locateWidget( TQWidget* w, int& row, int& col, int& rowspan, int & colspan )
{
    int r,c, r2, c2;
    for ( c = 0; c < ncols; c++ ) {
	for ( r = 0; r < nrows; r++ ) {
	    if ( cell( r, c ) == w  ) {
		row = 0;
		for ( r2 = 1; r2 <= r; r2++ ) {
		    if ( rows[ r2-1 ] )
			row++;
		}
		col = 0;
		for ( c2 = 1; c2 <= c; c2++ ) {
		    if ( cols[ c2-1 ] )
			col++;
		}
		rowspan = 0;
		for ( r2 = r ; r2 < nrows && cell( r2, c) == w; r2++ ) {
		    if ( rows[ r2 ] )
			rowspan++;
		}
		colspan = 0;
		for ( c2 = c; c2 < ncols && cell( r, c2) == w; c2++ ) {
		    if ( cols[ c2] )
			colspan++;
		}
		return TRUE;
	    }
	}
    }
    return FALSE;
}




GridLayout::GridLayout( const TQWidgetList &wl, TQWidget *p, FormWindow *fw, TQWidget *lb, const TQSize &res, bool doSetup )
    : Layout( wl, p, fw, lb, doSetup ), resolution( res )
{
    grid = 0;
    if ( doSetup )
	setup();
}

GridLayout::~GridLayout()
{
    delete grid;
}

void GridLayout::doLayout()
{
    bool needMove, needReparent;
    if ( !prepareLayout( needMove, needReparent ) )
	return;

    TQDesignerGridLayout *layout = (TQDesignerGridLayout*)WidgetFactory::createLayout( layoutBase, 0, WidgetFactory::Grid );

    if ( !grid )
	buildGrid();

    TQWidget* w;
    int r, c, rs, cs;
    for ( w = widgets.first(); w; w = widgets.next() ) {
	if ( grid->locateWidget( w, r, c, rs, cs) ) {
	    if ( needReparent && w->parent() != layoutBase )
		w->reparent( layoutBase, 0, TQPoint( 0, 0 ), FALSE );
	    if ( rs * cs == 1 ) {
		layout->addWidget( w, r, c, ::tqt_cast<Spacer*>(w) ? ( (Spacer*)w )->alignment() : 0 );
	    } else {
		layout->addMultiCellWidget( w, r, r+rs-1, c, c+cs-1, ::tqt_cast<Spacer*>(w) ? ( (Spacer*)w )->alignment() : 0 );
	    }
	    if ( ::tqt_cast<TQLayoutWidget*>(w) )
		( (TQLayoutWidget*)w )->updateSizePolicy();
	    w->show();
	} else {
	    tqWarning("ooops, widget '%s' does not fit in layout", w->name() );
	}
    }
    finishLayout( needMove, layout );
}

void GridLayout::setup()
{
    Layout::setup();
    buildGrid();
}

void GridLayout::buildGrid()
{
    if ( !widgets.count() )
	return;

    // Pixel to cell conversion:
    // By keeping a list of start'n'stop values (x & y) for each widget,
    // it is possible to create a very small grid of cells to represent
    // the widget layout.
    // -----------------------------------------------------------------

    // We need a list of both start and stop values for x- & y-axis
    TQValueVector<int> x( widgets.count()*2 );
    TQValueVector<int> y( widgets.count()*2 );

    // Using push_back would look nicer, but operator[] is much faster
    int index  = 0;
    TQWidget* w = 0;
    for ( w = widgets.first(); w; w = widgets.next() ) {
	TQRect widgetPos = w->geometry();
	x[index]   = widgetPos.left();
	x[index+1] = widgetPos.right();
	y[index]   = widgetPos.top();
	y[index+1] = widgetPos.bottom();
	index += 2;
    }

    qHeapSort(x);
    qHeapSort(y);

    // Remove duplicate x enteries (Remove next, if equal to current)
    if ( !x.empty() ) {
	for (TQValueVector<int>::iterator current = x.begin() ;
	     (current != x.end()) && ((current+1) != x.end()) ; )
	    if ( (*current == *(current+1)) )
		x.erase(current+1);
	    else
		current++;
    }

    // Remove duplicate y enteries (Remove next, if equal to current)
    if ( !y.empty() ) {
	for (TQValueVector<int>::iterator current = y.begin() ;
	     (current != y.end()) && ((current+1) != y.end()) ; )
	    if ( (*current == *(current+1)) )
		y.erase(current+1);
	    else
		current++;
    }

    // Create the smallest grid possible to represent the current layout
    // Since no widget will be placed in the last row and column, we'll
    // skip them to increase speed even further
    delete grid;
    grid = new Grid( (int)(y.size()-1), (int)(x.size()-1) );

    // Mark the cells in the grid that contains a widget
    for ( w = widgets.first(); w; w = widgets.next() ) {
	TQRect c(0,0,0,0), widgetPos = w->geometry();
	// From left til right (not including)
	for (uint cw=0; cw<x.size(); cw++) {
	    if ( x[cw] == widgetPos.left() )
		c.setLeft(cw);
	    if ( x[cw] <  widgetPos.right())
		c.setRight(cw);
	}
	// From top til bottom (not including)
	for (uint ch=0; ch<y.size(); ch++) {
	    if ( y[ch] == widgetPos.top()    )
		c.setTop(ch);
	    if ( y[ch] <  widgetPos.bottom() )
		c.setBottom(ch);
	}
	grid->setCells(c, w); // Mark cellblock
    }
    grid->simplify();
}







Spacer::Spacer( TQWidget *parent, const char *name )
    : TQWidget( parent, name, WMouseNoMask ),
      orient( Vertical ), interactive(TRUE), sh( TQSize(20,20) )
{
    setSizeType( Expanding );
    setAutoMask( TRUE );
}

void Spacer::paintEvent( TQPaintEvent * )
{
    TQPainter p( this );
    p.setPen( TQt::blue );

    if ( orient == Horizontal ) {
	const int dist = 3;
	const int amplitude = TQMIN( 3, height() / 3 );
	const int base = height() / 2;
	int i = 0;
	p.setPen( white );
	for ( i = 0; i < width() / 3 +2; ++i )
	    p.drawLine( i * dist, base - amplitude, i * dist + dist / 2, base + amplitude );
	p.setPen( blue );
	for ( i = 0; i < width() / 3 +2; ++i )
	    p.drawLine( i * dist + dist / 2, base + amplitude, i * dist + dist, base - amplitude );
	p.drawLine( 0, 0, 0, height() );
	p.drawLine( width() - 1, 0, width() - 1, height());
    } else {
	const int dist = 3;
	const int amplitude = TQMIN( 3, width() / 3 );
	const int base = width() / 2;
	int i = 0;
	p.setPen( white );
	for ( i = 0; i < height() / 3 +2; ++i )
	    p.drawLine( base - amplitude, i * dist, base + amplitude,i * dist + dist / 2 );
	p.setPen( blue );
	for ( i = 0; i < height() / 3 +2; ++i )
	    p.drawLine( base + amplitude, i * dist + dist / 2, base - amplitude, i * dist + dist );
	p.drawLine( 0, 0, width(), 0 );
	p.drawLine( 0, height() - 1, width(), height() - 1 );
    }
}

void Spacer::resizeEvent( TQResizeEvent* e)
{
    TQWidget::resizeEvent( e );
    if ( !parentWidget() || WidgetFactory::layoutType( parentWidget() ) == WidgetFactory::NoLayout )
	sh = size();
}

void Spacer::updateMask()
{
    TQRegion r( rect() );
    if ( orient == Horizontal ) {
	const int amplitude = TQMIN( 3, height() / 3 );
	const int base = height() / 2;
	r = r.subtract( TQRect(1, 0, width() - 2, base - amplitude ) );
	r = r.subtract( TQRect(1, base + amplitude, width() - 2, height() - base - amplitude ) );
    } else {
	const int amplitude = TQMIN( 3, width() / 3 );
	const int base = width() / 2;
	r = r.subtract( TQRect(0, 1, base - amplitude, height() - 2 ) );
	r = r.subtract( TQRect( base + amplitude, 1, width() - base - amplitude, height() - 2 ) );
    }
    setMask( r );
}

void Spacer::setSizeType( SizeType t )
{
    TQSizePolicy sizeP;
    if ( orient == Vertical )
	sizeP = TQSizePolicy( TQSizePolicy::Minimum, (TQSizePolicy::SizeType)t );
    else
	sizeP = TQSizePolicy( (TQSizePolicy::SizeType)t, TQSizePolicy::Minimum );
    setSizePolicy( sizeP );
}


Spacer::SizeType Spacer::sizeType() const
{
    if ( orient == Vertical )
	return (SizeType)sizePolicy().verData();
    return (SizeType)sizePolicy().horData();
}

int Spacer::alignment() const
{
    if ( orient == Vertical )
	return AlignHCenter;
    return AlignVCenter;
}

TQSize Spacer::minimumSize() const
{
    TQSize s = TQSize( 20,20 );
    if ( sizeType() == Expanding ) {
	if ( orient == Vertical )
	    s.rheight() = 0;
	else
	    s.rwidth() = 0;
	}
    return s;
}

TQSize Spacer::sizeHint() const
{
    return sh;
}


void Spacer::setSizeHint( const TQSize &s )
{
    sh = s;
    if ( !parentWidget() || WidgetFactory::layoutType( parentWidget() ) == WidgetFactory::NoLayout )
	resize( sizeHint() );
    updateGeometry();
}

TQt::Orientation Spacer::orientation() const
{
    return orient;
}

void Spacer::setOrientation( TQt::Orientation o )
{
    if ( orient == o )
	return;

    SizeType st = sizeType();
    orient = o;
    setSizeType( st );
    if ( interactive ) {
	sh = TQSize( sh.height(), sh.width() );
	if (!parentWidget() || WidgetFactory::layoutType( parentWidget() ) == WidgetFactory::NoLayout )
	    resize( height(), width() );
    }
    updateMask();
    update();
    updateGeometry();
}


void TQDesignerGridLayout::addWidget( TQWidget *w, int row, int col, int align_ )
{
    items.insert( w, Item(row, col, 1, 1) );
    TQGridLayout::addWidget( w, row, col, align_ );
}

void TQDesignerGridLayout::addMultiCellWidget( TQWidget *w, int fromRow, int toRow,
					      int fromCol, int toCol, int align_ )
{
    items.insert( w, Item(fromRow, fromCol, toRow - fromRow + 1, toCol - fromCol +1) );
    TQGridLayout::addMultiCellWidget( w, fromRow, toRow, fromCol, toCol, align_ );
}
