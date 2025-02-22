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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <ntqdialog.h>

class TQCheckBox;
class TQLineEdit;

class FindDialog : public TQDialog
{
    TQ_OBJECT
public:
    enum { SourceText = 0x1, Translations = 0x2, Comments = 0x4 };

    FindDialog( bool replace, TQWidget *parent = 0, const char *name = 0, bool modal = FALSE );

signals:
    void findNext( const TQString& text, int where, bool matchCase );
    void replace( const TQString& before, const TQString& after, bool matchCase, bool all );

private slots:
    void emitFindNext();
    void emitReplace();
    void emitReplaceAll();

private:
    TQLineEdit *led;
    TQLineEdit *red;
    TQCheckBox *sourceText;
    TQCheckBox *translations;
    TQCheckBox *comments;
    TQCheckBox *matchCase;
};

#endif
