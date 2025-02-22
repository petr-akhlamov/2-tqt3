/****************************************************************************
**
** Japanese Font utilities for X11
**
** Created : 20010130
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the tools module of the TQt GUI Toolkit.
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

#include "private/qfontcodecs_p.h"

#ifndef TQT_NO_CODECS
#ifndef TQT_NO_BIG_CODECS
#include "ntqjpunicode.h"


// JIS X 0201

TQFontJis0201Codec::TQFontJis0201Codec()
{
}

const char *TQFontJis0201Codec::name() const
{
    return "jisx0201*-0";
}

int TQFontJis0201Codec::mibEnum() const
{
    return 15;
}

unsigned short
TQFontJis0201Codec::characterFromUnicode(const TQString &str, int pos) const
{
    const TQChar *c = str.unicode() + pos;
    if ( c->unicode() < 0x80 )
        return c->unicode();
    if ( c->unicode() >= 0xff61 && c->unicode() <= 0xff9f )
        return c->unicode() - 0xff61 + 0xa1;
    return 0;
}

TQCString TQFontJis0201Codec::fromUnicode(const TQString& uc, int& lenInOut ) const
{
    TQCString rstring( lenInOut+1 );
    uchar *rdata = (uchar *) rstring.data();
    const TQChar *sdata = uc.unicode();
    int i = 0;
    for ( ; i < lenInOut; ++i, ++sdata, ++rdata ) {
	if ( sdata->unicode() < 0x80 ) {
	    *rdata = (uchar) sdata->unicode();
	} else if ( sdata->unicode() >= 0xff61 && sdata->unicode() <= 0xff9f ) {
	    *rdata = (uchar) (sdata->unicode() - 0xff61 + 0xa1);
	} else {
	    *rdata = '?';
	}
    }
    *rdata = 0u;
    return rstring;
}

void TQFontJis0201Codec::fromUnicode(const TQChar *in, unsigned short *out, int length) const
{
    while (length--) {
	if ( in->unicode() < 0x80 ) {
	    *out = (uchar) in->unicode();
	} else if ( in->unicode() >= 0xff61 && in->unicode() <= 0xff9f ) {
	    *out = (uchar) (in->unicode() - 0xff61 + 0xa1);
	} else {
	    *out = 0;
	}

	++in;
	++out;
    }
}

int TQFontJis0201Codec::heuristicNameMatch(const char* hint) const
{
    if ( tqstrncmp( hint, "jisx0201", 8 ) == 0 )
	return 20;
    return -1;

}

bool TQFontJis0201Codec::canEncode( TQChar ch ) const
{
    return ( ch.unicode() < 0x80 || ( ch.unicode() >= 0xff61 &&
				      ch.unicode() <= 0xff9f ) );
}

int TQFontJis0201Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


// JIS X 0208

int TQFontJis0208Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


int TQFontJis0208Codec::heuristicNameMatch(const char *hint) const
{
    if ( tqstrncmp( hint, "jisx0208.", 9 ) == 0 )
	return 20;
    return -1;
}

TQFontJis0208Codec::TQFontJis0208Codec()
{
    convJP = TQJpUnicodeConv::newConverter(TQJpUnicodeConv::Default);
}


TQFontJis0208Codec::~TQFontJis0208Codec()
{
    delete convJP;
    convJP = 0;
}


const char* TQFontJis0208Codec::name() const
{
    return "jisx0208*-0";
}


int TQFontJis0208Codec::mibEnum() const
{
    return 63;
}


TQString TQFontJis0208Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return TQString::null;
}

unsigned short TQFontJis0208Codec::characterFromUnicode(const TQString &str, int pos) const
{
    return convJP->unicodeToJisx0208((str.unicode() + pos)->unicode());
}

TQCString TQFontJis0208Codec::fromUnicode(const TQString& uc, int& lenInOut ) const
{
    TQCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const TQChar *ucp = uc.unicode();

    for ( int i = 0; i < lenInOut; i++ ) {
	TQChar ch(*ucp++);
	ch = convJP->unicodeToJisx0208(ch.unicode());

	if ( ! ch.isNull() ) {
	    *rdata++ = ch.row();
	    *rdata++ = ch.cell();
	} else {
	    //white square
	    *rdata++ = 0x22;
	    *rdata++ = 0x22;
	}
    }

    lenInOut *= 2;

    return result;
}

void TQFontJis0208Codec::fromUnicode(const TQChar *in, unsigned short *out, int length) const
{
    while (length--) {
	*out++ = convJP->unicodeToJisx0208(in->unicode());
	++in;
    }
}


bool TQFontJis0208Codec::canEncode( TQChar ch ) const
{
    return ( convJP->unicodeToJisx0208(ch.unicode()) != 0 );
}




#endif // TQT_NO_BIG_CODECS
#endif // TQT_NO_CODECS
