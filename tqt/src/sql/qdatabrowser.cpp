/****************************************************************************
**
** Implementation of TQDataBrowser class
**
** Created : 2000-11-03
**
** Copyright (C) 2005-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the sql module of the TQt GUI Toolkit.
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

#include "ntqdatabrowser.h"

#ifndef TQT_NO_SQL_VIEW_WIDGETS

#include "ntqsqlform.h"
#include "qsqlmanager_p.h"
#include "ntqsqlresult.h"

class TQDataBrowserPrivate
{
public:
    TQDataBrowserPrivate() : boundaryCheck( TRUE ), readOnly( FALSE ) {}
    TQSqlCursorManager cur;
    TQSqlFormManager frm;
    TQDataManager dat;
    bool boundaryCheck;
    bool readOnly;
};

/*!
    \class TQDataBrowser ntqdatabrowser.h
    \brief The TQDataBrowser class provides data manipulation and
    navigation for data entry forms.

    \ingroup database
    \mainclass
    \module sql

    A high-level API is provided for navigating through data records
    in a cursor, for inserting, updating and deleting records, and for
    refreshing data in the display.

    If you want a read-only form to present database data use
    TQDataView; if you want a table-based presentation of your data use
    TQDataTable.

    A TQDataBrowser is used to associate a dataset with a form in much
    the same way as a TQDataTable associates a dataset with a table.
    Once the data browser has been constructed it can be associated
    with a dataset with setSqlCursor(), and with a form with
    setForm(). Boundary checking, sorting and filtering can be set
    with setBoundaryChecking(), setSort() and setFilter(),
    respectively.

    The insertCurrent() function reads the fields from the default
    form into the default cursor and performs the insert. The
    updateCurrent() and deleteCurrent() functions perform similarly to
    update and delete the current record respectively.

    The user can be asked to confirm all edits with setConfirmEdits().
    For more precise control use setConfirmInsert(),
    setConfirmUpdate(), setConfirmDelete() and setConfirmCancels().
    Use setAutoEdit() to control the behaviour of the form when the
    user edits a record and then navigates.

    The record set is navigated using first(), next(), prev(), last()
    and seek(). The form's display is updated with refresh(). When
    navigation takes place the firstRecordAvailable(),
    lastRecordAvailable(), nextRecordAvailable() and
    prevRecordAvailable() signals are emitted. When the cursor record
    is changed due to navigation the cursorChanged() signal is
    emitted.

    If you want finer control of the insert, update and delete
    processes then you can use the lower level functions to perform
    these operations as described below.

    The form is populated with data from the database with
    readFields(). If the user is allowed to edit, (see setReadOnly()),
    write the form's data back to the cursor's edit buffer with
    writeFields(). You can clear the values in the form with
    clearValues(). Editing is performed as follows:
    \list
    \i \e insert When the data browser enters insertion mode it emits the
    primeInsert() signal which you can connect to, for example to
    pre-populate fields. Call writeFields() to write the user's edits to
    the cursor's edit buffer then call insert() to insert the record
    into the database. The beforeInsert() signal is emitted just before
    the cursor's edit buffer is inserted into the database; connect to
    this for example, to populate fields such as an auto-generated
    primary key.
    \i \e update For updates the primeUpdate() signal is emitted when
    the data browser enters update mode. After calling writeFields()
    call update() to update the record and connect to the beforeUpdate()
    signal to manipulate the user's data before the update takes place.
    \i \e delete For deletion the primeDelete() signal is emitted when
    the data browser enters deletion mode. After calling writeFields()
    call del() to delete the record and connect to the beforeDelete()
    signal, for example to record an audit of the deleted record.
    \endlist

*/

/*!
    \enum TQDataBrowser::Boundary

    This enum describes where the data browser is positioned.

    \value Unknown  the boundary cannot be determined (usually because
    there is no default cursor, or the default cursor is not active).

    \value None  the browser is not positioned on a boundary, but it is
    positioned on a record somewhere in the middle.

    \value BeforeBeginning  the browser is positioned before the
    first available record.

    \value Beginning  the browser is positioned at the first record.

    \value End  the browser is positioned at the last
    record.

    \value AfterEnd  the browser is positioned after the last
    available record.
*/

