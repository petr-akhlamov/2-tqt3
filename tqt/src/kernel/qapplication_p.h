/****************************************************************************
**
** Definition of some TQt private functions.
**
** Created : 000228
**
** Copyright (C) 2000-2008 Trolltech ASA.  All rights reserved.
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

#ifndef TQAPPLICATION_P_H
#define TQAPPLICATION_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the TQt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp, qcolor_x11.cpp, qfiledialog.cpp
// and many other.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#endif // QT_H

class TQWidget;
class TQObject;
class TQClipboard;
class TQKeyEvent;
class TQMouseEvent;
class TQWheelEvent;

extern TQ_EXPORT bool tqt_modal_state();
extern TQ_EXPORT void tqt_enter_modal( TQWidget* );
extern TQ_EXPORT void tqt_leave_modal( TQWidget* );

extern bool tqt_is_gui_used;
#ifndef TQT_NO_CLIPBOARD
extern TQClipboard *tqt_clipboard;
#endif

#if defined (Q_OS_WIN32) || defined (Q_OS_CYGWIN)
extern TQt::WindowsVersion qt_winver;
const int QT_TABLET_NPACKETQSIZE = 128;
# ifdef Q_OS_TEMP
  extern DWORD qt_cever;
# endif
#elif defined (Q_OS_MAC)
extern TQt::MacintoshVersion qt_macver;
#endif

#if defined (TQ_WS_X11)
extern int qt_ncols_option;
#endif


extern void tqt_dispatchEnterLeave( TQWidget*, TQWidget* );
extern bool tqt_tryModalHelper( TQWidget *, TQWidget ** = 0 );

#endif
