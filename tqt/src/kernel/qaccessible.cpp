/****************************************************************************
**
** Implementation of TQAccessible and TQAccessibleObject classes
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

#include "ntqaccessible.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)

#include "ntqptrdict.h"
#include "ntqmetaobject.h"
#include <private/qpluginmanager_p.h>
#include "ntqapplication.h"
#include <stdlib.h>

/*!
    \class TQAccessible ntqaccessible.h
    \brief The TQAccessible class provides enums and static functions
    relating to accessibility.

    \ingroup misc

    Accessibility clients use implementations of the
    TQAccessibleInterface to read the information an accessible object
    exposes, or to call functions to manipulate the accessible object.

\omit
    TQt provides implementations of the TQAccessibleInterface for most
    widget classes in a plugin. This plugin is located in the \e
    accessibility subdirectory of the plugins installation directory.
    The default installation directory for plugins is \c INSTALL/plugins,
    where \c INSTALL is the directory where TQt was installed.  Calling
    queryAccessibleInterface( TQObject *object, TQAccessibleInterface
    **iface ) will ask all plugins located in this directory for an
    implementation that exposes the information for objects of the
    class of \e object.

    To make a TQt application accessible you have to distribute the
    accessibility plugin provded with TQt together with your
    application. Simply add the plugins created in
    INSTALL/plugins/accessibility to your distribution process. Use \l
    TQApplication::addLibraryPath() to specify a plugin directory for
    your application, and copy the files into an \e accessibility
    subdirectory of one of those plugin directories. TQt's
    accessibility framework will load the plugins upon request and use
    the implementations provided to expose an object's accessibility
    information.
\endomit

    See the \link plugins-howto.html plugin documentation \endlink for
    more details about how to redistribute TQt plugins.
*/

/*!
    \enum TQAccessible::State

    This enum type defines bitflags that can be combined to indicate
    the state of the accessible object. The values are:

    \value Normal
    \value Unavailable
    \value Selected
    \value Focused
    \value Pressed
    \value Checked
    \value Mixed
    \value ReadOnly
    \value HotTracked
    \value Default
    \value Expanded
    \value Collapsed
    \value Busy
    \value Floating
    \value Marqueed
    \value Animated
    \value Invisible
    \value Offscreen
    \value Sizeable
    \value Moveable
    \value SelfVoicing
    \value Focusable
    \value Selectable
    \value Linked
    \value Traversed
    \value MultiSelectable
    \value ExtSelectable
    \value AlertLow
    \value AlertMedium
    \value AlertHigh
    \value Protected
    \value Valid
*/

/*!
    \enum TQAccessible::Event

    This enum type defines event types when the state of the
    accessible object has changed. The event types are:

    \value SoundPlayed
    \value Alert
    \value ForegroundChanged
    \value MenuStart
    \value MenuEnd
    \value PopupMenuStart
    \value PopupMenuEnd
    \value ContextHelpStart
    \value ContextHelpEnd
    \value DragDropStart
    \value DragDropEnd
    \value DialogStart
    \value DialogEnd
    \value ScrollingStart
    \value ScrollingEnd
    \value ObjectCreated
    \value ObjectDestroyed
    \value ObjectShow
    \value ObjectHide
    \value ObjectReorder
    \value Focus
    \value Selection
    \value SelectionAdd
    \value SelectionRemove
    \value SelectionWithin
    \value StateChanged
    \value LocationChanged
    \value NameChanged
    \value DescriptionChanged
    \value ValueChanged
    \value ParentChanged
    \value HelpChanged
    \value DefaultActionChanged
    \value AcceleratorChanged
    \value MenuCommand
*/

/*!
    \enum TQAccessible::Role

    This enum defines a number of roles an accessible object can have.
    The roles are:

    \value NoRole
    \value TitleBar
    \value MenuBar
    \value ScrollBar
    \value Grip
    \value Sound
    \value Cursor
    \value Caret
    \value AlertMessage
    \value Window
    \value Client
    \value PopupMenu
    \value MenuItem
    \value ToolTip
    \value Application
    \value Document
    \value Pane
    \value Chart
    \value Dialog
    \value Border
    \value Grouping
    \value Separator
    \value ToolBar
    \value StatusBar
    \value Table
    \value ColumnHeader
    \value RowHeader
    \value Column
    \value Row
    \value Cell
    \value Link
    \value HelpBalloon
    \value Character
    \value List
    \value ListItem
    \value Outline
    \value OutlineItem
    \value PageTab
    \value PropertyPage
    \value Indicator
    \value Graphic
    \value StaticText
    \value EditableText
    \value PushButton
    \value CheckBox
    \value RadioButton
    \value ComboBox
    \value DropLest
    \value ProgressBar
    \value Dial
    \value HotkeyField
    \value Slider
    \value SpinBox
    \value Diagram
    \value Animation
    \value Equation
    \value ButtonDropDown
    \value ButtonMenu
    \value ButtonDropGrid
    \value Whitespace
    \value PageTabList
    \value Clock
*/