/*!
    Constructs a data browser which is a child of \a parent, with the
    name \a name and widget flags set to \a fl.
*/

TQDataBrowser::TQDataBrowser( TQWidget *parent, const char *name, WFlags fl )
    : TQWidget( parent, name, fl )
{
    d = new TQDataBrowserPrivate();
    d->dat.setMode( TQSql::Update );
}

/*!
    Destroys the object and frees any allocated resources.
*/

TQDataBrowser::~TQDataBrowser()
{
    delete d;
}


/*!
    Returns an enum indicating the boundary status of the browser.

    This is achieved by moving the default cursor and checking the
    position, however the current default form values will not be
    altered. After checking for the boundary, the cursor is moved back
    to its former position. See \l TQDataBrowser::Boundary.

    \sa Boundary
*/

TQDataBrowser::Boundary TQDataBrowser::boundary()
{
    TQSqlCursor* cur = d->cur.cursor();
    if ( !cur || !cur->isActive() )
	return Unknown;
    if ( !cur->isValid() ) {
	if ( cur->at() == TQSql::BeforeFirst )
	    return BeforeBeginning;
	if ( cur->at() == TQSql::AfterLast )
	    return AfterEnd;
	return Unknown;
    }
    if ( cur->at() == 0 )
	return Beginning;
    int currentAt = cur->at();

    Boundary b = None;
    if ( !cur->prev() )
	b = Beginning;
    else
	cur->seek( currentAt );
    if ( b == None && !cur->next() )
	b = End;
    cur->seek( currentAt );
    return b;
}


/*!
    \property TQDataBrowser::boundaryChecking
    \brief whether boundary checking is active

    When boundary checking is active (the default), signals are
    emitted indicating the current position of the default cursor.

    \sa boundary()
*/

void TQDataBrowser::setBoundaryChecking( bool active )
{
    d->boundaryCheck = active;
}

bool TQDataBrowser::boundaryChecking() const
{
    return d->boundaryCheck;
}

/*!
    \property TQDataBrowser::sort
    \brief the data browser's sort

    The data browser's sort affects the order in which records are
    viewed in the browser. Call refresh() to apply the new sort.

    When retrieving the sort property, a string list is returned in
    the form 'fieldname order', e.g. 'id ASC', 'surname DESC'.

    There is no default sort.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    TQStringList list = myDataBrowser.sort();
    TQStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/

void TQDataBrowser::setSort( const TQStringList& sort )
{
    d->cur.setSort( sort );
}

/*!
    \overload

    Sets the data browser's sort to the TQSqlIndex \a sort. To apply
    the new sort, use refresh().

*/
void TQDataBrowser::setSort( const TQSqlIndex& sort )
{
    d->cur.setSort( sort );
}

TQStringList TQDataBrowser::sort() const
{
    return d->cur.sort();
}


/*!
    \property TQDataBrowser::filter
    \brief the data browser's filter

    The filter applies to the data shown in the browser. Call
    refresh() to apply the new filter. A filter is a string containing
    a SQL WHERE clause without the WHERE keyword, e.g. "id>1000",
    "name LIKE 'A%'", etc.

    There is no default filter.

    \sa sort()
*/

void TQDataBrowser::setFilter( const TQString& filter )
{
    d->cur.setFilter( filter );
}


TQString TQDataBrowser::filter() const
{
    return d->cur.filter();
}


/*!
    Sets the default cursor used by the data browser to \a cursor. If
    \a autoDelete is TRUE (the default is FALSE), the data browser
    takes ownership of the \a cursor pointer, which will be deleted
    when the browser is destroyed, or when setSqlCursor() is called
    again. To activate the \a cursor use refresh(). The cursor's edit
    buffer is used in the default form to browse and edit records.

    \sa sqlCursor() form() setForm()
*/

void TQDataBrowser::setSqlCursor( TQSqlCursor* cursor, bool autoDelete )
{
    if ( !cursor )
	return;
    d->cur.setCursor( cursor, autoDelete );
    d->frm.setRecord( cursor->editBuffer() );
    if ( cursor->isReadOnly() )
	setReadOnly( TRUE );
}


