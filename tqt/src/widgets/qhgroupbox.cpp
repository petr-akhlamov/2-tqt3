/****************************************************************************
**
** Implementation of TQHGroupBox class
**
** Created : 990602
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

#include "ntqhgroupbox.h"
#ifndef TQT_NO_HGROUPBOX

/*!
    \class TQHGroupBox ntqhgroupbox.h

    \brief The TQHGroupBox widget organizes widgets in a group with one
    horizontal row.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance

    TQHGroupBox is a convenience class that offers a thin layer on top
    of TQGroupBox. Think of it as a TQHBox that offers a frame with a
    title.

    \img qgroupboxes.png Group Boxes

    \sa TQVGroupBox
*/

/*!
    Constructs a horizontal group box with no title.

    The \a parent and \a name arguments are passed to the TQWidget
    constructor.
*/
TQHGroupBox::TQHGroupBox( TQWidget *parent, const char *name )
    : TQGroupBox( 1, Vertical /* sic! */, parent, name )
{
}

/*!
    Constructs a horizontal group box with the title \a title.

    The \a parent and \a name arguments are passed to the TQWidget
    constructor.
*/

TQHGroupBox::TQHGroupBox( const TQString &title, TQWidget *parent,
			    const char *name )
    : TQGroupBox( 1, Vertical /* sic! */, title, parent, name )
{
}

/*!
    Destroys the horizontal group box, deleting its child widgets.
*/
TQHGroupBox::~TQHGroupBox()
{
}
#endif
