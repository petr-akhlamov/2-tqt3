/****************************************************************************
**
** Implementation of TQMessageBox class
**
** Created : 950503
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the dialogs module of the TQt GUI Toolkit.
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

#include "ntqmessagebox.h"

#ifndef TQT_NO_MESSAGEBOX

#include "ntqaccel.h"
#include "ntqlabel.h"
#include "ntqpushbutton.h"
#include "ntqimage.h"
#include "ntqapplication.h"
#include "ntqstyle.h"
#include "ntqobjectlist.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "ntqaccessible.h"
#endif

#ifdef TQ_WS_X11
#include "private/tqttdeintegration_x11_p.h"
#endif

extern bool tqt_use_native_dialogs;

// Internal class - don't touch

class TQMessageBoxLabel : public TQLabel
{
    TQ_OBJECT
public:
    TQMessageBoxLabel( TQWidget* parent ) : TQLabel( parent, "messageBoxText")
    {
	setAlignment( AlignAuto|ExpandTabs );
    }
};
#include "qmessagebox.moc"



// the TQt logo, for aboutTQt
/* XPM */
static const char * const qtlogo_xpm[] = {
/* width height ncolors chars_per_pixel */
"50 50 17 1",
/* colors */
"  c #000000",
". c #495808",
"X c #2A3304",
"o c #242B04",
"O c #030401",
"+ c #9EC011",
"@ c #93B310",
"# c #748E0C",
"$ c #A2C511",
"% c #8BA90E",
"& c #99BA10",
"* c #060701",
"= c #181D02",
"- c #212804",
"; c #61770A",
": c #0B0D01",
"/ c None",
/* pixels */
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$@;.o=::=o.;@$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X*         **X#+$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$#oO*         O  **o#+$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$&.* OO              O*.&$$$$$$$$$$$$$",
"$$$$$$$$$$$$@XOO            * OO    X&$$$$$$$$$$$$",
"$$$$$$$$$$$@XO OO  O  **:::OOO OOO   X@$$$$$$$$$$$",
"$$$$$$$$$$&XO      O-;#@++@%.oOO      X&$$$$$$$$$$",
"$$$$$$$$$$.O  :  *-#+$$$$$$$$+#- : O O*.$$$$$$$$$$",
"$$$$$$$$$#*OO  O*.&$$$$$$$$$$$$+.OOOO **#$$$$$$$$$",
"$$$$$$$$+-OO O *;$$$$$$$$$$$&$$$$;*     o+$$$$$$$$",
"$$$$$$$$#O*  O .+$$$$$$$$$$@X;$$$+.O    *#$$$$$$$$",
"$$$$$$$$X*    -&$$$$$$$$$$@- :;$$$&-    OX$$$$$$$$",
"$$$$$$$@*O  *O#$$$$$$$$$$@oOO**;$$$#    O*%$$$$$$$",
"$$$$$$$;     -+$$$$$$$$$@o O OO ;+$$-O   *;$$$$$$$",
"$$$$$$$.     ;$$$$$$$$$@-OO OO  X&$$;O    .$$$$$$$",
"$$$$$$$o    *#$$$$$$$$@o  O O O-@$$$#O   *o$$$$$$$",
"$$$$$$+=    *@$$$$$$$@o* OO   -@$$$$&:    =$$$$$$$",
"$$$$$$+:    :+$$$$$$@-      *-@$$$$$$:    :+$$$$$$",
"$$$$$$+:    :+$$$$$@o* O    *-@$$$$$$:    :+$$$$$$",
"$$$$$$$=    :@$$$$@o*OOO      -@$$$$@:    =+$$$$$$",
"$$$$$$$-    O%$$$@o* O O    O O-@$$$#*   OX$$$$$$$",
"$$$$$$$. O *O;$$&o O*O* *O      -@$$;    O.$$$$$$$",
"$$$$$$$;*   Oo+$$;O*O:OO--      Oo@+=    *;$$$$$$$",
"$$$$$$$@*  O O#$$$;*OOOo@@-O     Oo;O*  **@$$$$$$$",
"$$$$$$$$X* OOO-+$$$;O o@$$@-    O O     OX$$$$$$$$",
"$$$$$$$$#*  * O.$$$$;X@$$$$@-O O        O#$$$$$$$$",
"$$$$$$$$+oO O OO.+$$+&$$$$$$@-O         o+$$$$$$$$",
"$$$$$$$$$#*    **.&$$$$$$$$$$@o      OO:#$$$$$$$$$",
"$$$$$$$$$+.   O* O-#+$$$$$$$$+;O    OOO:@$$$$$$$$$",
"$$$$$$$$$$&X  *O    -;#@++@#;=O    O    -@$$$$$$$$",
"$$$$$$$$$$$&X O     O*O::::O      OO    Oo@$$$$$$$",
"$$$$$$$$$$$$@XOO                  OO    O*X+$$$$$$",
"$$$$$$$$$$$$$&.*       **  O      ::    *:#$$$$$$$",
"$$$$$$$$$$$$$$$#o*OO       O    Oo#@-OOO=#$$$$$$$$",
"$$$$$$$$$$$$$$$$+#X:* *     O**X#+$$@-*:#$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$%;.o=::=o.#@$$$$$$@X#$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$+++$$$$$$$$$$$+$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"/$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$/",
};