/*!
    Returns the default cursor used for navigation, or 0 if there is
    no default cursor.

    \sa setSqlCursor()
*/

TQSqlCursor* TQDataBrowser::sqlCursor() const
{
    return d->cur.cursor();
}


/*!
    Sets the browser's default form to \a form. The cursor and all
    navigation and data manipulation functions that the browser
    provides become available to the \a form.
*/

void TQDataBrowser::setForm( TQSqlForm* form )
{
    d->frm.setForm( form );
}


/*!
    Returns the data browser's default form or 0 if no form has been
    set.
*/

TQSqlForm* TQDataBrowser::form()
{
    return d->frm.form();
}

/*!
    \property TQDataBrowser::readOnly
    \brief whether the browser is read-only

    The default is FALSE, i.e. data can be edited. If the data browser
    is read-only, no database edits will be allowed.
*/

void TQDataBrowser::setReadOnly( bool active )
{
    d->readOnly = active;
}

bool TQDataBrowser::isReadOnly() const
{
    return d->readOnly;
}

void TQDataBrowser::setConfirmEdits( bool confirm )
{
    d->dat.setConfirmEdits( confirm );
}

/*!
    \property TQDataBrowser::confirmInsert
    \brief whether the data browser confirms insertions

    If this property is TRUE, the browser confirms insertions,
    otherwise insertions happen immediately.

    \sa confirmCancels() confirmEdits() confirmUpdate() confirmDelete() confirmEdit()
*/

void TQDataBrowser::setConfirmInsert( bool confirm )
{
    d->dat.setConfirmInsert( confirm );
}

/*!
    \property TQDataBrowser::confirmUpdate
    \brief whether the browser confirms updates

    If this property is TRUE, the browser confirms updates, otherwise
    updates happen immediately.

    \sa confirmCancels() confirmEdits() confirmInsert() confirmDelete() confirmEdit()
*/

void TQDataBrowser::setConfirmUpdate( bool confirm )
{
    d->dat.setConfirmUpdate( confirm );
}

/*!
    \property TQDataBrowser::confirmDelete
    \brief whether the browser confirms deletions

    If this property is TRUE, the browser confirms deletions,
    otherwise deletions happen immediately.

    \sa confirmCancels() confirmEdits() confirmUpdate() confirmInsert() confirmEdit()
*/

void TQDataBrowser::setConfirmDelete( bool confirm )
{
    d->dat.setConfirmDelete( confirm );
}

/*!
    \property TQDataBrowser::confirmEdits
    \brief whether the browser confirms edits

    If this property is TRUE, the browser confirms all edit operations
    (insertions, updates and deletions), otherwise all edit operations
    happen immediately. Confirmation is achieved by presenting the
    user with a message box -- this behavior can be changed by
    reimplementing the confirmEdit() function,

    \sa confirmEdit() confirmCancels() confirmInsert() confirmUpdate() confirmDelete()
*/

bool TQDataBrowser::confirmEdits() const
{
    return ( d->dat.confirmEdits() );
}

bool TQDataBrowser::confirmInsert() const
{
    return ( d->dat.confirmInsert() );
}

bool TQDataBrowser::confirmUpdate() const
{
    return ( d->dat.confirmUpdate() );
}

bool TQDataBrowser::confirmDelete() const
{
    return ( d->dat.confirmDelete() );
}

/*!
    \property TQDataBrowser::confirmCancels
    \brief whether the browser confirms cancel operations

    If this property is TRUE, all cancels must be confirmed by the
    user through a message box (this behavior can be changed by
    overriding the confirmCancel() function), otherwise all cancels
    occur immediately. The default is FALSE.

    \sa confirmEdits() confirmCancel()
*/

void TQDataBrowser::setConfirmCancels( bool confirm )
{
    d->dat.setConfirmCancels( confirm );
}

bool TQDataBrowser::confirmCancels() const
{
    return d->dat.confirmCancels();
}

/*!
    \property TQDataBrowser::autoEdit
    \brief whether the browser automatically applies edits

    The default value for this property is TRUE. When the user begins
    an insertion or an update on a form there are two possible
    outcomes when they navigate to another record:

    \list
    \i the insert or update is is performed -- this occurs if autoEdit is TRUE
    \i the insert or update is discarded -- this occurs if autoEdit is FALSE
    \endlist
*/