/*!
    \enum TQAccessible::NavDirection

    This enum specifies which item to move to when navigating.

    \value NavUp	    sibling above
    \value NavDown	    sibling below
    \value NavLeft	    left sibling
    \value NavRight	    right sibling
    \value NavNext	    next sibling
    \value NavPrevious	    previous sibling
    \value NavFirstChild    first child
    \value NavLastChild	    last child
    \value NavFocusChild    child with focus
*/

/*!
    \enum TQAccessible::Text

    This enum specifies string information that an accessible object
    returns.

    \value Name		    The name of the object
    \value Description	    A short text describing the object
    \value Value	    The value of the object
    \value Help		    A longer text giving information about how
			    to use the object
    \value DefaultAction    The default method to interact with the object
    \value Accelerator	    The keyboard shortcut that executes the
			    default action
*/

/*!
    \fn static void TQAccessible::updateAccessibility( TQObject *object, int control, Event reason )

    Notifies accessibility clients about a change in \a object's
    accessibility information.

    \a reason specifies the cause of the change, for example,
    ValueChange when the position of a slider has been changed. \a
    control is the ID of the child element that has changed. When \a
    control is 0, the object itself has changed.

    Call this function whenever the state of your accessible object or
    one of it's sub-elements has been changed either programmatically
    (e.g. by calling TQLabel::setText()) or by user interaction.

    If there are no accessibility tools listening to this event, the
    performance penalty for calling this function is minor, but if determining
    the parameters of the call is expensive you can use isActive() to
    avoid unnecessary performance penalties if no client is listening.
*/

static TQPluginManager<TQAccessibleFactoryInterface> *qAccessibleManager = 0;

class AccessibleCache : public TQObject, public TQPtrDict<TQAccessibleInterface>
{
    TQ_OBJECT
public:
    AccessibleCache()
        : TQPtrDict<TQAccessibleInterface>(73)
    {
    }

    void addObject(TQObject *object, TQAccessibleInterface *iface)
    {
        insert(object, iface);
        connect(object, TQ_SIGNAL(destroyed(TQObject*)), this, TQ_SLOT(removeObject(TQObject*)));
    }

public slots:
    void removeObject(TQObject *object);
};

#include "qaccessible.moc"

static AccessibleCache *qAccessibleInterface = 0;
static bool cleanupAdded = FALSE;

static void qAccessibleCleanup()
{
    if ( qAccessibleInterface && qAccessibleInterface->count() && qAccessibleManager )
	qAccessibleManager->setAutoUnload( FALSE );

    delete qAccessibleInterface;
    qAccessibleInterface = 0;
    delete qAccessibleManager;
    qAccessibleManager = 0;
}

#ifdef TQ_WS_MAC
TQObject *TQAccessible::queryAccessibleObject(TQAccessibleInterface *o)
{
    if(qAccessibleInterface) {
	for(TQPtrDictIterator<TQAccessibleInterface> it(*qAccessibleInterface); it.current(); ++it) {
	    if(it.current() == o)
		return (TQObject*)it.currentKey();
	}
    }
    return NULL;
}
#endif

void AccessibleCache::removeObject(TQObject *object)
{
    if (!object)
        return;

    remove(object);
    if (!count()) {
        delete this;
        qAccessibleInterface = 0;
    }
}


/*!
    Sets \a iface to point to the implementation of the
    TQAccessibleInterface for \a object, and returns \c TQS_OK if
    successfull, or sets \a iface to 0 and returns \c TQE_NOCOMPONENT if
    no accessibility implementation for \a object exists.

    The function uses the \link TQObject::className() classname
    \endlink of \a object to find a suitable implementation. If no
    implementation for the object's class is available the function
    tries to find an implementation for the object's parent class.

    This function is called to answer an accessibility client's
    request for object information. You should never need to call this
    function yourself.
*/
TQRESULT TQAccessible::queryAccessibleInterface( TQObject *object, TQAccessibleInterface **iface )
{
    *iface = 0;
    if ( !object )
	return TQE_INVALIDARG;

    if ( qAccessibleInterface ) {
	*iface = qAccessibleInterface->find( object );
	if ( *iface ) {
	    (*iface)->addRef();
	    return TQS_OK;
	}
    }

    if ( !qAccessibleManager ) {
	qAccessibleManager = new TQPluginManager<TQAccessibleFactoryInterface>( IID_QAccessibleFactory, TQApplication::libraryPaths(), "/accessible" );
	if ( !cleanupAdded ) {
	    tqAddPostRoutine( qAccessibleCleanup );
	    cleanupAdded = TRUE;
	}
    }

    TQInterfacePtr<TQAccessibleFactoryInterface> factory = 0;
    TQMetaObject *mo = object->metaObject();
    while ( mo ) {
	qAccessibleManager->queryInterface( mo->className(), &factory );
	if ( factory )
	    break;
	mo = mo->superClass();
    }
    if ( factory )
	return factory->createAccessibleInterface( mo->className(), object, iface );

    return TQE_NOCOMPONENT;
}

