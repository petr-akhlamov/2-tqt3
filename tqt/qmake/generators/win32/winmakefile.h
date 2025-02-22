/****************************************************************************
**
** Definition of Win32MakefileGenerator class.
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of qmake.
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

#ifndef __WINMAKEFILE_H__
#define __WINMAKEFILE_H__

#include "makefile.h"

// In the TQt evaluation and educational version, we have a postfix in the
// library name (e.g. qtmteval301.dll). TQTDLL_POSTFIX is used for this.
// A script modifies these lines when building eval/edu version, so be careful
// when changing them.
#ifndef TQTDLL_POSTFIX
#define TQTDLL_POSTFIX ""
#endif

class Win32MakefileGenerator : public MakefileGenerator
{
protected:
    virtual void writeSubDirs(TQTextStream &t);
    int findHighestVersion(const TQString &dir, const TQString &stem);
    bool findLibraries(const TQString &);
    TQString findDependency(const TQString &);
    virtual bool findLibraries();
    virtual void processPrlFiles();

public:
    Win32MakefileGenerator(TQMakeProject *p);
    ~Win32MakefileGenerator();
};

inline Win32MakefileGenerator::~Win32MakefileGenerator()
{ }

inline bool Win32MakefileGenerator::findLibraries()
{ return findLibraries("QMAKE_LIBS"); }



#endif /* __WINMAKEFILE_H__ */