void TQDataBrowser::setAutoEdit( bool autoEdit )
{
    d->dat.setAutoEdit( autoEdit );
}

bool TQDataBrowser::autoEdit() const
{
    return d->dat.autoEdit();
}

/*!
    \fn void TQDataBrowser::firstRecordAvailable( bool available )

    This signal is emitted whenever the position of the cursor
    changes. The \a available parameter indicates whether or not the
    first record in the default cursor is available.
*/

/*!
    \fn void TQDataBrowser::lastRecordAvailable( bool available )

    This signal is emitted whenever the position of the cursor
    changes. The \a available parameter indicates whether or not the
    last record in the default cursor is available.
*/

/*!
    \fn void TQDataBrowser::nextRecordAvailable( bool available )

    This signal is emitted whenever the position of the cursor
    changes. The \a available parameter indicates whether or not the
    next record in the default cursor is available.
*/


/*!
    \fn void TQDataBrowser::prevRecordAvailable( bool available )

    This signal is emitted whenever the position of the cursor
    changes. The \a available parameter indicates whether or not the
    previous record in the default cursor is available.
*/


/*!
    \fn void TQDataBrowser::currentChanged( const TQSqlRecord* record )

    This signal is emitted whenever the current cursor position
    changes. The \a record parameter points to the contents of the
    current cursor's record.
*/


/*!
    \fn void TQDataBrowser::primeInsert( TQSqlRecord* buf )

    This signal is emitted when the data browser enters insertion
    mode. The \a buf parameter points to the record buffer that is to
    be inserted. Connect to this signal to, for example, prime the
    record buffer with default data values, auto-numbered fields etc.
    (Note that TQSqlCursor::primeInsert() is \e not called on the
    default cursor, as this would corrupt values in the form.)

    \sa insert()
*/


/*!
    \fn void TQDataBrowser::primeUpdate( TQSqlRecord* buf )

    This signal is emitted when the data browser enters update mode.
    Note that during navigation (first(), last(), next(), prev()),
    each record that is shown in the default form is primed for
    update. The \a buf parameter points to the record buffer being
    updated. (Note that TQSqlCursor::primeUpdate() is \e not called on
    the default cursor, as this would corrupt values in the form.)
    Connect to this signal in order to, for example, keep track of
    which records have been updated, perhaps for auditing purposes.

    \sa update()
*/

/*!
    \fn void TQDataBrowser::primeDelete( TQSqlRecord* buf )

    This signal is emitted when the data browser enters deletion mode.
    The \a buf parameter points to the record buffer being deleted.
    (Note that TQSqlCursor::primeDelete() is \e not called on the
    default cursor, as this would corrupt values in the form.)
    Connect to this signal in order to, for example, save a copy of
    the deleted record for auditing purposes.

    \sa del()
*/


/*!
    \fn void TQDataBrowser::cursorChanged( TQSqlCursor::Mode mode )

    This signal is emitted whenever the cursor record was changed due
    to navigation. The \a mode parameter is the edit that just took
    place, e.g. Insert, Update or Delete. See \l TQSqlCursor::Mode.
*/


/*!
    Refreshes the data browser's data using the default cursor. The
    browser's current filter and sort are applied if they have been
    set.

    \sa setFilter() setSort()
*/

void TQDataBrowser::refresh()
{
    d->cur.refresh();
}


/*!
    Performs an insert operation on the data browser's cursor. If
    there is no default cursor or no default form, nothing happens.

    If auto-editing is on (see setAutoEdit()), the following happens:

    \list
    \i If the browser is already actively inserting a record,
    the current form's data is inserted into the database.
    \i If the browser is not inserting a record, but the current record
    was changed by the user, the record is updated in the database with
    the current form's data (i.e. with the changes).
    \endlist

    If there is an error handling any of the above auto-edit actions,
    handleError() is called and no insert or update is performed.

    If no error occurred, or auto-editing is not enabled, the data browser
    begins actively inserting a record into the database by performing the
    following actions:

    \list
    \i The default cursor is primed for insert using TQSqlCursor::primeInsert().
    \i The primeInsert() signal is emitted.
    \i The form is updated with the values in the default cursor's.
    edit buffer so that the user can fill in the values to be inserted.
    \endlist

*/