/*!
    Returns TRUE if an accessibility implementation has been requested,
    during the runtime of the application, otherwise returns FALSE. 
    
    Use this function to prevent potentially expensive notifications via
    updateAccessibility().

    \omit
    TQListView uses this function to prevent index-lookups for item based
    notifications.
    \endomit
*/
bool TQAccessible::isActive()
{
    return qAccessibleManager != 0;
}

/*!
    \class TQAccessibleInterface ntqaccessible.h
    \brief The TQAccessibleInterface class defines an interface that exposes information about accessible objects.

    \ingroup misc
*/

/*!
    \fn bool TQAccessibleInterface::isValid() const

    Returns TRUE if all the data necessary to use this interface
    implementation is valid (e.g. all pointers are non-null),
    otherwise returns FALSE.
*/

/*!
    \fn int TQAccessibleInterface::childCount() const

    Returns the number of children that belong to this object. A child
    can provide accessibility information on it's own (e.g. a child
    widget), or be a sub-element of this accessible object.

    All objects provide this information.

    \sa queryChild()
*/

/*!
    \fn TQRESULT TQAccessibleInterface::queryChild( int control, TQAccessibleInterface **iface ) const

    Sets \a iface to point to the implementation of the
    TQAccessibleInterface for the child specified with \a control. If
    the child doesn't provide accessibility information on it's own,
    the value of \a iface is set to 0. For those elements, this
    object is responsible for exposing the child's properties.

    All objects provide this information.

    \sa childCount(), queryParent()
*/

/*!
    \fn TQRESULT TQAccessibleInterface::queryParent( TQAccessibleInterface **iface ) const

    Sets \a iface to point to the implementation of the
    TQAccessibleInterface for the parent object, or to 0 if there is
    no such implementation or object.

    All objects provide this information.

    \sa queryChild()
*/

/*!
    \fn int TQAccessibleInterface::controlAt( int x, int y ) const

    Returns the ID of the child that contains the screen coordinates
    (\a x, \a y). This function returns 0 if the point is positioned
    on the object itself. If the tested point is outside the
    boundaries of the object this function returns -1.

    All visual objects provide this information.
*/

/*!
    \fn TQRect TQAccessibleInterface::rect( int control ) const

    Returns the location of the child specified with \a control in
    screen coordinates. This function returns the location of the
    object itself if \a control is 0.

    All visual objects provide this information.
*/

/*!
    \fn int TQAccessibleInterface::navigate( NavDirection direction, int startControl ) const

    This function traverses to another object, or to a sub-element of
    the current object. \a direction specifies in which direction to
    navigate, and \a startControl specifies the start point of the
    navigation, which is either 0 if the navigation starts at the
    object itself, or an ID of one of the object's sub-elements.

    The function returns the ID of the sub-element located in the \a
    direction specified. If there is nothing in the navigated \a
    direction, this function returns -1.

    All objects support navigation.
*/

/*!
    \fn TQString TQAccessibleInterface::text( Text t, int control ) const

    Returns a string property \a t of the child object specified by \a
    control, or the string property of the object itself if \a control
    is 0.

    The \e Name is a string used by clients to identify, find or
    announce an accessible object for the user. All objects must have
    a name that is unique within their container.

    An accessible object's \e Description provides textual information
    about an object's visual appearance. The description is primarily
    used to provide greater context for low-vision or blind users, but
    is also used for context searching or other applications. Not all
    objects have a description. An "OK" button would not need a
    description, but a toolbutton that shows a picture of a smiley
    would.

    The \e Value of an accessible object represents visual information
    contained by the object, e.g. the text in a line edit. Usually,
    the value can be modified by the user. Not all objects have a
    value, e.g. static text labels don't, and some objects have a
    state that already is the value, e.g. toggle buttons.

    The \e Help text provides information about the function and
    usage of an accessible object. Not all objects provide this
    information.

    An accessible object's \e DefaultAction describes the object's
    primary method of manipulation, and should be a verb or a short
    phrase, e.g. "Press" for a button.

    The accelerator is a keyboard shortcut that activates the default
    action of the object. A keyboard shortcut is the underlined
    character in the text of a menu, menu item or control, and is
    either the character itself, or a combination of this character
    and a modifier key like ALT, CTRL or SHIFT. Command controls like
    tool buttons also have shortcut keys and usually display them in
    their tooltip.

    \sa role(), state(), selection()
*/

