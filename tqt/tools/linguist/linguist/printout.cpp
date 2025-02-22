/**********************************************************************
** Copyright (C) 2000-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of TQt Linguist.
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

#include "printout.h"

#include <ntqprinter.h>
#include <ntqfontmetrics.h>

PrintOut::PrintOut( TQPrinter *printer )
    : pr( printer ), pdmetrics( printer ), nextRule( NoRule ), page( 0 )
{
    p.begin( pr );
    TQFont f( "Arial" );
    f8 = f;
    f8.setPointSize( 8 );
    f10 = f;
    f10.setPointSize( 10 );
    p.setFont( f10 );
    fmetrics = new TQFontMetrics( p.fontMetrics() );
    hmargin = 5 * pdmetrics.width() / pdmetrics.widthMM(); // 5 mm
    vmargin = 5 * pdmetrics.height() / pdmetrics.heightMM(); // 5 mm
    hsize = pdmetrics.width() - 2 * hmargin;
    vsize = pdmetrics.height() - vmargin;
    dateTime = TQDateTime::currentDateTime();
    breakPage();
    vsize -= voffset;
    cp = Paragraph( TQPoint(hmargin, voffset) );
}

PrintOut::~PrintOut()
{
    flushLine();
    delete fmetrics;
    p.end();
}

void PrintOut::setRule( Rule rule )
{
    if ( (int) nextRule < (int) rule )
	nextRule = rule;
}

void PrintOut::setGuide( const TQString& guide )
{
    g = guide;
}

void PrintOut::vskip()
{
    if ( !firstParagraph )
	voffset += 14;
}

void PrintOut::flushLine( bool /* mayBreak */ )
{
    if ( voffset + cp.rect.height() > vsize )
	breakPage();
    else if ( !firstParagraph )
	drawRule( nextRule );

    for ( int i = 0; i < (int) cp.boxes.count(); i++ ) {
	Box b = cp.boxes[i];
	b.rect.moveBy( 0, voffset );
	TQRect r = b.rect;
	p.setFont( b.font );
	p.drawText( r, b.align, b.text );
    }
    voffset += cp.rect.height();

    nextRule = NoRule;
    cp = Paragraph( TQPoint(hmargin, voffset) );
    firstParagraph = FALSE;
}

void PrintOut::addBox( int percent, const TQString& text, Style style,
		       int halign )
{
    int align = halign | TQt::AlignTop;
    TQFont f = f10;
    if ( style == Strong )
	f.setBold( TRUE );
    else if ( style == Emphasis )
	f.setItalic( TRUE );
    int wd = hsize * percent / 100;
    TQRect r( cp.rect.x() + cp.rect.width(), 0, wd, vsize );
    int ht = p.boundingRect( r, align, text ).height();

    Box b( r, text, f, align );
    cp.boxes.append( b );
    cp.rect.setSize( TQSize(cp.rect.width() + wd, TQMAX(cp.rect.height(), ht)) );
}

void PrintOut::breakPage()
{
    static const int LeftAlign = TQt::AlignLeft | TQt::AlignTop;
    static const int RightAlign = TQt::AlignRight | TQt::AlignTop;
    TQRect r1, r2;
    int h1 = 0;
    int h2 = 0;

    if ( page++ > 0 )
	pr->newPage();
    voffset = 0;

    p.setFont( f10 );
    r1 = TQRect( hmargin, voffset, 3 * hsize / 4, vsize );
    r2 = TQRect( r1.x() + r1.width(), voffset, hsize - r1.width(), vsize );
    h1 = p.boundingRect( r1, LeftAlign, pr->docName() ).height();
    p.drawText( r1, LeftAlign, pr->docName() );
    h2 = p.boundingRect( r2, RightAlign, TQString::number(page) ).height();
    p.drawText( r2, RightAlign, TQString::number(page) );
    voffset += TQMAX( h1, h2 );

    r1 = TQRect( hmargin, voffset, hsize / 2, LeftAlign );
    p.setFont( f8 );
    h1 = p.boundingRect( r1, LeftAlign, dateTime.toString() ).height();
    p.drawText( r1, LeftAlign, dateTime.toString() );
    p.setFont( f10 );
    voffset += TQMAX( h1, h2 );

    voffset += 4;
    p.drawLine( TQPoint(hmargin, voffset), TQPoint(hmargin + hsize, voffset) );
    voffset += 14;
    firstParagraph = TRUE;
}

void PrintOut::drawRule( Rule rule )
{
    TQPen pen;

    switch ( rule ) {
    case NoRule:
	voffset += 5;
	break;
    case ThinRule:
	pen.setColor( TQColor(192, 192, 192) );
	pen.setStyle( TQPen::DotLine );
	pen.setWidth( 0 );
	p.setPen( pen );
	voffset += 5;
	p.drawLine( TQPoint(hmargin, voffset),
		    TQPoint(hmargin + hsize, voffset) );
	p.setPen( TQPen() );
	voffset += 2;
	break;
    case ThickRule:
	voffset += 7;
	p.drawLine( TQPoint(hmargin, voffset),
		    TQPoint(hmargin + hsize, voffset) );
	voffset += 4;
    }
}