void TQDataBrowser::insert()
{
    TQSqlRecord* buf = d->frm.record();
    TQSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    bool doIns = TRUE;
    TQSql::Confirm conf = TQSql::Yes;
    switch ( d->dat.mode() ) {
    case TQSql::Insert:
	if ( autoEdit() ) {
	    if ( confirmInsert() )
		conf = confirmEdit( TQSql::Insert );
	    switch ( conf ) {
	    case TQSql::Yes:
		insertCurrent();
		break;
	    case TQSql::No:
		break;
	    case TQSql::Cancel:
		doIns = FALSE;
		break;
	    }
	}
	break;
    default:
	if ( autoEdit() && currentEdited() ) {
	    if ( confirmUpdate() )
		conf = confirmEdit( TQSql::Update );
	    switch ( conf ) {
	    case TQSql::Yes:
		updateCurrent();
		break;
	    case TQSql::No:
		break;
	    case TQSql::Cancel:
		doIns = FALSE;
		break;
	    }
	}
	break;
    }
    if ( doIns ) {
	d->dat.setMode( TQSql::Insert );
	sqlCursor()->primeInsert();
	emit primeInsert( d->frm.record() );
	readFields();
    }
}


/*!
    Performs an update operation on the data browser's cursor.

    If there is no default cursor or no default form, nothing happens.
    Otherwise, the following happens:

    If the data browser is actively inserting a record (see insert()),
    that record is inserted into the database using insertCurrent().
    Otherwise, the database is updated with the current form's data
    using updateCurrent(). If there is an error handling either
    action, handleError() is called.
*/

void TQDataBrowser::update()
{
    TQSqlRecord* buf = d->frm.record();
    TQSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    TQSql::Confirm conf = TQSql::Yes;
    switch ( d->dat.mode() ){
    case TQSql::Insert:
	if ( confirmInsert() )
	    conf = confirmEdit( TQSql::Insert );		
	switch ( conf ) {
	    case TQSql::Yes:
		if ( insertCurrent() )
		    d->dat.setMode( TQSql::Update );
	    break;
	    case TQSql::No:
		d->dat.setMode( TQSql::Update );
	    cur->editBuffer( TRUE );
	    readFields();
	    break;
	    case TQSql::Cancel:
	    break;
	}
    break;
    default:
	d->dat.setMode( TQSql::Update );
	if ( confirmUpdate() )
	    conf = confirmEdit( TQSql::Update );
	switch ( conf ) {
	case TQSql::Yes:
	    updateCurrent();
	    break;
	case TQSql::No:
	case TQSql::Cancel:
	    break;
	}
	break;
    }
}


/*!
    Performs a delete operation on the data browser's cursor. If there
    is no default cursor or no default form, nothing happens.

    Otherwise, the following happens:

    The current form's record is deleted from the database, providing
    that the data browser is not in insert mode. If the data browser
    is actively inserting a record (see insert()), the insert action
    is canceled, and the browser navigates to the last valid record
    that was current. If there is an error, handleError() is called.
*/

void TQDataBrowser::del()
{
    TQSqlRecord* buf = d->frm.record();
    TQSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return;
    TQSql::Confirm conf = TQSql::Yes;
    switch ( d->dat.mode() ){
    case TQSql::Insert:
	if ( confirmCancels() )
	    conf = confirmCancel( TQSql::Insert );
	if ( conf == TQSql::Yes ) {
	    cur->editBuffer( TRUE ); /* restore from cursor */
	    readFields();
	    d->dat.setMode( TQSql::Update );
	} else
	    d->dat.setMode( TQSql::Insert );
	break;
    default:
	if ( confirmDelete() )
	    conf = confirmEdit( TQSql::Delete );
	switch ( conf ) {
	case TQSql::Yes:
	    emit primeDelete( buf );
	    deleteCurrent();
	    break;
	case TQSql::No:
	case TQSql::Cancel:
	    break;
	}
	d->dat.setMode( TQSql::Update );
	break;
    }
}