/*!
    \fn void TQAccessibleInterface::setText( Text t, int control, const TQString &text )

    Sets the text property \a t of the child object \a control to \a
    text. If \a control is 0, the text property of the object itself
    is set.
*/

/*!
    \fn TQAccessible::Role TQAccessibleInterface::role( int control ) const

    Returns the role of the object if \a control is 0, or the role of
    the object's sub-element with ID \a control. The role of an object
    is usually static. All accessible objects have a role.

    \sa text(), state(), selection()
*/

/*!
    \fn TQAccessible::State TQAccessibleInterface::state( int control ) const

    Returns the current state of the object if \a control is 0, or the
    state of the object's sub-element element with ID \a control. All
    objects have a state.

    \sa text(), role(), selection()
*/

/*!
    \fn TQMemArray<int> TQAccessibleInterface::selection() const

    Returns the list of all the element IDs that are selected.

    \sa text(), role(), state()
*/

/*!
    \fn bool TQAccessibleInterface::doDefaultAction( int control )

    Calling this function performs the default action of the child
    object specified by \a control, or the default action of the
    object itself if \a control is 0.
*/

/*!
    \fn bool TQAccessibleInterface::setFocus( int control )

    Gives the focus to the child object specified by \a control, or to
    the object itself if \a control is 0.

    Returns TRUE if the focus could be set; otherwise returns FALSE.
*/

/*!
    \fn bool TQAccessibleInterface::setSelected( int control, bool on, bool extend )

    Sets the selection of the child object with ID \a control to \a
    on. If \a extend is TRUE, all child elements between the focused
    item and the specified child object have their selection set to \a
    on.

    Returns TRUE if the selection could be set; otherwise returns
    FALSE.

    \sa setFocus(), clearSelection()
*/

/*!
    \fn void TQAccessibleInterface::clearSelection()

    Removes any selection from the object.

    \sa setSelected()
*/



/*!
    \class TQAccessibleObject ntqaccessible.h
    \brief The TQAccessibleObject class implements parts of the
    TQAccessibleInterface for TQObjects.

    \ingroup misc

    This class is mainly provided for convenience. All subclasses of
    the TQAccessibleInterface should use this class as the base class.
*/

/*!
    Creates a TQAccessibleObject for \a object.
*/
TQAccessibleObject::TQAccessibleObject( TQObject *object )
: object_(object)
{
    if ( !qAccessibleInterface ) {
	qAccessibleInterface = new AccessibleCache;
	if ( !cleanupAdded ) {
	    tqAddPostRoutine( qAccessibleCleanup );
	    cleanupAdded = TRUE;
	}
    }

    qAccessibleInterface->addObject(object, this);
}

/*!
    Destroys the TQAccessibleObject.

    This only happens when a call to release() decrements the internal
    reference counter to zero.
*/
TQAccessibleObject::~TQAccessibleObject()
{
    if ( qAccessibleInterface ) {
	qAccessibleInterface->removeObject(object_);
	if ( !qAccessibleInterface->count() ) {
	    delete qAccessibleInterface;
	    qAccessibleInterface = 0;
	}
    }
}

/*!
    \reimp
*/
TQRESULT TQAccessibleObject::queryInterface( const TQUuid &uuid, TQUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QAccessible )
	*iface = (TQAccessibleInterface*)this;
    else if ( uuid == IID_QUnknown )
	*iface = (TQUnknownInterface*)this;
    else
	return TQE_NOINTERFACE;

    (*iface)->addRef();
    return TQS_OK;
}

/*!
    Returns the TQObject for which this TQAccessibleInterface
    implementation provides information. Use isValid() to make sure
    the object pointer is safe to use.
*/
TQObject *TQAccessibleObject::object() const
{
#if defined(QT_CHECK_RANGE)
    if ( !isValid() )
	tqWarning( "TQAccessibleInterface is invalid. Crash pending..." );
#endif
    return object_;
}

/*!
    \reimp
*/
bool TQAccessibleObject::isValid() const
{
    return !object_.isNull();
}

#endif
