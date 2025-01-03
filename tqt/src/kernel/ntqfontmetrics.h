/****************************************************************************
**
** Definition of TQFontMetrics class
**
** Created : 940514
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the TQt GUI Toolkit.
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

#ifndef TQFONTMETRICS_H
#define TQFONTMETRICS_H

#ifndef QT_H
#include "ntqfont.h"
#include "ntqrect.h"
#endif // QT_H

#ifdef Q_WS_QWS
class TQFontEngine;
#endif

class TQTextCodec;
class TQTextParag;

class Q_EXPORT TQFontMetrics
{
public:
    TQFontMetrics( const TQFont & );
    TQFontMetrics( const TQFont &, TQFont::Script );
    TQFontMetrics( const TQFontMetrics & );
    ~TQFontMetrics();

    TQFontMetrics &operator=( const TQFontMetrics & );

    int		ascent()	const;
    int		descent()	const;
    int		height()	const;
    int		leading()	const;
    int		lineSpacing()	const;
    int		minLeftBearing() const;
    int		minRightBearing() const;
    int		maxWidth()	const;

    bool	inFont(TQChar)	const;

    int		leftBearing(TQChar) const;
    int		rightBearing(TQChar) const;
    int		width( const TQString &, int len = -1 ) const;

    int		width( TQChar ) const;
#ifndef QT_NO_COMPAT
    int		width( char c ) const { return width( (TQChar) c ); }
#endif

    int 		charWidth( const TQString &str, int pos ) const;
    TQRect	boundingRect( const TQString &, int len = -1 ) const;
    TQRect	boundingRect( TQChar ) const;
    TQRect	boundingRect( int x, int y, int w, int h, int flags,
			      const TQString& str, int len=-1, int tabstops=0,
			      int *tabarray=0, TQTextParag **intern=0 ) const;
    TQSize	size( int flags,
		      const TQString& str, int len=-1, int tabstops=0,
		      int *tabarray=0, TQTextParag **intern=0 ) const;

    int		underlinePos()	const;
    int         overlinePos()   const;
    int		strikeOutPos()	const;
    int		lineWidth()	const;

private:
    TQFontMetrics( const TQPainter * );

    friend class TQWidget;
    friend class TQPainter;
    friend class TQTextFormat;
#if defined( Q_WS_MAC )
    friend class TQFontPrivate;
#endif

    TQFontPrivate  *d;
    TQPainter      *painter;
    int		   fscript;
};


#endif // TQFONTMETRICS_H