/*!
    \class TQMessageBox
    \brief The TQMessageBox class provides a modal dialog with a short message, an icon, and some buttons.
    \ingroup dialogs
    \mainclass

    Message boxes are used to provide informative messages and to ask
    simple questions.

    TQMessageBox provides a range of different messages, arranged
    roughly along two axes: severity and complexity.

    Severity is
    \table
    \row
    \i \img qmessagebox-quest.png
    \i Question
    \i For message boxes that ask a question as part of normal
    operation. Some style guides recommend using Information for this
    purpose.
    \row
    \i \img qmessagebox-info.png
    \i Information
    \i For message boxes that are part of normal operation.
    \row
    \i \img qmessagebox-warn.png
    \i Warning
    \i For message boxes that tell the user about unusual errors.
    \row
    \i \img qmessagebox-crit.png
    \i Critical
    \i For message boxes that tell the user about critical errors.
    \endtable

    The message box has a different icon for each of the severity levels.

    Complexity is one button (OK) for simple messages, or two or even
    three buttons for questions.

    There are static functions for the most common cases.

    Examples:

    If a program is unable to find a supporting file, but can do perfectly
    well without it:

    \code
    TQMessageBox::information( this, "Application name",
    "Unable to find the user preferences file.\n"
    "The factory default will be used instead." );
    \endcode

    question() is useful for simple yes/no questions:

    \code
    if ( TQFile::exists( filename ) &&
	TQMessageBox::question(
	    this,
	    tr("Overwrite File? -- Application Name"),
	    tr("A file called %1 already exists."
		"Do you want to overwrite it?")
		.arg( filename ),
	    tr("&Yes"), tr("&No"),
	    TQString::null, 0, 1 ) )
	return false;
    \endcode

    warning() can be used to tell the user about unusual errors, or
    errors which can't be easily fixed:

    \code
    switch( TQMessageBox::warning( this, "Application name",
        "Could not connect to the <mumble> server.\n"
        "This program can't function correctly "
        "without the server.\n\n",
        "Retry",
        "Quit", 0, 0, 1 ) ) {
    case 0: // The user clicked the Retry again button or pressed Enter
	// try again
	break;
    case 1: // The user clicked the Quit or pressed Escape
	// exit
	break;
    }
    \endcode

    The text part of all message box messages can be either rich text
    or plain text. If you specify a rich text formatted string, it
    will be rendered using the default stylesheet. See
    TQStyleSheet::defaultSheet() for details. With certain strings that
    contain XML meta characters, the auto-rich text detection may
    fail, interpreting plain text incorrectly as rich text. In these
    rare cases, use TQStyleSheet::convertFromPlainText() to convert
    your plain text string to a visually equivalent rich text string
    or set the text format explicitly with setTextFormat().

    Note that the Microsoft Windows User Interface Guidelines
    recommend using the application name as the window's caption.

    Below are more examples of how to use the static member functions.
    After these examples you will find an overview of the non-static
    member functions.

    Exiting a program is part of its normal operation. If there is
    unsaved data the user probably should be asked if they want to
    save the data. For example:

    \code
    switch( TQMessageBox::information( this, "Application name here",
	"The document contains unsaved changes\n"
	"Do you want to save the changes before exiting?",
	"&Save", "&Discard", "Cancel",
	0,      // Enter == button 0
	2 ) ) { // Escape == button 2
    case 0: // Save clicked or Alt+S pressed or Enter pressed.
	// save
	break;
    case 1: // Discard clicked or Alt+D pressed
	// don't save but exit
	break;
    case 2: // Cancel clicked or Escape pressed
	// don't exit
	break;
    }
    \endcode

    The Escape button cancels the entire exit operation, and pressing
    Enter causes the changes to be saved before the exit occurs.

    Disk full errors are unusual and they certainly can be hard to
    correct. This example uses predefined buttons instead of
    hard-coded button texts:

    \code
    switch( TQMessageBox::warning( this, "Application name here",
	"Could not save the user preferences,\n"
	"because the disk is full. You can delete\n"
	"some files and press Retry, or you can\n"
	"abort the Save Preferences operation.",
	TQMessageBox::Retry | TQMessageBox::Default,
	TQMessageBox::Abort | TQMessageBox::Escape )) {
    case TQMessageBox::Retry: // Retry clicked or Enter pressed
	// try again
	break;
    case TQMessageBox::Abort: // Abort clicked or Escape pressed
	// abort
	break;
    }
    \endcode

    The critical() function should be reserved for critical errors. In
    this example errorDetails is a TQString or const char*, and TQString
    is used to concatenate several strings:

    \code
    TQMessageBox::critical( 0, "Application name here",
	TQString("An internal error occurred. Please ") +
	"call technical support at 1234-56789 and report\n"+
	"these numbers:\n\n" + errorDetails +
	"\n\nApplication will now exit." );
    \endcode

    In this example an OK button is displayed.

    TQMessageBox provides a very simple About box which displays an
    appropriate icon and the string you provide:

    \code
    TQMessageBox::about( this, "About <Application>",
	"<Application> is a <one-paragraph blurb>\n\n"
	"Copyright 1991-2003 Such-and-such. "
	"<License words here.>\n\n"
	"For technical support, call 1234-56789 or see\n"
	"http://www.such-and-such.com/Application/\n" );
    \endcode

    See about() for more information.

    If you want your users to know that the application is built using
    TQt (so they know that you use high quality tools) you might like
    to add an "About TQt" menu option under the Help menu to invoke
    aboutTQt().

    If none of the standard message boxes is suitable, you can create a
    TQMessageBox from scratch and use custom button texts:

    \code
    TQMessageBox mb( "Application name here",
	"Saving the file will overwrite the original file on the disk.\n"
	"Do you really want to save?",
	TQMessageBox::Information,
	TQMessageBox::Yes | TQMessageBox::Default,
	TQMessageBox::No,
	TQMessageBox::Cancel | TQMessageBox::Escape );
    mb.setButtonText( TQMessageBox::Yes, "Save" );
    mb.setButtonText( TQMessageBox::No, "Discard" );
    switch( mb.exec() ) {
    case TQMessageBox::Yes:
	// save and exit
	break;
    case TQMessageBox::No:
	// exit without saving
	break;
    case TQMessageBox::Cancel:
	// don't save and don't exit
	break;
    }
    \endcode

    TQMessageBox defines two enum types: Icon and an unnamed button type.
    Icon defines the \c Question, \c Information, \c Warning, and \c
    Critical icons for each GUI style. It is used by the constructor
    and by the static member functions question(), information(),
    warning() and critical(). A function called standardIcon() gives
    you access to the various icons.

    The button types are:
    \list
    \i Ok - the default for single-button message boxes
    \i Cancel - note that this is \e not automatically Escape
    \i Yes
    \i No
    \i Abort
    \i Retry
    \i Ignore
    \i YesAll
    \i NoAll
    \endlist

    Button types can be combined with two modifiers by using OR, '|':
    \list
    \i Default - makes pressing Enter equivalent to
    clicking this button. Normally used with Ok, Yes or similar.
    \i Escape - makes pressing Escape equivalent to clicking this button.
    Normally used with Abort, Cancel or similar.
    \endlist

    The text(), icon() and iconPixmap() functions provide access to the
    current text and pixmap of the message box. The setText(), setIcon()
    and setIconPixmap() let you change it. The difference between
    setIcon() and setIconPixmap() is that the former accepts a
    TQMessageBox::Icon and can be used to set standard icons, whereas the
    latter accepts a TQPixmap and can be used to set custom icons.

    setButtonText() and buttonText() provide access to the buttons.

    TQMessageBox has no signals or slots.

    <img src=qmsgbox-m.png> <img src=qmsgbox-w.png>

    \sa TQDialog,
	\link http://www.iarchitect.com/errormsg.htm
	    Isys on error messages \endlink,
	\link guibooks.html#fowler GUI Design Handbook: Message Box \endlink
*/


