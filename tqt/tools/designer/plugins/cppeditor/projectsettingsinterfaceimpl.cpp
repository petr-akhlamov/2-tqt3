/**********************************************************************
**
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

#include "projectsettingsinterfaceimpl.h"
#include "projectsettings.h"

ProjectSettingsInterfaceImpl::ProjectSettingsInterfaceImpl( TQUnknownInterface *outer )
    : parent( outer ),
      ref( 0 ),
      settingsTab( 0 )
{
}

ulong ProjectSettingsInterfaceImpl::addRef()
{
    return parent ? parent->addRef() : ref++;
}

ulong ProjectSettingsInterfaceImpl::release()
{
    if ( parent )
	return parent->release();
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

ProjectSettingsInterface::ProjectSettings *ProjectSettingsInterfaceImpl::projectSetting()
{
    if ( !settingsTab ) {
	settingsTab = new CppProjectSettings( 0 );
	settingsTab->hide();
    }
    ProjectSettings *pf = 0;
    pf = new ProjectSettings;
    pf->tab = settingsTab;
    pf->title = "C++";
    pf->receiver = pf->tab;
    pf->init_slot = TQ_SLOT( reInit( TQUnknownInterface * ) );
    pf->accept_slot = TQ_SLOT( save( TQUnknownInterface * ) );
    return pf;
}

TQStringList ProjectSettingsInterfaceImpl::projectSettings() const
{
    return TQStringList();
}

void ProjectSettingsInterfaceImpl::connectTo( TQUnknownInterface * )
{
}

void ProjectSettingsInterfaceImpl::deleteProjectSettingsObject( ProjectSettings *pf )
{
    delete pf;
}

TQRESULT ProjectSettingsInterfaceImpl::queryInterface( const TQUuid &uuid, TQUnknownInterface **iface )
{
    if ( parent )
	return parent->queryInterface( uuid, iface );

    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = (TQUnknownInterface*)this;
    else if ( uuid == IID_ProjectSettings )
	*iface = (ProjectSettingsInterface*)this;
    else
	return TQE_NOINTERFACE;

    (*iface)->addRef();
    return TQS_OK;
}
