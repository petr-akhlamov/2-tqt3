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

#include <ntqvbox.h>
#include <ntqpainter.h>

#include "previewframe.h"

PreviewFrame::PreviewFrame( TQWidget *parent, const char *name )
    : TQVBox( parent, name )
{
    setMinimumSize(200, 200);
    setFrameStyle(TQFrame::StyledPanel | TQFrame::Sunken);
    setLineWidth(1);

    PreviewWorkspace * w = new PreviewWorkspace( this );
    w->setEraseColor(colorGroup().dark());
    previewWidget = new PreviewWidget( w );
    previewWidget->move( 10, 10 );
}

void PreviewFrame::setPreviewPalette(TQPalette pal)
{
    previewWidget->setPalette(pal);
}

void PreviewWorkspace::paintEvent( TQPaintEvent* )
{
    TQPainter p ( this );
    p.setPen( TQPen( white ) );
    p.drawText ( 0, height() / 2,  width(), height(), AlignHCenter,
		"The moose in the noose\nate the goose who was loose." );
}