/*!
    \enum TQMessageBox::Icon

    This enum has the following values:

    \value NoIcon the message box does not have any icon.

    \value Question an icon indicating that
    the message is asking a question.

    \value Information an icon indicating that
    the message is nothing out of the ordinary.

    \value Warning an icon indicating that the
    message is a warning, but can be dealt with.

    \value Critical an icon indicating that
    the message represents a critical problem.

*/


struct TQMessageBoxData {
    TQMessageBoxData(TQMessageBox* parent) :
        iconLabel( parent, "icon" )
    {
    }

    int                 numButtons;             // number of buttons
    TQMessageBox::Icon   icon;                   // message box icon
    TQLabel              iconLabel;              // label holding any icon
    int                 button[3];              // button types
    int                 defButton;              // default button (index)
    int                 escButton;              // escape button (index)
    TQSize               buttonSize;             // button size
    TQPushButton        *pb[3];                  // buttons
};

static const int LastButton = TQMessageBox::NoAll;

/*
  NOTE: The table of button texts correspond to the button enum.
*/

#ifndef Q_OS_TEMP
static const char * const mb_texts[] = {
#else
const char * mb_texts[] = {
#endif
    0,
    TQT_TRANSLATE_NOOP("TQMessageBox","OK"),
    TQT_TRANSLATE_NOOP("TQMessageBox","Cancel"),
    TQT_TRANSLATE_NOOP("TQMessageBox","&Yes"),
    TQT_TRANSLATE_NOOP("TQMessageBox","&No"),
    TQT_TRANSLATE_NOOP("TQMessageBox","&Abort"),
    TQT_TRANSLATE_NOOP("TQMessageBox","&Retry"),
    TQT_TRANSLATE_NOOP("TQMessageBox","&Ignore"),
    TQT_TRANSLATE_NOOP("TQMessageBox","Yes to &All"),
    TQT_TRANSLATE_NOOP("TQMessageBox","N&o to All"),
    0
};

/*!
    Constructs a message box with no text and a button with the label
    "OK".

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    The \a parent and \a name arguments are passed to the TQDialog
    constructor.
*/

TQMessageBox::TQMessageBox( TQWidget *parent, const char *name )
    : TQDialog( parent, name, TRUE, WStyle_Customize | WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu )
{
    init( Ok, 0, 0 );
}


/*!
    Constructs a message box with a \a caption, a \a text, an \a icon,
    and up to three buttons.

    The \a icon must be one of the following:
    \list
    \i TQMessageBox::NoIcon
    \i TQMessageBox::Question
    \i TQMessageBox::Information
    \i TQMessageBox::Warning
    \i TQMessageBox::Critical
    \endlist

    Each button, \a button0, \a button1 and \a button2, can have one
    of the following values:
    \list
    \i TQMessageBox::NoButton
    \i TQMessageBox::Ok
    \i TQMessageBox::Cancel
    \i TQMessageBox::Yes
    \i TQMessageBox::No
    \i TQMessageBox::Abort
    \i TQMessageBox::Retry
    \i TQMessageBox::Ignore
    \i TQMessageBox::YesAll
    \i TQMessageBox::NoAll
    \endlist

    Use TQMessageBox::NoButton for the later parameters to have fewer
    than three buttons in your message box. If you don't specify any
    buttons at all, TQMessageBox will provide an Ok button.

    One of the buttons can be OR-ed with the \c TQMessageBox::Default
    flag to make it the default button (clicked when Enter is
    pressed).

    One of the buttons can be OR-ed with the \c TQMessageBox::Escape
    flag to make it the cancel or close button (clicked when Escape is
    pressed).

    Example:
    \code
    TQMessageBox mb( "Application Name",
        "Hardware failure.\n\nDisk error detected\nDo you want to stop?",
        TQMessageBox::Question,
        TQMessageBox::Yes | TQMessageBox::Default,
        TQMessageBox::No  | TQMessageBox::Escape,
        TQMessageBox::NoButton );
    if ( mb.exec() == TQMessageBox::No )
        // try again
    \endcode

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    If \a modal is TRUE the message box is modal; otherwise it
    is modeless.

    The \a parent, \a name, \a modal, and \a f arguments are passed to
    the TQDialog constructor.

    \sa setCaption(), setText(), setIcon()
*/

TQMessageBox::TQMessageBox( const TQString& caption,
			  const TQString &text, Icon icon,
			  int button0, int button1, int button2,
			  TQWidget *parent, const char *name,
			  bool modal, WFlags f )
    : TQDialog( parent, name, modal, f | WStyle_Customize | WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu )
{
    init( button0, button1, button2 );
#ifndef TQT_NO_WIDGET_TOPEXTRA
    setCaption( caption );
#endif
    setText( text );
    setIcon( icon );
}


/*!
    Destroys the message box.
*/

TQMessageBox::~TQMessageBox()
{
    delete mbd;
}

static TQString * translatedTextAboutTQt = 0;

void TQMessageBox::init( int button0, int button1, int button2 )
{
    if ( !translatedTextAboutTQt ) {
	translatedTextAboutTQt = new TQString;

        *translatedTextAboutTQt = tr(
	    "<h3>About TQt</h3>"
	    "<p>This program uses TQt version %1.</p>"
	    "<p>TQt is a C++ toolkit for multiplatform GUI &amp; "
	    "application development.</p>"
	    "<p>TQt provides single-source "
	    "portability across MS&nbsp;Windows, Mac&nbsp;OS&nbsp;X, "
	    "Linux, and all major commercial Unix variants."
	    "<br>TQt is also available for embedded devices.</p>"
	    "<p>TQt is a Trolltech product. "
	    "See <tt>https://trinitydesktop.org/docs/qt3/</tt> "
	    "for more information.</p>"
	    ).arg( TQT_VERSION_STR );

    }
    label = new TQMessageBoxLabel( this );
    TQ_CHECK_PTR( label );

    if ( (button2 && !button1) || (button1 && !button0) ) {
#if defined(QT_CHECK_RANGE)
        tqWarning( "TQMessageBox: Inconsistent button parameters" );
#endif
        button0 = button1 = button2 = 0;
    }
    mbd = new TQMessageBoxData(this);
    TQ_CHECK_PTR( mbd );
    mbd->icon = NoIcon;
    mbd->iconLabel.setPixmap( TQPixmap() );
    mbd->numButtons = 0;
    mbd->button[0] = button0;
    mbd->button[1] = button1;
    mbd->button[2] = button2;
    mbd->defButton = -1;
    mbd->escButton = -1;
    int i;
    for ( i=0; i<3; i++ ) {
        int b = mbd->button[i];
        if ( (b & Default) ) {
            if ( mbd->defButton >= 0 ) {
#if defined(QT_CHECK_RANGE)
                tqWarning( "TQMessageBox: There can be at most one "
                           "default button" );
#endif
            } else {
                mbd->defButton = i;
            }
        }
        if ( (b & Escape) ) {
            if ( mbd->escButton >= 0 ) {
#if defined(QT_CHECK_RANGE)
                tqWarning( "TQMessageBox: There can be at most one "
                           "escape button" );
#endif
            } else {
                mbd->escButton = i;
            }
        }
        b &= ButtonMask;
        if ( b == 0 ) {
            if ( i == 0 )                       // no buttons, add an Ok button
                b = Ok;
        } else if ( b < 0 || b > LastButton ) {
#if defined(QT_CHECK_RANGE)
            tqWarning( "TQMessageBox: Invalid button specifier" );
#endif
            b = Ok;
        } else {
            if ( i > 0 && mbd->button[i-1] == 0 ) {
#if defined(QT_CHECK_RANGE)
                tqWarning( "TQMessageBox: Inconsistent button parameters; "
                           "button %d defined but not button %d",
                           i+1, i );
#endif
                b = 0;
            }
        }
        mbd->button[i] = b;
        if ( b )
            mbd->numButtons++;
    }
    for ( i=0; i<3; i++ ) {
        if ( i >= mbd->numButtons ) {
            mbd->pb[i] = 0;
        } else {
            TQCString buttonName;
            buttonName.sprintf( "button%d", i+1 );
            mbd->pb[i] = new TQPushButton(
                tr(mb_texts[mbd->button[i]]),
                this, buttonName );
	    if ( mbd->defButton == i ) {
                mbd->pb[i]->setDefault( TRUE );
                mbd->pb[i]->setFocus();
            }
            mbd->pb[i]->setAutoDefault( TRUE );
            mbd->pb[i]->setFocusPolicy( TQWidget::StrongFocus );
            connect( mbd->pb[i], TQ_SIGNAL(clicked()), TQ_SLOT(buttonClicked()) );
        }
    }
    resizeButtons();
    reserved1 = reserved2 = 0;
}


int TQMessageBox::indexOf( int button ) const
{
    int index = -1;
    for ( int i=0; i<mbd->numButtons; i++ ) {
        if ( mbd->button[i] == button ) {
            index = i;
            break;
        }
    }
    return index;
}


void TQMessageBox::resizeButtons()
{
    int i;
    TQSize maxSize;
    for ( i=0; i<mbd->numButtons; i++ ) {
        TQSize s = mbd->pb[i]->sizeHint();
        maxSize.setWidth(  TQMAX(maxSize.width(), s.width()) );
        maxSize.setHeight( TQMAX(maxSize.height(),s.height()) );
    }
    mbd->buttonSize = maxSize;
    for ( i=0; i<mbd->numButtons; i++ )
        mbd->pb[i]->resize( maxSize );
}


/*!
    \property TQMessageBox::text
    \brief the message box text to be displayed.

    The text will be interpreted either as a plain text or as rich
    text, depending on the text format setting (\l
    TQMessageBox::textFormat). The default setting is \c AutoText, i.e.
    the message box will try to auto-detect the format of the text.

    The default value of this property is TQString::null.

    \sa textFormat
*/
TQString TQMessageBox::text() const
{
    return label->text();
}


void TQMessageBox::setText( const TQString &text )
{
    label->setText( text );
}


/*!
    \property TQMessageBox::icon
    \brief the message box's icon

    The icon of the message box can be one of the following predefined
    icons:
    \list
    \i TQMessageBox::NoIcon
    \i TQMessageBox::Question
    \i TQMessageBox::Information
    \i TQMessageBox::Warning
    \i TQMessageBox::Critical
    \endlist

    The actual pixmap used for displaying the icon depends on the
    current \link TQWidget::style() GUI style\endlink. You can also set
    a custom pixmap icon using the \l TQMessageBox::iconPixmap
    property. The default icon is TQMessageBox::NoIcon.

    \sa iconPixmap
*/

TQMessageBox::Icon TQMessageBox::icon() const
{
    return mbd->icon;
}

void TQMessageBox::setIcon( Icon icon )
{
    setIconPixmap( standardIcon(icon) );
    mbd->icon = icon;
}

/*!
  \obsolete

  Returns the pixmap used for a standard icon. This
  allows the pixmaps to be used in more complex message boxes.
  \a icon specifies the required icon, e.g. TQMessageBox::Information,
  TQMessageBox::Warning or TQMessageBox::Critical.

  \a style is unused.
*/

TQPixmap TQMessageBox::standardIcon( Icon icon, GUIStyle style)
{
    Q_UNUSED(style);
    return TQMessageBox::standardIcon(icon);
}


/*!
    Returns the pixmap used for a standard icon. This allows the
    pixmaps to be used in more complex message boxes. \a icon
    specifies the required icon, e.g. TQMessageBox::Question,
    TQMessageBox::Information, TQMessageBox::Warning or
    TQMessageBox::Critical.
*/

TQPixmap TQMessageBox::standardIcon( Icon icon )
{
    TQPixmap pm;
    switch ( icon ) {
    case Information:
	pm = TQApplication::style().stylePixmap( TQStyle::SP_MessageBoxInformation );
        break;
    case Warning:
	pm = TQApplication::style().stylePixmap( TQStyle::SP_MessageBoxWarning );
        break;
    case Critical:
	pm = TQApplication::style().stylePixmap( TQStyle::SP_MessageBoxCritical );
        break;
    case Question:
	pm = TQApplication::style().stylePixmap( TQStyle::SP_MessageBoxQuestion );
    default:
	break;
    }
    return pm;
}


/*!
    \property TQMessageBox::iconPixmap
    \brief the current icon

    The icon currently used by the message box. Note that it's often
    hard to draw one pixmap that looks appropriate in both Motif and
    Windows GUI styles; you may want to draw two pixmaps.

    \sa icon
*/

const TQPixmap *TQMessageBox::iconPixmap() const
{
    return mbd->iconLabel.pixmap();
}


void TQMessageBox::setIconPixmap( const TQPixmap &pixmap )
{
    mbd->iconLabel.setPixmap(pixmap);
    mbd->icon = NoIcon;
}


/*!
    Returns the text of the message box button \a button, or
    TQString::null if the message box does not contain the button.

    \sa setButtonText()
*/

TQString TQMessageBox::buttonText( int button ) const
{
    int index = indexOf(button);
    return index >= 0 && mbd->pb[index]
            ? mbd->pb[index]->text()
            : TQString::null;
}


/*!
    Sets the text of the message box button \a button to \a text.
    Setting the text of a button that is not in the message box is
    silently ignored.

    \sa buttonText()
*/

void TQMessageBox::setButtonText( int button, const TQString &text )
{
    int index = indexOf(button);
    if ( index >= 0 && mbd->pb[index] ) {
        mbd->pb[index]->setText( text );
        resizeButtons();
    }
}


/*!
    \internal
    Internal slot to handle button clicks.
*/

void TQMessageBox::buttonClicked()
{
    int reply = 0;
    const TQObject *s = sender();
    for ( int i=0; i<mbd->numButtons; i++ ) {
        if ( mbd->pb[i] == s )
            reply = mbd->button[i];
    }
    done( reply );
}


/*!
    Adjusts the size of the message box to fit the contents just before
    TQDialog::exec() or TQDialog::show() is called.

    This function will not be called if the message box has been explicitly
    resized before showing it.
*/
void TQMessageBox::adjustSize()
{
    if ( !testWState(WState_Polished) )
        polish();
    resizeButtons();
    label->adjustSize();
    TQSize labelSize( label->size() );
    int n  = mbd->numButtons;
    int bw = mbd->buttonSize.width();
    int bh = mbd->buttonSize.height();
    int border = bh / 2 - style().pixelMetric(TQStyle::PM_ButtonDefaultIndicator);
    if ( border <= 0 )
        border = 10;
    int btn_spacing = 7;
    if ( style().styleHint(TQStyle::SH_GUIStyle) == MotifStyle )
        btn_spacing = border;
#ifndef Q_OS_TEMP
    int buttons = mbd->numButtons * bw + (n-1) * btn_spacing;
    int h = bh;
#else
    int visibleButtons = 0;
    for ( int i = 0; i < mbd->numButtons; ++i )
	visibleButtons += mbd->pb[i]->isVisible() ? 1 : 0;
    int buttons = visibleButtons == 0 ? 0 : visibleButtons * bw + (visibleButtons-1) * btn_spacing;
    int h = visibleButtons == 0 ? 0 : bh;
    n = visibleButtons;
#endif
    if ( labelSize.height() )
        h += labelSize.height() + 3*border;
    else
        h += 2*border;
    int lmargin = 0;
    if ( mbd->iconLabel.pixmap() && mbd->iconLabel.pixmap()->width() )  {
        mbd->iconLabel.adjustSize();
        lmargin += mbd->iconLabel.width() + border;
        if ( h < mbd->iconLabel.height() + 3*border + bh && n )
            h = mbd->iconLabel.height() + 3*border + bh;
    }
    int w = TQMAX( buttons, labelSize.width() + lmargin ) + 2*border;
    TQRect screen = TQApplication::desktop()->screenGeometry( pos() );
    if ( w > screen.width() )
        w = screen.width();
    resize( w, h );
    setMinimumSize( size() );
#ifdef TQ_WS_MAC
    setMaximumSize(size());
#endif
}


/*!\reimp
*/
void TQMessageBox::resizeEvent( TQResizeEvent * )
{
    int i;
    int n  = mbd->numButtons;
    int bw = mbd->buttonSize.width();
    int bh = mbd->buttonSize.height();
#ifdef Q_OS_TEMP
    int visibleButtons = 0;
    for ( i = 0; i < n; ++i )
	visibleButtons += mbd->pb[i]->isVisible() ? 1 : 0;
    n  = visibleButtons;
    bw = visibleButtons == 0 ? 0 : bw;
    bh = visibleButtons == 0 ? 0 : bh;
#endif
    int border = bh / 2 - style().pixelMetric(TQStyle::PM_ButtonDefaultIndicator);
    if ( border <= 0 )
        border = 10;
    int btn_spacing = 7;
    if ( style().styleHint(TQStyle::SH_GUIStyle) == MotifStyle )
        btn_spacing = border;
    int lmargin = 0;
    mbd->iconLabel.adjustSize();
    bool rtl = TQApplication::reverseLayout();
    if (rtl)
        mbd->iconLabel.move(width() - border - mbd->iconLabel.width(), border);
    else
        mbd->iconLabel.move(border, border);
    if ( mbd->iconLabel.pixmap() && mbd->iconLabel.pixmap()->width() )
        lmargin += mbd->iconLabel.width() + border;
    label->setGeometry((rtl ? 0 : lmargin) + border,
                       border,
                       width() - lmargin -2*border,
                       height() - 3*border - bh);
    int extra_space = (width() - bw*n - 2*border - (n-1)*btn_spacing);
    if ( style().styleHint(TQStyle::SH_GUIStyle) == MotifStyle )
        for ( i=0; i<n; i++ )
            mbd->pb[rtl ? n - i - 1 : i]->move(border + i*bw + i*btn_spacing + extra_space*(i+1)/(n+1),
                                               height() - border - bh );
    else
        for ( i=0; i<n; i++ )
            mbd->pb[rtl ? n - i - 1 : i]->move(border + i*bw + extra_space/2 + i*btn_spacing,
                                               height() - border - bh );
}


/*!\reimp
*/
void TQMessageBox::keyPressEvent( TQKeyEvent *e )
{
    if ( e->key() == Key_Escape ) {
        if ( mbd->escButton >= 0 ) {
            TQPushButton *pb = mbd->pb[mbd->escButton];
            pb->animateClick();
            e->accept();
            return;
        }
    }
#ifndef TQT_NO_ACCEL
    if ( !( e->state() & AltButton ) ) {
	TQObjectList *list = queryList( "TQPushButton" );
	TQObjectListIt it( *list );
	TQPushButton *pb;
	while ( (pb = (TQPushButton*)it.current()) ) {
	    int key = e->key() & ~(MODIFIER_MASK|UNICODE_ACCEL);
	    int acc = pb->accel() & ~(MODIFIER_MASK|UNICODE_ACCEL);
	    if ( key && acc && acc == key ) {
		delete list;
		emit pb->animateClick();
		return;
	    }
	    ++it;
	}
	delete list;
    }
#endif
    TQDialog::keyPressEvent( e );
}

/*!\reimp
*/
void TQMessageBox::showEvent( TQShowEvent *e )
{
#if defined(QT_ACCESSIBILITY_SUPPORT)
    TQAccessible::updateAccessibility( this, 0, TQAccessible::Alert );
#endif
    TQDialog::showEvent( e );
}

/*!\reimp
*/
void TQMessageBox::closeEvent( TQCloseEvent *e )
{
    TQDialog::closeEvent( e );
    if ( mbd->escButton != -1 )
	setResult( mbd->button[mbd->escButton] );
}

/*****************************************************************************
  Static TQMessageBox functions
 *****************************************************************************/

/*!\fn int TQMessageBox::message( const TQString &,const TQString&,const TQString&,TQWidget*,const char * )
  \obsolete
  Opens a modal message box directly using the specified parameters.

  Please use information(), warning(), question(), or critical() instead.
*/

/*! \fn bool TQMessageBox::query( const TQString &,const TQString&,const TQString&,const TQString&,TQWidget *, const char * )
  \obsolete
  Queries the user using a modal message box with two buttons.
  Note that \a caption is not always shown, it depends on the window manager.

  Please use information(), question(), warning(), or critical() instead.
*/

/*!
    Opens an information message box with the caption \a caption and
    the text \a text. The dialog may have up to three buttons. Each of
    the buttons, \a button0, \a button1 and \a button2 may be set to
    one of the following values:

    \list
    \i TQMessageBox::NoButton
    \i TQMessageBox::Ok
    \i TQMessageBox::Cancel
    \i TQMessageBox::Yes
    \i TQMessageBox::No
    \i TQMessageBox::Abort
    \i TQMessageBox::Retry
    \i TQMessageBox::Ignore
    \i TQMessageBox::YesAll
    \i TQMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to TQMessageBox::NoButton.

    One button can be OR-ed with \c TQMessageBox::Default, and one
    button can be OR-ed with \c TQMessageBox::Escape.

    Returns the identity (TQMessageBox::Ok, or TQMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa question(), warning(), critical()
*/

int TQMessageBox::information( TQWidget *parent,
                              const TQString& caption, const TQString& text,
                              int button0, int button1, int button2 )
{
#if defined(TQ_WS_X11)
    if ( tqt_use_native_dialogs && TQTDEIntegration::enabled())
        return TQTDEIntegration::information( parent, caption, text, button0, button1, button2 );
#endif
    TQMessageBox *mb = new TQMessageBox( caption, text, Information,
                                       button0, button1, button2,
                                       parent, "qt_msgbox_information", TRUE,
				       WDestructiveClose);
    TQ_CHECK_PTR( mb );
    return mb->exec();
}

/*!
    Opens a question message box with the caption \a caption and the
    text \a text. The dialog may have up to three buttons. Each of the
    buttons, \a button0, \a button1 and \a button2 may be set to one
    of the following values:

    \list
    \i TQMessageBox::NoButton
    \i TQMessageBox::Ok
    \i TQMessageBox::Cancel
    \i TQMessageBox::Yes
    \i TQMessageBox::No
    \i TQMessageBox::Abort
    \i TQMessageBox::Retry
    \i TQMessageBox::Ignore
    \i TQMessageBox::YesAll
    \i TQMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to TQMessageBox::NoButton.

    One button can be OR-ed with \c TQMessageBox::Default, and one
    button can be OR-ed with \c TQMessageBox::Escape.

    Returns the identity (TQMessageBox::Yes, or TQMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), warning(), critical()
*/

int TQMessageBox::question( TQWidget *parent,
                           const TQString& caption, const TQString& text,
                           int button0, int button1, int button2 )
{
#if defined(TQ_WS_X11)
    if ( tqt_use_native_dialogs && TQTDEIntegration::enabled())
        return TQTDEIntegration::question( parent, caption, text, button0, button1, button2 );
#endif
    TQMessageBox *mb = new TQMessageBox( caption, text, Question,
                                       button0, button1, button2,
                                       parent, "qt_msgbox_information", TRUE,
				       WDestructiveClose);
    TQ_CHECK_PTR( mb );
    return mb->exec();
}


/*!
    Opens a warning message box with the caption \a caption and the
    text \a text. The dialog may have up to three buttons. Each of the
    button parameters, \a button0, \a button1 and \a button2 may be
    set to one of the following values:

    \list
    \i TQMessageBox::NoButton
    \i TQMessageBox::Ok
    \i TQMessageBox::Cancel
    \i TQMessageBox::Yes
    \i TQMessageBox::No
    \i TQMessageBox::Abort
    \i TQMessageBox::Retry
    \i TQMessageBox::Ignore
    \i TQMessageBox::YesAll
    \i TQMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to TQMessageBox::NoButton.

    One button can be OR-ed with \c TQMessageBox::Default, and one
    button can be OR-ed with \c TQMessageBox::Escape.

    Returns the identity (TQMessageBox::Ok, or TQMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), question(), critical()
*/

int TQMessageBox::warning( TQWidget *parent,
                          const TQString& caption, const TQString& text,
                          int button0, int button1, int button2 )
{
#if defined(TQ_WS_X11)
    if ( tqt_use_native_dialogs && TQTDEIntegration::enabled())
        return TQTDEIntegration::warning( parent, caption, text, button0, button1, button2 );
#endif
    TQMessageBox *mb = new TQMessageBox( caption, text, Warning,
                                       button0, button1, button2,
                                       parent, "qt_msgbox_warning", TRUE,
				       WDestructiveClose);
    TQ_CHECK_PTR( mb );
    return mb->exec();
}


/*!
    Opens a critical message box with the caption \a caption and the
    text \a text. The dialog may have up to three buttons. Each of the
    button parameters, \a button0, \a button1 and \a button2 may be
    set to one of the following values:

    \list
    \i TQMessageBox::NoButton
    \i TQMessageBox::Ok
    \i TQMessageBox::Cancel
    \i TQMessageBox::Yes
    \i TQMessageBox::No
    \i TQMessageBox::Abort
    \i TQMessageBox::Retry
    \i TQMessageBox::Ignore
    \i TQMessageBox::YesAll
    \i TQMessageBox::NoAll
    \endlist

    If you don't want all three buttons, set the last button, or last
    two buttons to TQMessageBox::NoButton.

    One button can be OR-ed with \c TQMessageBox::Default, and one
    button can be OR-ed with \c TQMessageBox::Escape.

    Returns the identity (TQMessageBox::Ok, or TQMessageBox::No, etc.)
    of the button that was clicked.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), question(), warning()
*/

int TQMessageBox::critical( TQWidget *parent,
                           const TQString& caption, const TQString& text,
                           int button0, int button1, int button2 )
{
#if defined(TQ_WS_X11)
    if ( tqt_use_native_dialogs && TQTDEIntegration::enabled())
        return TQTDEIntegration::critical( parent, caption, text, button0, button1, button2 );
#endif
    TQMessageBox *mb = new TQMessageBox( caption, text, Critical,
                                       button0, button1, button2,
                                       parent, "qt_msgbox_critical", TRUE,
				       WDestructiveClose);
    TQ_CHECK_PTR( mb );
    return mb->exec();
}


/*!
    Displays a simple about box with caption \a caption and text \a
    text. The about box's parent is \a parent.

    about() looks for a suitable icon in four locations:
    \list 1
    \i It prefers \link TQWidget::icon() parent->icon() \endlink if that exists.
    \i If not, it tries the top-level widget containing \a parent.
    \i If that fails, it tries the \link
    TQApplication::mainWidget() main widget. \endlink
    \i As a last resort it uses the Information icon.
    \endlist

    The about box has a single button labelled "OK".

    \sa TQWidget::icon() TQApplication::mainWidget()
*/

void TQMessageBox::about( TQWidget *parent, const TQString &caption,
                         const TQString& text )
{
    TQMessageBox *mb = new TQMessageBox( caption, text,
                                       Information,
                                       Ok + Default, 0, 0,
                                       parent, "qt_msgbox_simple_about_box", TRUE,
				       WDestructiveClose);
    TQ_CHECK_PTR( mb );
#ifndef TQT_NO_WIDGET_TOPEXTRA
    const TQPixmap *pm = parent ? parent->icon() : 0;
    if ( pm && !pm->isNull() )
	mb->setIconPixmap( *pm );
    else {
	pm = parent ? parent->topLevelWidget()->icon() : 0;
	if ( pm && !pm->isNull() )
	    mb->setIconPixmap( *pm );
	else {
	    pm = tqApp && tqApp->mainWidget() ? tqApp->mainWidget()->icon() : 0;
	    if ( pm && !pm->isNull() )
		mb->setIconPixmap( *pm );
	}
    }
#endif
    mb->exec();
}


/*! \reimp
*/

void TQMessageBox::styleChanged( TQStyle& )
{
    if ( mbd->icon != NoIcon ) {
        // Reload icon for new style
        setIcon( mbd->icon );
    }
}


static int textBox( TQWidget *parent, TQMessageBox::Icon severity,
                    const TQString& caption, const TQString& text,
                    const TQString& button0Text,
                    const TQString& button1Text,
                    const TQString& button2Text,
                    int defaultButtonNumber,
                    int escapeButtonNumber )
{
    int b[3];
    b[0] = 1;
    b[1] = button1Text.isEmpty() ? 0 : 2;
    b[2] = button2Text.isEmpty() ? 0 : 3;

    int i;
    for( i=0; i<3; i++ ) {
        if ( b[i] && defaultButtonNumber == i )
            b[i] += TQMessageBox::Default;
        if ( b[i] && escapeButtonNumber == i )
            b[i] += TQMessageBox::Escape;
    }

    TQMessageBox *mb = new TQMessageBox( caption, text, severity,
                                       b[0], b[1], b[2],
                                       parent, "qt_msgbox_information", TRUE,
				       TQt::WDestructiveClose);
    TQ_CHECK_PTR( mb );
    if ( button0Text.isEmpty() )
        mb->setButtonText( 1, TQMessageBox::tr(mb_texts[TQMessageBox::Ok]) );
    else
        mb->setButtonText( 1, button0Text );
    if ( b[1] )
        mb->setButtonText( 2, button1Text );
    if ( b[2] )
        mb->setButtonText( 3, button2Text );

#ifndef TQT_NO_CURSOR
    mb->setCursor( TQt::arrowCursor );
#endif
    return mb->exec() - 1;
}


/*!
    \overload

    Displays an information message box with caption \a caption, text
    \a text and one, two or three buttons. Returns the index of the
    button that was clicked (0, 1 or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional.
    \a button2Text is the text of the third button, and is optional.
    \a defaultButtonNumber (0, 1 or 2) is the index of the default
    button; pressing Return or Enter is the same as clicking the
    default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1 or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    Note: If you do not specify an Escape button then if the Escape
    button is pressed then -1 will be returned.  It is suggested that
    you specify an Escape button to prevent this from happening.

    \sa question(), warning(), critical()
*/

int TQMessageBox::information( TQWidget *parent, const TQString &caption,
                              const TQString& text,
                              const TQString& button0Text,
                              const TQString& button1Text,
                              const TQString& button2Text,
                              int defaultButtonNumber,
                              int escapeButtonNumber )
{
#if defined(TQ_WS_X11)
    if ( tqt_use_native_dialogs && TQTDEIntegration::enabled())
        return TQTDEIntegration::information( parent, caption, text,
            button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber );
#endif
    return textBox( parent, Information, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber );
}

/*!
    \overload

    Displays a question message box with caption \a caption, text \a
    text and one, two or three buttons. Returns the index of the
    button that was clicked (0, 1 or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional.
    \a button2Text is the text of the third button, and is optional.
    \a defaultButtonNumber (0, 1 or 2) is the index of the default
    button; pressing Return or Enter is the same as clicking the
    default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1 or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    Note: If you do not specify an Escape button then if the Escape
    button is pressed then -1 will be returned.  It is suggested that
    you specify an Escape button to prevent this from happening.

    \sa information(), warning(), critical()
*/
int TQMessageBox::question( TQWidget *parent, const TQString &caption,
                           const TQString& text,
                           const TQString& button0Text,
                           const TQString& button1Text,
                           const TQString& button2Text,
                           int defaultButtonNumber,
                           int escapeButtonNumber )
{
#if defined(TQ_WS_X11)
    if ( tqt_use_native_dialogs && TQTDEIntegration::enabled())
        return TQTDEIntegration::question( parent, caption, text,
            button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber );
#endif
    return textBox( parent, Question, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber );
}


/*!
    \overload

    Displays a warning message box with a caption, a text, and 1, 2 or
    3 buttons. Returns the number of the button that was clicked (0,
    1, or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional,
    and \a button2Text is the text of the third button, and is
    optional. \a defaultButtonNumber (0, 1 or 2) is the index of the
    default button; pressing Return or Enter is the same as clicking
    the default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1, or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    Note: If you do not specify an Escape button then if the Escape
    button is pressed then -1 will be returned.  It is suggested that
    you specify an Escape button to prevent this from happening.

    \sa information(), question(), critical()
*/

int TQMessageBox::warning( TQWidget *parent, const TQString &caption,
                                 const TQString& text,
                                 const TQString& button0Text,
                                 const TQString& button1Text,
                                 const TQString& button2Text,
                                 int defaultButtonNumber,
                                 int escapeButtonNumber )
{
#if defined(TQ_WS_X11)
    if ( tqt_use_native_dialogs && TQTDEIntegration::enabled())
        return TQTDEIntegration::warning( parent, caption, text,
            button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber );
#endif
    return textBox( parent, Warning, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber );
}


/*!
    \overload

    Displays a critical error message box with a caption, a text, and
    1, 2 or 3 buttons. Returns the number of the button that was
    clicked (0, 1 or 2).

    \a button0Text is the text of the first button, and is optional.
    If \a button0Text is not supplied, "OK" (translated) will be used.
    \a button1Text is the text of the second button, and is optional,
    and \a button2Text is the text of the third button, and is
    optional. \a defaultButtonNumber (0, 1 or 2) is the index of the
    default button; pressing Return or Enter is the same as clicking
    the default button. It defaults to 0 (the first button). \a
    escapeButtonNumber is the index of the Escape button; pressing
    Escape is the same as clicking this button. It defaults to -1;
    supply 0, 1, or 2 to make pressing Escape equivalent to clicking
    the relevant button.

    If \a parent is 0, the message box becomes an application-global
    modal dialog box. If \a parent is a widget, the message box
    becomes modal relative to \a parent.

    \sa information(), question(), warning()
*/

int TQMessageBox::critical( TQWidget *parent, const TQString &caption,
                                  const TQString& text,
                                  const TQString& button0Text,
                                  const TQString& button1Text,
                                  const TQString& button2Text,
                                  int defaultButtonNumber,
                                  int escapeButtonNumber )
{
#if defined(TQ_WS_X11)
    if ( tqt_use_native_dialogs && TQTDEIntegration::enabled())
        return TQTDEIntegration::critical( parent, caption, text,
            button0Text, button1Text, button2Text, defaultButtonNumber, escapeButtonNumber );
#endif
    return textBox( parent, Critical, caption, text,
                    button0Text, button1Text, button2Text,
                    defaultButtonNumber, escapeButtonNumber );
}


/*!
    Displays a simple message box about TQt, with caption \a caption
    and centered over \a parent (if \a parent is not 0). The message
    includes the version number of TQt being used by the application.

    This is useful for inclusion in the Help menu of an application.
    See the examples/menu/menu.cpp example.

    TQApplication provides this functionality as a slot.

    \sa TQApplication::aboutTQt()
*/

void TQMessageBox::aboutTQt( TQWidget *parent, const TQString &caption )
{
    TQMessageBox *mb = new TQMessageBox( parent, "qt_msgbox_about_qt" );
    TQ_CHECK_PTR( mb );
    mb->setWFlags( WDestructiveClose );

#ifndef TQT_NO_WIDGET_TOPEXTRA
    TQString c = caption;
    if ( c.isNull() )
        c = tr( "About TQt" );
    mb->setCaption( c );
#endif
    mb->setText( *translatedTextAboutTQt );
#ifndef TQT_NO_IMAGEIO
    TQPixmap pm;
    TQImage logo( (const char **)qtlogo_xpm);
    if ( tqGray(mb->palette().active().text().rgb()) >
         tqGray(mb->palette().active().base().rgb()) )
    {
        // light on dark, adjust some colors (where's 10?)
        logo.setColor( 0, 0xffffffff );
        logo.setColor( 1, 0xff666666 );
        logo.setColor( 2, 0xffcccc66 );
        logo.setColor( 4, 0xffcccccc );
        logo.setColor( 6, 0xffffff66 );
        logo.setColor( 7, 0xff999999 );
        logo.setColor( 8, 0xff3333ff );
        logo.setColor( 9, 0xffffff33 );
        logo.setColor( 11, 0xffcccc99 );
    }
    if ( pm.convertFromImage( logo ) )
        mb->setIconPixmap( pm );
#endif
    mb->setButtonText( 0, tr("OK") );
    if ( mb->mbd && mb->mbd->pb[0] ) {
        mb->mbd->pb[0]->setAutoDefault( TRUE );
        mb->mbd->pb[0]->setFocusPolicy( TQWidget::StrongFocus );
        mb->mbd->pb[0]->setDefault( TRUE );
        mb->mbd->pb[0]->setFocus();
    }
    mb->exec();
}

/*!
  \reimp
*/

void TQMessageBox::setIcon( const TQPixmap &pix )
{
    //reimplemented to avoid compiler warning.
#ifndef TQT_NO_WIDGET_TOPEXTRA
    TQDialog::setIcon( pix );
#endif
}


/*!
    \property TQMessageBox::textFormat
    \brief the format of the text displayed by the message box

    The current text format used by the message box. See the \l
    TQt::TextFormat enum for an explanation of the possible options.

    The default format is \c AutoText.

    \sa setText()
*/

TQt::TextFormat TQMessageBox::textFormat() const
{
    return label->textFormat();
}


void TQMessageBox::setTextFormat( TQt::TextFormat format )
{
    label->setTextFormat( format );
}


#endif