/*!
    Moves the default cursor to the record specified by the index \a i
    and refreshes the default form to display this record. If there is
    no default form or no default cursor, nothing happens. If \a
    relative is TRUE (the default is FALSE), the cursor is moved
    relative to its current position. If the data browser successfully
    navigated to the desired record, the default cursor is primed for
    update and the primeUpdate() signal is emitted.

    If the browser is already positioned on the desired record nothing
    happens.
*/

bool TQDataBrowser::seek( int i, bool relative )
{
    int b = 0;
    TQSqlCursor* cur = d->cur.cursor();
    if ( !cur )
	return FALSE;
    if ( preNav() )
	b = cur->seek( i, relative );
    postNav( b );
    return b;
}

/*!
    Moves the default cursor to the first record and refreshes the
    default form to display this record. If there is no default form
    or no default cursor, nothing happens. If the data browser
    successfully navigated to the first record, the default cursor is
    primed for update and the primeUpdate() signal is emitted.

    If the browser is already positioned on the first record nothing
    happens.

*/

void TQDataBrowser::first()
{
    nav( &TQSqlCursor::first );
}


/*!
    Moves the default cursor to the last record and refreshes the
    default form to display this record. If there is no default form
    or no default cursor, nothing happens. If the data browser
    successfully navigated to the last record, the default cursor is
    primed for update and the primeUpdate() signal is emitted.

    If the browser is already positioned on the last record nothing
    happens.
*/

void TQDataBrowser::last()
{
    nav( &TQSqlCursor::last );
}


/*!
    Moves the default cursor to the next record and refreshes the
    default form to display this record. If there is no default form
    or no default cursor, nothing happens. If the data browser
    successfully navigated to the next record, the default cursor is
    primed for update and the primeUpdate() signal is emitted.

    If the browser is positioned on the last record nothing happens.
*/

void TQDataBrowser::next()
{
    nav( &TQSqlCursor::next );
}


/*!
    Moves the default cursor to the previous record and refreshes the
    default form to display this record. If there is no default form
    or no default cursor, nothing happens. If the data browser
    successfully navigated to the previous record, the default cursor
    is primed for update and the primeUpdate() signal is emitted.

    If the browser is positioned on the first record nothing happens.
*/

void TQDataBrowser::prev()
{
    nav( &TQSqlCursor::prev );
}

/*!
    Reads the fields from the default cursor's edit buffer and
    displays them in the form. If there is no default cursor or no
    default form, nothing happens.
*/

void TQDataBrowser::readFields()
{
    d->frm.readFields();
}


/*!
    Writes the form's data to the default cursor's edit buffer. If
    there is no default cursor or no default form, nothing happens.
*/

void TQDataBrowser::writeFields()
{
    d->frm.writeFields();
}


/*!
    Clears all the values in the form.

    All the edit buffer field values are set to their 'zero state',
    e.g. 0 for numeric fields and "" for string fields. Then the
    widgets are updated using the property map. For example, a
    combobox that is property-mapped to integers would scroll to the
    first item. See the \l TQSqlPropertyMap constructor for the default
    mappings of widgets to properties.
*/

void TQDataBrowser::clearValues()
{
    d->frm.clearValues();
}

/*!
    Reads the fields from the default form into the default cursor and
    performs an insert on the default cursor. If there is no default
    form or no default cursor, nothing happens. If an error occurred
    during the insert into the database, handleError() is called and
    FALSE is returned. If the insert was successfull, the cursor is
    refreshed and relocated to the newly inserted record, the
    cursorChanged() signal is emitted, and TRUE is returned.

    \sa cursorChanged() sqlCursor() form() handleError()
*/

bool TQDataBrowser::insertCurrent()
{
    if ( isReadOnly() )
	return FALSE;
    TQSqlRecord* buf = d->frm.record();
    TQSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    writeFields();
    emit beforeInsert( buf );
    int ar = cur->insert();
    if ( !ar || !cur->isActive() ) {
	handleError( cur->lastError() );
	refresh();
	updateBoundary();
    } else {
	refresh();
	d->cur.findBuffer( cur->primaryIndex() );
	updateBoundary();
	cursorChanged( TQSqlCursor::Insert );
	return TRUE;
    }
    return FALSE;
}


