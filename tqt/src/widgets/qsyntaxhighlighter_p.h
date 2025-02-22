/****************************************************************************
**
** Definition of the internal TQSyntaxHighlighterInternal class
**
** Created : 031111
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

#ifndef TQSYNTAXHIGHLIGHTER_P_H
#define TQSYNTAXHIGHLIGHTER_P_H

#ifndef TQT_NO_SYNTAXHIGHLIGHTER
#include "ntqsyntaxhighlighter.h"
#include "private/qrichtext_p.h"

class TQSyntaxHighlighterPrivate
{
public:
    TQSyntaxHighlighterPrivate() :
	currentParagraph( -1 )
	{}

    int currentParagraph;
};

class TQSyntaxHighlighterInternal : public TQTextPreProcessor
{
public:
    TQSyntaxHighlighterInternal( TQSyntaxHighlighter *h ) : highlighter( h ) {}
    void process( TQTextDocument *doc, TQTextParagraph *p, int, bool invalidate ) {
	if ( p->prev() && p->prev()->endState() == -1 )
	    process( doc, p->prev(), 0, FALSE );

	highlighter->para = p;
	TQString text = p->string()->toString();
	int endState = p->prev() ? p->prev()->endState() : -2;
	int oldEndState = p->endState();
	highlighter->d->currentParagraph = p->paragId();
	p->setEndState( highlighter->highlightParagraph( text, endState ) );
	highlighter->d->currentParagraph = -1;
	highlighter->para = 0;

	p->setFirstPreProcess( FALSE );
	TQTextParagraph *op = p;
	p = p->next();
	if ( (!!oldEndState || !!op->endState()) && oldEndState != op->endState() &&
	     invalidate && p && !p->firstPreProcess() && p->endState() != -1 ) {
	    while ( p ) {
		if ( p->endState() == -1 )
		    return;
		p->setEndState( -1 );
		p = p->next();
	    }
	}
    }
    TQTextFormat *format( int ) { return 0; }

private:
    TQSyntaxHighlighter *highlighter;

    friend class TQTextEdit;
};

#endif // TQT_NO_SYNTAXHIGHLIGHTER
#endif // TQSYNTAXHIGHLIGHTER_P_H
