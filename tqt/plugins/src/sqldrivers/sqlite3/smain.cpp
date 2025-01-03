/****************************************************************************
**
** Implementation of SQLite driver plugin
**
** Created : 031106
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the TQt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.TQPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid TQt Enterprise Edition licenses may use this
** file in accordance with the TQt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about TQt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for TQPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <ntqsqldriverplugin.h>
#include "../../../../src/sql/drivers/sqlite3/qsql_sqlite3.h"

class TQSQLite3DriverPlugin : public TQSqlDriverPlugin
{
public:
    TQSQLite3DriverPlugin();

    TQSqlDriver* create( const TQString & );
    TQStringList keys() const;
};

TQSQLite3DriverPlugin::TQSQLite3DriverPlugin()
    : TQSqlDriverPlugin()
{
}

TQSqlDriver* TQSQLite3DriverPlugin::create( const TQString &name )
{
    if ( name == "TQSQLITE3" ) {
	TQSQLite3Driver* driver = new TQSQLite3Driver();
	return driver;
    }
    return 0;
}

TQStringList TQSQLite3DriverPlugin::keys() const
{
    TQStringList l;
    l  << "TQSQLITE3";
    return l;
}

Q_EXPORT_PLUGIN( TQSQLite3DriverPlugin )