/*!
    Reads the fields from the default form into the default cursor and
    performs an update on the default cursor. If there is no default
    form or no default cursor, nothing happens. If an error occurred
    during the update on the database, handleError() is called and
    FALSE is returned. If the update was successfull, the cursor is
    refreshed and relocated to the updated record, the cursorChanged()
    signal is emitted, and TRUE is returned.

    \sa cursor() form() handleError()
*/

bool TQDataBrowser::updateCurrent()
{
    if ( isReadOnly() )
	return FALSE;
    TQSqlRecord* buf = d->frm.record();
    TQSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    writeFields();
    emit beforeUpdate( buf );
    int ar = cur->update();
    if ( !ar || !cur->isActive() ) {
	handleError( cur->lastError() );
	refresh();
	updateBoundary();
    } else {
	refresh();
	d->cur.findBuffer( cur->primaryIndex() );
	updateBoundary();
	cur->editBuffer( TRUE );
	cursorChanged( TQSqlCursor::Update );
	readFields();
	return TRUE;
    }
    return FALSE;
}


/*!
    Performs a delete on the default cursor using the values from the
    default form and updates the default form. If there is no default
    form or no default cursor, nothing happens. If the deletion was
    successful, the cursor is repositioned to the nearest record and
    TRUE is returned. The nearest record is the next record if there
    is one otherwise the previous record if there is one. If an error
    occurred during the deletion from the database, handleError() is
    called and FALSE is returned.

    \sa cursor() form() handleError()
*/

bool TQDataBrowser::deleteCurrent()
{
    if ( isReadOnly() )
	return FALSE;
    TQSqlRecord* buf = d->frm.record();
    TQSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    writeFields();
    int n = cur->at();
    emit beforeDelete( buf );
    int ar = cur->del();
    if ( ar ) {
	refresh();
	updateBoundary();
	cursorChanged( TQSqlCursor::Delete );
	if ( !cur->seek( n ) )
	    last();
	if ( cur->isValid() ) {
	    cur->editBuffer( TRUE );
	    readFields();
	} else {
	    clearValues();
	}
	return TRUE;
    } else {
	if ( !cur->isActive() ) {
	    handleError( cur->lastError() );
	    refresh();
	    updateBoundary();
	}
    }
    return FALSE;
}


/*!
    Returns TRUE if the form's edit buffer differs from the current
    cursor buffer; otherwise returns FALSE.
*/

bool TQDataBrowser::currentEdited()
{
    TQSqlRecord* buf = d->frm.record();
    TQSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;
    if ( !cur->isActive() || !cur->isValid() )
	return FALSE;
    writeFields();
    for ( uint i = 0; i < cur->count(); ++i ) {
	if ( cur->value(i) != buf->value(i) )
	    return TRUE;
    }
    return FALSE;
}

/*! \internal

  Pre-navigation checking.
*/

bool TQDataBrowser::preNav()
{
    TQSqlRecord* buf = d->frm.record();
    TQSqlCursor* cur = d->cur.cursor();
    if ( !buf || !cur )
	return FALSE;

    if ( !isReadOnly() && autoEdit() && currentEdited() ) {
	bool ok = TRUE;
	TQSql::Confirm conf = TQSql::Yes;
	switch ( d->dat.mode() ){
	case TQSql::Insert:
	    if ( confirmInsert() )
		conf = confirmEdit( TQSql::Insert );
	    switch ( conf ) {
	    case TQSql::Yes:
		ok = insertCurrent();
		d->dat.setMode( TQSql::Update );
		break;
	    case TQSql::No:
		d->dat.setMode( TQSql::Update );
		break;
	    case TQSql::Cancel:
		return FALSE;
	    }
	    break;
	default:
	    if ( confirmUpdate() )
		conf = confirmEdit( TQSql::Update );
	    switch ( conf ) {
	    case TQSql::Yes:
		ok = updateCurrent();
		break;
	    case TQSql::No:
		break;
	    case TQSql::Cancel:
		return FALSE;
	    }
	}
	return ok;
    }
    return TRUE;
}

/*! \internal

  Handles post-navigation according to \a primeUpd.
*/

void TQDataBrowser::postNav( bool primeUpd )
{
    if ( primeUpd ) {
	TQSqlRecord* buf = d->frm.record();
	TQSqlCursor* cur = d->cur.cursor();
	if ( !buf || !cur )
	    return;
	currentChanged( cur );
	cur->primeUpdate();
	emit primeUpdate( buf );
	readFields();
    }
    updateBoundary();
}

/*! \internal

  Navigate default cursor according to \a nav. Handles autoEdit.

*/
void TQDataBrowser::nav( Nav nav )
{
    int b = 0;
    TQSqlCursor* cur = d->cur.cursor();
    if ( !cur )
	return;
    if ( preNav() )
	b = (cur->*nav)();
    postNav( b );
}

/*!
    If boundaryChecking() is TRUE, checks the boundary of the current
    default cursor and emits signals which indicate the position of
    the cursor.
*/

void TQDataBrowser::updateBoundary()
{
    if ( d->boundaryCheck ) {
	Boundary bound = boundary();
	switch ( bound ) {
	case Unknown:
	case None:
	    emit firstRecordAvailable( TRUE );
	    emit prevRecordAvailable( TRUE );
	    emit nextRecordAvailable( TRUE );
	    emit lastRecordAvailable( TRUE );
	    break;

	case BeforeBeginning:
	    emit firstRecordAvailable( FALSE );
	    emit prevRecordAvailable( FALSE );
	    emit nextRecordAvailable( TRUE );
	    emit lastRecordAvailable( TRUE );
	    break;

	case Beginning:
	    emit firstRecordAvailable( FALSE );
	    emit prevRecordAvailable( FALSE );
	    emit nextRecordAvailable( TRUE );
	    emit lastRecordAvailable( TRUE );
	    break;

	case End:
	    emit firstRecordAvailable( TRUE );
	    emit prevRecordAvailable( TRUE );
	    emit nextRecordAvailable( FALSE );
	    emit lastRecordAvailable( FALSE );
	    break;

	case AfterEnd:
	    emit firstRecordAvailable( TRUE );
	    emit prevRecordAvailable( TRUE );
	    emit nextRecordAvailable( FALSE );
	    emit lastRecordAvailable( FALSE );
	    break;
	}
    }
}

/*!
    Virtual function which handles the error \a error. The default
    implementation warns the user with a message box.
*/

void TQDataBrowser::handleError( const TQSqlError& error )
{
    d->dat.handleError( this, error );
}

/*!
    Protected virtual function which returns a confirmation for an
    edit of mode \a m. Derived classes can reimplement this function
    and provide their own confirmation dialog. The default
    implementation uses a message box which prompts the user to
    confirm the edit action.
*/

TQSql::Confirm TQDataBrowser::confirmEdit( TQSql::Op m )
{
    return d->dat.confirmEdit( this, m );
}

/*!
    Protected virtual function which returns a confirmation for
    cancelling an edit mode \a m. Derived classes can reimplement this
    function and provide their own confirmation dialog. The default
    implementation uses a message box which prompts the user to
    confirm the edit action.
*/

TQSql::Confirm  TQDataBrowser::confirmCancel( TQSql::Op m )
{
    return d->dat.confirmCancel( this, m );
}

/*!
    \fn void TQDataBrowser::beforeInsert( TQSqlRecord* buf )

    This signal is emitted just before the cursor's edit buffer is
    inserted into the database. The \a buf parameter points to the
    edit buffer being inserted. You might connect to this signal to
    populate a generated primary key for example.
*/

/*!
    \fn void TQDataBrowser::beforeUpdate( TQSqlRecord* buf )

    This signal is emitted just before the cursor's edit buffer is
    updated in the database. The \a buf parameter points to the edit
    buffer being updated. You might connect to this signal to capture
    some auditing information about the update.
*/

/*!
    \fn void TQDataBrowser::beforeDelete( TQSqlRecord* buf )

    This signal is emitted just before the cursor's edit buffer  is
    deleted from the database. The \a buf parameter points to the edit
    buffer being deleted. You might connect to this signal to capture
    some auditing information about the deletion.
*/

#endif
