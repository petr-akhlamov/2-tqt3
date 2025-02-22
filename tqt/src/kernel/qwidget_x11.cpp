/****************************************************************************
**
** Implementation of TQWidget and TQWindow classes for X11
**
** Created : 931031
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
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

#include "ntqapplication.h"
#include "qapplication_p.h"
#include "ntqnamespace.h"
#include "ntqpaintdevicemetrics.h"
#include "ntqpainter.h"
#include "ntqbitmap.h"
#include "ntqimage.h"
#include "ntqobjectlist.h"
#include "ntqlayout.h"
#include "ntqtextcodec.h"
#include "ntqdatetime.h"
#include "ntqcursor.h"
#include "qt_x11_p.h"
#include <stdlib.h>

// NOT REVISED

// defined in qapplication_x11.cpp
extern Window qt_x11_wm_client_leader;
extern void qt_x11_create_wm_client_leader();

// defined in qapplication_x11.cpp
void qt_insert_sip( TQWidget*, int, int );
int  qt_sip_count( TQWidget* );
bool qt_wstate_iconified( WId );
void qt_updated_rootinfo();

#ifndef TQT_NO_IM
#include "ntqinputcontext.h"
#include "ntqinputcontextfactory.h"
#endif

// Paint event clipping magic
extern void qt_set_paintevent_clipping( TQPaintDevice* dev, const TQRegion& region);
extern void qt_clear_paintevent_clipping();

extern bool qt_dnd_enable( TQWidget* w, bool on );
extern bool qt_nograb();

// defined in qapplication_x11.cpp
extern void qt_deferred_map_add( TQWidget* );
extern void qt_deferred_map_take( TQWidget* );
extern bool qt_deferred_map_contains(TQWidget *);

static TQWidget *mouseGrb    = 0;
static TQWidget *keyboardGrb = 0;

// defined in qapplication_x11.cpp
extern Time tqt_x_time;
extern Time tqt_x_user_time;

#ifndef TQT_NO_XSYNC
extern Atom qt_net_wm_sync_request_counter;
extern Atom qt_net_wm_sync_request;
extern bool qt_use_xsync;
#endif

// defined in qfont_x11.cpp
extern bool tqt_has_xft;

int qt_x11_create_desktop_on_screen = -1;

/*****************************************************************************
  TQWidget member functions
 *****************************************************************************/

// defined in qapplication_x11.cpp
extern Atom tqt_wm_state;
extern Atom qt_wm_change_state;
extern Atom tqt_wm_delete_window;
extern Atom tqt_wm_take_focus;
extern Atom qt_wm_client_leader;
extern Atom tqt_window_role;
extern Atom tqt_sm_client_id;
extern Atom qt_utf8_string;
extern Atom qt_net_wm_context_help;
extern Atom qt_net_wm_ping;
extern Atom qt_xa_motif_wm_hints;
extern Atom qt_net_wm_name;
extern Atom qt_net_wm_icon_name;
extern Atom qt_net_wm_state;
extern Atom qt_net_wm_state_modal;
extern Atom qt_net_wm_state_max_v;
extern Atom qt_net_wm_state_max_h;
extern Atom qt_net_wm_state_fullscreen;
extern Atom qt_net_wm_state_above;
extern Atom qt_net_wm_state_stays_on_top;
extern Atom qt_net_wm_action;
extern Atom qt_net_wm_action_move;
extern Atom qt_net_wm_action_resize;
extern Atom qt_net_wm_action_minimize;
extern Atom qt_net_wm_action_shade;
extern Atom qt_net_wm_action_stick;
extern Atom qt_net_wm_action_max_h;
extern Atom qt_net_wm_action_max_v;
extern Atom qt_net_wm_action_fullscreen;
extern Atom qt_net_wm_action_change_desktop;
extern Atom qt_net_wm_action_close;
extern Atom qt_net_wm_action_above;
extern Atom qt_net_wm_action_below;
extern Atom qt_net_wm_window_type;
extern Atom qt_net_wm_window_type_normal;
extern Atom qt_net_wm_window_type_dialog;
extern Atom qt_net_wm_window_type_toolbar;
extern Atom qt_net_wm_window_type_menu;
extern Atom qt_net_wm_window_type_utility;
extern Atom qt_net_wm_window_type_splash;
extern Atom qt_net_wm_window_type_override;
extern Atom qt_net_wm_window_type_dropdown_menu;
extern Atom qt_net_wm_window_type_popup_menu;
extern Atom qt_net_wm_window_type_combo;
extern Atom qt_net_wm_window_type_dnd;
extern Atom qt_net_wm_window_type_tooltip;
extern Atom qt_net_wm_pid;
extern Atom qt_net_wm_user_time;
extern Atom qt_enlightenment_desktop;
extern Atom qt_net_virtual_roots;
extern bool qt_broken_wm;

// defined in qapplication_x11.cpp
extern bool qt_net_supports(Atom);
extern unsigned long *qt_net_virtual_root_list;

#if defined (QT_TABLET_SUPPORT)
extern XDevice *devStylus;
extern XDevice *devEraser;
extern XEventClass event_list_stylus[7];
extern XEventClass event_list_eraser[7];
extern int qt_curr_events_stylus;
extern int qt_curr_events_eraser;
#endif

const uint stdWidgetEventMask =			// X event mask
	(uint)(
	    KeyPressMask | KeyReleaseMask |
	    ButtonPressMask | ButtonReleaseMask |
	    KeymapStateMask |
	    ButtonMotionMask |
	    EnterWindowMask | LeaveWindowMask |
	    FocusChangeMask |
	    ExposureMask |
	    PropertyChangeMask |
	    StructureNotifyMask
	);

const uint stdDesktopEventMask =			// X event mask
       (uint)(
           KeymapStateMask |
	   EnterWindowMask | LeaveWindowMask |
	   PropertyChangeMask
       );


/*
  The qt_ functions below are implemented in qwidgetcreate_x11.cpp.
*/

Window qt_XCreateWindow( const TQWidget *creator,
			 Display *display, Window parent,
			 int x, int y, uint w, uint h,
			 int borderwidth, int depth,
			 uint windowclass, Visual *visual,
			 ulong valuemask, XSetWindowAttributes *attributes );
Window qt_XCreateSimpleWindow( const TQWidget *creator,
			       Display *display, Window parent,
			       int x, int y, uint w, uint h, int borderwidth,
			       ulong border, ulong background );
void qt_XDestroyWindow( const TQWidget *destroyer,
			Display *display, Window window );

TQ_EXPORT void tqt_x11_enforce_cursor( TQWidget * w )
{
    if ( w->testWState( TQt::WState_OwnCursor ) ) {
	TQCursor * oc = TQApplication::overrideCursor();
	if ( oc ) {
	    XDefineCursor( w->x11Display(), w->winId(), oc->handle() );
	} else if ( w->isEnabled() ) {
	    XDefineCursor( w->x11Display(), w->winId(), w->cursor().handle() );
	} else {
	    // enforce the windows behavior of clearing the cursor on
	    // disabled widgets
	    XDefineCursor( w->x11Display(), w->winId(), None );
	}
    } else {
	XDefineCursor( w->x11Display(), w->winId(), None );
    }
}

TQ_EXPORT void tqt_wait_for_window_manager( TQWidget* w )
{
    TQApplication::flushX();
    XEvent ev;
    TQTime t;
    t.start();
    while ( !XCheckTypedWindowEvent( w->x11Display(), w->winId(), ReparentNotify, &ev ) ) {
	if ( XCheckTypedWindowEvent( w->x11Display(), w->winId(), MapNotify, &ev ) )
	    break;
	if ( t.elapsed() > 500 )
	    return; // give up, no event available
	tqApp->syncX(); // non-busy wait
    }
    tqApp->x11ProcessEvent( &ev );
    if ( XCheckTypedWindowEvent( w->x11Display(), w->winId(), ConfigureNotify, &ev ) )
	tqApp->x11ProcessEvent( &ev );
}

static void qt_net_change_wm_state(const TQWidget* w, bool set, Atom one, Atom two = 0)
{
    if (w->isShown()) {
	// managed by WM
        XEvent e;
        e.xclient.type = ClientMessage;
        e.xclient.message_type = qt_net_wm_state;
        e.xclient.display = w->x11Display();
        e.xclient.window = w->winId();
        e.xclient.format = 32;
        e.xclient.data.l[ 0 ] = set ? 1 : 0;
        e.xclient.data.l[ 1 ] = one;
        e.xclient.data.l[ 2 ] = two;
        e.xclient.data.l[ 3 ] = 0;
        e.xclient.data.l[ 4 ] = 0;
        XSendEvent(w->x11Display(), RootWindow(w->x11Display(), w->x11Screen()),
		   False, (SubstructureNotifyMask|SubstructureRedirectMask), &e);
    } else {
        Atom ret;
	int format = 0, status;
	unsigned char *data = 0;
	unsigned long nitems = 0, after = 0;
	Atom *old_states = 0;
	status = XGetWindowProperty(w->x11Display(), w->winId(),
				    qt_net_wm_state, 0, 1024, False,
				    XA_ATOM, &ret, &format, &nitems,
				    &after, &data);
	if (status == Success && ret == XA_ATOM && format == 32 && nitems > 0)
	    old_states = (Atom *) data;
	else
	    nitems = 0;

	Atom *new_states = new Atom[nitems + 2];
	int i, j = 0;
	for (i = 0; i < (int)nitems; ++i) {
	    if (old_states[i] && old_states[i] != one && old_states[i] != two)
		new_states[j++] = old_states[i];
	}

	if (set) {
	    if (one) new_states[j++] = one;
	    if (two) new_states[j++] = two;
	}

	if (j)
	    XChangeProperty(w->x11Display(), w->winId(), qt_net_wm_state, XA_ATOM, 32,
			    PropModeReplace, (uchar *) new_states, j);
	else
	    XDeleteProperty(w->x11Display(), w->winId(), qt_net_wm_state);

	delete [] new_states;
	if (data) XFree(data);
    }
}

/*!
    Creates a new widget window if \a window is 0, otherwise sets the
    widget's window to \a window.

    Initializes the window (sets the geometry etc.) if \a
    initializeWindow is TRUE. If \a initializeWindow is FALSE, no
    initialization is performed. This parameter only makes sense if \a
    window is a valid window.

    Destroys the old window if \a destroyOldWindow is TRUE. If \a
    destroyOldWindow is FALSE, you are responsible for destroying the
    window yourself (using platform native code).

    The TQWidget constructor calls create(0,TRUE,TRUE) to create a
    window for this widget.
*/

void TQWidget::create( WId window, bool initializeWindow, bool destroyOldWindow)
{
    if ( testWState(WState_Created) && window == 0 )
	return;

    // set created flag
    setWState( WState_Created );

    bool popup = testWFlags(WType_Popup);
    bool dialog = testWFlags(WType_Dialog);
    bool desktop = testWFlags(WType_Desktop);

    // top-level widget
    if ( !parentWidget() || parentWidget()->isDesktop() )
	setWFlags( WType_TopLevel );

    // these are top-level, too
    if ( dialog || popup || desktop || testWFlags(WStyle_Splash))
	setWFlags( WType_TopLevel );

    // a popup stays on top
    if ( popup )
	setWFlags(WStyle_StaysOnTop);

    bool topLevel = testWFlags(WType_TopLevel);
    Window parentw, destroyw = 0;
    WId	   id;

    // always initialize
    if ( !window )
	initializeWindow = TRUE;

    if ( desktop &&
	 qt_x11_create_desktop_on_screen >= 0 &&
	 qt_x11_create_desktop_on_screen != x11Screen() ) {
	// desktop on a certain screen other than the default requested
	TQPaintDeviceX11Data* xd = getX11Data( TRUE );
	xd->x_screen = qt_x11_create_desktop_on_screen;
	xd->x_depth = TQPaintDevice::x11AppDepth( xd->x_screen );
	xd->x_cells = TQPaintDevice::x11AppCells( xd->x_screen );
	xd->x_colormap = TQPaintDevice::x11AppColormap( xd->x_screen );
	xd->x_defcolormap = TQPaintDevice::x11AppDefaultColormap( xd->x_screen );
	xd->x_visual = TQPaintDevice::x11AppVisual( xd->x_screen );
	xd->x_defvisual = TQPaintDevice::x11AppDefaultVisual( xd->x_screen );
	setX11Data( xd );
    } else if ( parentWidget() &&  parentWidget()->x11Screen() != x11Screen() ) {
	// if we have a parent widget, move to its screen if necessary
	TQPaintDeviceX11Data* xd = getX11Data( TRUE );
	xd->x_screen = parentWidget()->x11Screen();
	xd->x_depth = TQPaintDevice::x11AppDepth( xd->x_screen );
	xd->x_cells = TQPaintDevice::x11AppCells( xd->x_screen );
	xd->x_colormap = TQPaintDevice::x11AppColormap( xd->x_screen );
	xd->x_defcolormap = TQPaintDevice::x11AppDefaultColormap( xd->x_screen );
	xd->x_visual = TQPaintDevice::x11AppVisual( xd->x_screen );
	xd->x_defvisual = TQPaintDevice::x11AppDefaultVisual( xd->x_screen );
	setX11Data( xd );
    }

    //get display, screen number, root window and desktop geometry for
    //the current screen
    Display *dpy = x11Display();
    int scr = x11Screen();
    Window root_win = RootWindow( dpy, scr );
    int sw = DisplayWidth(dpy,scr);
    int sh = DisplayHeight(dpy,scr);

    if ( desktop ) {				// desktop widget
	dialog = popup = FALSE;			// force these flags off
	crect.setRect( 0, 0, sw, sh );
    } else if ( topLevel ) {			// calc pos/size from screen
	crect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
    } else {					// child widget
	crect.setRect( 0, 0, 100, 30 );
    }

    parentw = topLevel ? root_win : parentWidget()->winId();

    XSetWindowAttributes wsa;

    if ( window ) {				// override the old window
	if ( destroyOldWindow )
	    destroyw = winid;
	id = window;
	setWinId( window );
	XWindowAttributes a;
	XGetWindowAttributes( dpy, window, &a );
	crect.setRect( a.x, a.y, a.width, a.height );

	if ( a.map_state == IsUnmapped )
	    clearWState( WState_Visible );
	else
	    setWState( WState_Visible );

	TQPaintDeviceX11Data* xd = getX11Data( TRUE );

	// find which screen the window is on...
	xd->x_screen = TQPaintDevice::x11AppScreen(); // by default, use the default :)
	int i;
	for ( i = 0; i < ScreenCount( dpy ); i++ ) {
	    if ( RootWindow( dpy, i ) == a.root ) {
		xd->x_screen = i;
		break;
	    }
	}

	xd->x_depth = a.depth;
	xd->x_cells = DisplayCells( dpy, xd->x_screen );
	xd->x_visual = a.visual;
	xd->x_defvisual = ( XVisualIDFromVisual( a.visual ) ==
			    XVisualIDFromVisual( (Visual*)x11AppVisual(x11Screen()) ) );
	xd->x_colormap = a.colormap;
	xd->x_defcolormap = ( a.colormap == x11AppColormap( x11Screen() ) );
	setX11Data( xd );
    } else if ( desktop ) {			// desktop widget
	id = (WId)parentw;			// id = root window
	TQWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->setWinId( 0 );	// remove id from widget mapper
	    setWinId( id );			// make sure otherDesktop is
	    otherDesktop->setWinId( id );	//   found first
	} else {
	    setWinId( id );
	}
    } else {
	if ( x11DefaultVisual() && x11DefaultColormap() ) {
	    id = (WId)qt_XCreateSimpleWindow( this, dpy, parentw,
					      crect.left(), crect.top(),
					      crect.width(), crect.height(),
					      0,
					      black.pixel(x11Screen()),
					      bg_col.pixel(x11Screen()) );
	} else {
	    wsa.background_pixel = bg_col.pixel(x11Screen());
	    wsa.border_pixel = black.pixel(x11Screen());
	    wsa.colormap = (Colormap)x11Colormap();
	    id = (WId)qt_XCreateWindow( this, dpy, parentw,
					crect.left(), crect.top(),
					crect.width(), crect.height(),
					0, x11Depth(), InputOutput,
					(Visual*)x11Visual(),
					CWBackPixel|CWBorderPixel|CWColormap,
					&wsa );
	}

	setWinId( id );				// set widget id/handle + hd
    }

#ifndef TQT_NO_XFTFREETYPE
    if (rendhd) {
	XftDrawDestroy( (XftDraw *) rendhd );
	rendhd = 0;
    }

    if ( tqt_has_xft )
	rendhd = (HANDLE) XftDrawCreate( dpy, id, (Visual *) x11Visual(),
					 x11Colormap() );
#endif // TQT_NO_XFTFREETYPE

    // NET window states
    long net_winstates[6] = { 0, 0, 0, 0, 0, 0 };
    long net_winactions[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int curr_winstate = 0;
    int curr_winaction = 0;

    // Add all default actions that cannot be turned off
    net_winactions[curr_winaction++] = qt_net_wm_action_stick;
    net_winactions[curr_winaction++] = qt_net_wm_action_fullscreen;
    net_winactions[curr_winaction++] = qt_net_wm_action_change_desktop;
    net_winactions[curr_winaction++] = qt_net_wm_action_above;
    net_winactions[curr_winaction++] = qt_net_wm_action_below;

    struct {
        ulong flags, functions, decorations;
        long input_mode;
        ulong status;
    } mwmhints;

    mwmhints.flags = mwmhints.functions = 0L;
    mwmhints.decorations = (1L << 0); // MWM_DECOR_ALL
    mwmhints.input_mode = 0L;
    mwmhints.status = 0L;

    if ( testNFlags(WX11DisableResize) ) {
        mwmhints.functions |= (1L << 1); // MWM_FUNC_RESIZE
        mwmhints.functions |= (1L << 0); // MWM_FUNC_ALL
        mwmhints.flags |= (1L << 0); // MWM_HINTS_FUNCTIONS
    }
    else {
        net_winactions[curr_winaction++] = qt_net_wm_action_resize;
    }

    if ( testNFlags(WX11DisableMove) ) {
        mwmhints.functions |= (1L << 2); // MWM_FUNC_MOVE
        mwmhints.functions |= (1L << 0); // MWM_FUNC_ALL
        mwmhints.flags |= (1L << 0); // MWM_HINTS_FUNCTIONS
    }
    else {
        net_winactions[curr_winaction++] = qt_net_wm_action_move;
    }

    if ( testNFlags(WX11DisableMinimize) ) {
        mwmhints.functions |= (1L << 3); // MWM_FUNC_MINIMIZE
        mwmhints.functions |= (1L << 0); // MWM_FUNC_ALL
        mwmhints.flags |= (1L << 0); // MWM_HINTS_FUNCTIONS
    }
    else {
        net_winactions[curr_winaction++] = qt_net_wm_action_minimize;
    }

    if ( testNFlags(WX11DisableMaximize) ) {
        mwmhints.functions |= (1L << 4); // MWM_FUNC_MAXIMIZE
        mwmhints.functions |= (1L << 0); // MWM_FUNC_ALL
        mwmhints.flags |= (1L << 0); // MWM_HINTS_FUNCTIONS
    }
    else {
        net_winactions[curr_winaction++] = qt_net_wm_action_max_h;
        net_winactions[curr_winaction++] = qt_net_wm_action_max_v;
    }

    if ( testNFlags(WX11DisableClose) ) {
        mwmhints.functions |= (1L << 5); // MWM_FUNC_CLOSE
        mwmhints.functions |= (1L << 0); // MWM_FUNC_ALL
        mwmhints.flags |= (1L << 0); // MWM_HINTS_FUNCTIONS
    }
    else {
        net_winactions[curr_winaction++] = qt_net_wm_action_close;
    }

    if ( ! testNFlags(WX11DisableShade) ) {
        net_winactions[curr_winaction++] = qt_net_wm_action_shade;
    }

    if (topLevel && ! (desktop || popup)) {
	ulong wsa_mask = 0;

	if ( testWFlags(WStyle_Splash) ) {
            if (qt_net_supports(qt_net_wm_window_type_splash)) {
                clearWFlags( WX11BypassWM );
	    } else {
		setWFlags( WX11BypassWM | WStyle_Tool | WStyle_NoBorder );
	    }
        }
	if (testWFlags(WStyle_Customize)) {
	    mwmhints.decorations = 0L;
	    mwmhints.flags |= (1L << 1); // MWM_HINTS_DECORATIONS

	    if ( testWFlags( WStyle_NormalBorder | WStyle_DialogBorder ) ) {
		mwmhints.decorations |= (1L << 1); // MWM_DECOR_BORDER
		mwmhints.decorations |= (1L << 2); //  MWM_DECOR_RESIZEH
	    }

	    if ( testWFlags( WStyle_Title ) )
		mwmhints.decorations |= (1L << 3); // MWM_DECOR_TITLE

	    if ( testWFlags( WStyle_SysMenu ) )
		mwmhints.decorations |= (1L << 4); // MWM_DECOR_MENU

	    if ( testWFlags( WStyle_Minimize ) )
		mwmhints.decorations |= (1L << 5); // MWM_DECOR_MINIMIZE

	    if ( testWFlags( WStyle_Maximize ) )
		mwmhints.decorations |= (1L << 6); // MWM_DECOR_MAXIMIZE

	    if (testWFlags(WStyle_Tool)) {
		wsa.save_under = True;
		wsa_mask |= CWSaveUnder;
	    }
	} else if (testWFlags(WType_Dialog)) {
	    setWFlags(WStyle_NormalBorder | WStyle_Title |
		      WStyle_SysMenu | WStyle_ContextHelp);
	} else {
	    setWFlags(WStyle_NormalBorder | WStyle_Title |
		      WStyle_MinMax | WStyle_SysMenu);

	    // maximized netwm state
	    if (testWFlags(WState_Maximized)) {
		net_winstates[curr_winstate++] = qt_net_wm_state_max_v;
		net_winstates[curr_winstate++] = qt_net_wm_state_max_h;
	    }
	}

	// stays on top
	if (testWFlags(WStyle_StaysOnTop)) {
	    net_winstates[curr_winstate++] = qt_net_wm_state_above;
	    net_winstates[curr_winstate++] = qt_net_wm_state_stays_on_top;
	}

        if (testWFlags(WShowModal)) {
            mwmhints.input_mode = 3L; // MWM_INPUT_FULL_APPLICATION_MODAL
            mwmhints.flags |= (1L << 2); // MWM_HINTS_INPUT_MODE

            net_winstates[curr_winstate++] = qt_net_wm_state_modal;
        }

        if ( testWFlags( WX11BypassWM ) ) {
	    wsa.override_redirect = True;
	    wsa_mask |= CWOverrideRedirect;
	}

	if ( wsa_mask && initializeWindow )
	    XChangeWindowAttributes( dpy, id, wsa_mask, &wsa );
    } else {
	if (! testWFlags(WStyle_Customize))
	    setWFlags(WStyle_NormalBorder | WStyle_Title |
		      WStyle_MinMax | WStyle_SysMenu);
    }


    if ( !initializeWindow ) {
	// do no initialization
    } else if ( popup ) {			// popup widget
	wsa.override_redirect = True;
	wsa.save_under = True;
	XChangeWindowAttributes( dpy, id, CWOverrideRedirect | CWSaveUnder,
				 &wsa );
	XClassHint class_hint;
	class_hint.res_name = (char *) tqAppName(); // application name
	class_hint.res_class = (char *) tqAppClass();	// application class
        XSetClassHint( dpy, id, &class_hint );
    } else if ( topLevel && !desktop ) {	// top-level widget
	TQWidget *p = parentWidget();	// real parent
	if (p)
	    p = p->topLevelWidget();

	if (dialog || testWFlags(WStyle_DialogBorder) || testWFlags(WStyle_Tool)) {
	    if ( p )
		XSetTransientForHint( dpy, id, p->winId() );
	    else				// application-modal
		XSetTransientForHint( dpy, id, root_win );
	}

	// find the real client leader, i.e. a toplevel without parent
	while ( p && p->parentWidget())
	    p = p->parentWidget()->topLevelWidget();

	XSizeHints size_hints;
	size_hints.flags = USSize | PSize | PWinGravity;
	size_hints.x = crect.left();
	size_hints.y = crect.top();
	size_hints.width = crect.width();
	size_hints.height = crect.height();
	size_hints.win_gravity =
	    TQApplication::reverseLayout() ? NorthEastGravity : NorthWestGravity;

	XWMHints wm_hints;			// window manager hints
	wm_hints.input = True;
	wm_hints.initial_state = NormalState;
	wm_hints.flags = InputHint | StateHint;
	if ( !qt_x11_wm_client_leader )
	    qt_x11_create_wm_client_leader();

	wm_hints.window_group = qt_x11_wm_client_leader;
	wm_hints.flags |= WindowGroupHint;

	XClassHint class_hint;
	class_hint.res_name = (char *) tqAppName(); // application name
	class_hint.res_class = (char *) tqAppClass();	// application class

       	XSetWMProperties( dpy, id, 0, 0, 0, 0, &size_hints, &wm_hints, &class_hint );

	XResizeWindow( dpy, id, crect.width(), crect.height() );
	XStoreName( dpy, id, tqAppName() );
	Atom protocols[5];
	int n = 0;
	protocols[n++] = tqt_wm_delete_window;	// support del window protocol
	protocols[n++] = tqt_wm_take_focus;	// support take focus window protocol
	protocols[n++] = qt_net_wm_ping;	// support _NET_WM_PING protocol
#ifndef TQT_NO_XSYNC
	protocols[n++] = qt_net_wm_sync_request;// support the _NET_WM_SYNC_REQUEST protocol
#endif
	if ( testWFlags( WStyle_ContextHelp ) )
	    protocols[n++] = qt_net_wm_context_help;
	XSetWMProtocols( dpy, id, protocols, n );

        // set mwm hints
        if ( mwmhints.flags != 0l )
            XChangeProperty(dpy, id, qt_xa_motif_wm_hints, qt_xa_motif_wm_hints, 32,
                            PropModeReplace, (unsigned char *) &mwmhints, 5);
        else
            XDeleteProperty(dpy, id, qt_xa_motif_wm_hints);

	// set _NET_WM_WINDOW_STATE
	if (curr_winstate > 0)
	    XChangeProperty(dpy, id, qt_net_wm_state, XA_ATOM, 32, PropModeReplace,
			    (unsigned char *) net_winstates, curr_winstate);
        else
            XDeleteProperty(dpy, id, qt_net_wm_state);

	// set _NET_WM_ALLOWED_ACTIONS
	if (curr_winaction > 0)
	    XChangeProperty(dpy, id, qt_net_wm_action, XA_ATOM, 32, PropModeReplace,
			    (unsigned char *) net_winactions, curr_winaction);
        else
            XDeleteProperty(dpy, id, qt_net_wm_action);

#ifndef TQT_NO_XSYNC
        // set _NET_WM_SYNC_COUNTER
        createSyncCounter();
        long counterVal = topData()->syncCounter;
        XChangeProperty( dpy, id, qt_net_wm_sync_request_counter, XA_CARDINAL, 32, PropModeReplace,
                         (unsigned char*) &counterVal, 1);
#endif

	// when we create a toplevel widget, the frame strut should be dirty
	fstrut_dirty = 1;

    } else {
	// non-toplevel widgets don't have a frame, so no need to
	// update the strut
	fstrut_dirty = 0;
    }

    if (initializeWindow && (popup || (topLevel && !desktop))) { // properties set on all toplevel windows
	x11SetWindowType();

	// set _NET_WM_PID
	long curr_pid = getpid();
	XChangeProperty(dpy, id, qt_net_wm_pid, XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *) &curr_pid, 1);

	// declare the widget's object name as window role
	XChangeProperty( dpy, id,
			 tqt_window_role, XA_STRING, 8, PropModeReplace,
			 (unsigned char *)name(), tqstrlen( name() ) );

	// set client leader property
	if ( !qt_x11_wm_client_leader )
	    qt_x11_create_wm_client_leader();
	XChangeProperty( dpy, id, qt_wm_client_leader,
			 XA_WINDOW, 32, PropModeReplace,
			 (unsigned char *)&qt_x11_wm_client_leader, 1 );
    }

    if ( initializeWindow ) {
	// don't erase when resizing
	wsa.bit_gravity =
	    TQApplication::reverseLayout() ? NorthEastGravity : NorthWestGravity;
	XChangeWindowAttributes( dpy, id, CWBitGravity, &wsa );
    }

    setWState( WState_MouseTracking );
    setMouseTracking( FALSE );			// also sets event mask
    if ( desktop ) {
	setWState( WState_Visible );
    } else if ( topLevel ) {			// set X cursor
	setWState( WState_OwnCursor );
	if ( initializeWindow )
	    tqt_x11_enforce_cursor( this );
    }

    if ( destroyw )
	qt_XDestroyWindow( this, dpy, destroyw );

#if !defined(TQT_NO_IM_EXTENSIONS)
    ic = 0;
#endif
}


/*!
    Frees up window system resources. Destroys the widget window if \a
    destroyWindow is TRUE.

    destroy() calls itself recursively for all the child widgets,
    passing \a destroySubWindows for the \a destroyWindow parameter.
    To have more control over destruction of subwidgets, destroy
    subwidgets selectively first.

    This function is usually called from the TQWidget destructor.
*/

void TQWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    deactivateWidgetCleanup();
    if ( testWState(WState_Created) ) {
	clearWState( WState_Created );
	if ( children() ) {
	    TQObjectListIt it(*children());
	    TQObject *obj;
	    while ( (obj=it.current()) ) {	// destroy all widget children
		++it;
		if ( obj->isWidgetType() )
		    ((TQWidget*)obj)->destroy(destroySubWindows,
					     destroySubWindows);
	    }
	}
	if ( mouseGrb == this )
	    releaseMouse();
	if ( keyboardGrb == this )
	    releaseKeyboard();
	if ( isTopLevel() )
	    qt_deferred_map_take( this );
	if ( testWFlags(WShowModal) )		// just be sure we leave modal
	    tqt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    tqApp->closePopup( this );

#ifndef TQT_NO_XFTFREETYPE
	if ( rendhd) {
	    if ( destroyWindow )
		XftDrawDestroy( (XftDraw *) rendhd );
	    else
		free( (void*) rendhd );
	    rendhd = 0;
	}
#endif // TQT_NO_XFTFREETYPE

	if ( testWFlags(WType_Desktop) ) {
	    if ( acceptDrops() )
		qt_dnd_enable( this, FALSE );
	} else {
	    if ( destroyWindow )
		qt_XDestroyWindow( this, x11Display(), winid );
	}
#ifndef TQT_NO_XSYNC
        destroySyncCounter();
#endif
	setWinId( 0 );

	extern void qPRCleanup( TQWidget *widget ); // from qapplication_x11.cpp
	if ( testWState(WState_Reparented) )
	    qPRCleanup(this);

	if( this == icHolderWidget() ) {
	    destroyInputContext();
	} else {
#ifndef TQT_NO_IM
	    // release previous focus information participating with
	    // preedit preservation of qic
	    TQInputContext *qic = getInputContext();
	    if ( qic )
		qic->releaseComposingWidget( this );
#endif // TQT_NO_IM
	}
    }
}

void TQWidget::reparentSys( TQWidget *parent, WFlags f, const TQPoint &p, bool showIt )
{
    extern void qPRCreate( const TQWidget *, Window );

    Display *dpy = x11Display();
    TQCursor oldcurs;
    bool setcurs = testWState(WState_OwnCursor);
    if ( setcurs ) {
	oldcurs = cursor();
	unsetCursor();
    }

    // dnd unregister (we will register again below)
    bool accept_drops = acceptDrops();
    setAcceptDrops( FALSE );

    // clear mouse tracking, re-enabled below
    bool mouse_tracking = hasMouseTracking();
    clearWState(WState_MouseTracking);

    TQWidget* oldtlw = topLevelWidget();
    TQWidget *oldparent = parentWidget();
    WId old_winid = winid;
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;
    setWinId( 0 );

    // hide and reparent our own window away. Otherwise we might get
    // destroyed when emitting the child remove event below. See TQWorkspace.
    XUnmapWindow( x11Display(), old_winid );
    XReparentWindow( x11Display(), old_winid,
		     RootWindow( x11Display(), x11Screen() ), 0, 0 );

    if ( this == icHolderWidget() ) {
	// input contexts are sometimes associated with toplevel widgets, so
	// we need destroy the context here.  if we are reparenting back to
	// toplevel, then we may have another context created, otherwise we
	// will use our new ic holder's context
        destroyInputContext();
    }

#ifndef TQT_NO_XSYNC
    destroySyncCounter();
#endif

    if ( isTopLevel() || !parent ) // we are toplevel, or reparenting to toplevel
        topData()->parentWinId = 0;

    if ( parent != parentObj ) {
	if ( parentObj )				// remove from parent
	    parentObj->removeChild( this );
	if ( parent )				// insert into new parent
	    parent->insertChild( this );
    }
    bool     enable = isEnabled();		// remember status
    FocusPolicy fp = focusPolicy();
    TQSize    s	    = size();
    TQPixmap *bgp    = (TQPixmap *)backgroundPixmap();
    TQColor   bgc    = bg_col;			// save colors
    TQString capt= caption();
    widget_flags = f;
    clearWState( WState_Created | WState_Visible | WState_ForceHide );
    create();
    if ( isTopLevel() || (!parent || parent->isVisible() ) )
	setWState( WState_ForceHide );	// new widgets do not show up in already visible parents

    const TQObjectList *chlist = children();
    if ( chlist ) {				// reparent children
        TQObjectList childList(*chlist);
	TQObjectListIt it(childList); // iterate over copy
	TQObject *obj;
	while ( (obj=it.current()) ) {
	    if ( obj->isWidgetType() ) {
		TQWidget *w = (TQWidget *)obj;
		if ( !w->isTopLevel() ) {
		    XReparentWindow( x11Display(), w->winId(), winId(),
				     w->geometry().x(), w->geometry().y() );
		} else if ( w->isPopup()
			    || w->testWFlags(WStyle_DialogBorder)
			    || w->testWFlags(WType_Dialog)
			    || w->testWFlags(WStyle_Tool) ) {
		    /*
		      when reparenting toplevel windows with toplevel-transient children,
		      we need to make sure that the window manager gets the updated
		      WM_TRANSIENT_FOR information... unfortunately, some window managers
		      don't handle changing WM_TRANSIENT_FOR before the toplevel window is
		      visible, so we unmap and remap all toplevel-transient children *after*
		      the toplevel parent has been mapped.  thankfully, this is easy in TQt :)
		    */
		    XUnmapWindow(w->x11Display(), w->winId());
		    XSetTransientForHint(w->x11Display(), w->winId(), winId());
		    TQApplication::postEvent(w, new TQEvent(TQEvent::ShowWindowRequest));
		}
	    }
	    ++it;
	}
    }
    qPRCreate( this, old_winid );
    if ( bgp )
	XSetWindowBackgroundPixmap( dpy, winid, bgp->handle() );
    else
	XSetWindowBackground( dpy, winid, bgc.pixel(x11Screen()) );

    if (isTopLevel()) {
        // preserve maximized/fullscreen flags and the normal geometry
        uint save_state = widget_state & (WState_Maximized | WState_FullScreen);
        const TQRect r = topData()->normalGeometry;
        setGeometry(p.x(), p.y(), s.width(), s.height());
        widget_state |= save_state;
        topData()->normalGeometry = r;
    } else {
        setGeometry(p.x(), p.y(), s.width(), s.height());
    }

    setEnabled( enable );
    setFocusPolicy( fp );
    if ( !capt.isNull() ) {
	extra->topextra->caption = TQString::null;
	setCaption( capt );
    }
    if ( showIt )
	show();
    if ( old_winid )
	qt_XDestroyWindow( this, dpy, old_winid );
    if ( setcurs )
	setCursor(oldcurs);

    reparentFocusWidgets( oldtlw );

    // re-register dnd
    if (oldparent)
	oldparent->checkChildrenDnd();

    if ( accept_drops )
	setAcceptDrops( TRUE );
    else {
	checkChildrenDnd();
	topData()->dnd = 0;
	qt_dnd_enable(this, (extra && extra->children_use_dnd));
    }

    // re-enable mouse tracking
    if (mouse_tracking)
	setMouseTracking(mouse_tracking);
}

// Sets the EWMH (netwm) window type. Needed as a separate function
// because create() may be too soon in some cases.
void TQWidget::x11SetWindowType( X11WindowType type )
{
    // NET window types
    long net_wintypes[7] = { 0, 0, 0, 0, 0, 0, 0 };
    int curr_wintype = 0;
    if( testWFlags(WType_Desktop))
        return;
    if( type == X11WindowTypeSelect ) {
        if ( testWFlags(WStyle_Splash)) {
            if (qt_net_supports(qt_net_wm_window_type_splash)) {
                net_wintypes[curr_wintype++] = qt_net_wm_window_type_splash;
            }
        } else if (inherits("TQToolBar")) {
	    // toolbar netwm type
	    net_wintypes[curr_wintype++] = qt_net_wm_window_type_toolbar;
        } else if (testWFlags(WStyle_Customize) && testWFlags(WStyle_Tool)) {
	    // utility netwm type
	    net_wintypes[curr_wintype++] = qt_net_wm_window_type_utility;
        } else if (testWFlags(WType_Dialog)) {
            // dialog netwm type
            net_wintypes[curr_wintype++] = qt_net_wm_window_type_dialog;
        }
    } else if( type == X11WindowTypeCombo ) {
        // combo netwm type
	net_wintypes[curr_wintype++] = qt_net_wm_window_type_combo;
    } else if( type == X11WindowTypeDND ) {
        // dnd netwm type
    	net_wintypes[curr_wintype++] = qt_net_wm_window_type_dnd;
    } else if( type == X11WindowTypeDropdown ) {
        // dropdown netwm type
    	net_wintypes[curr_wintype++] = qt_net_wm_window_type_dropdown_menu;
    } else if( type == X11WindowTypePopup ) {
        // popup netwm type
    	net_wintypes[curr_wintype++] = qt_net_wm_window_type_popup_menu;
    } else if( type == X11WindowTypeMenu ) {
        // menu netwm type
	net_wintypes[curr_wintype++] = qt_net_wm_window_type_menu;
    } else if( type == X11WindowTypeTooltip ) {
        // tooltip netwm type
    	net_wintypes[curr_wintype++] = qt_net_wm_window_type_tooltip;
    }

    // normal netwm type - default
    net_wintypes[curr_wintype++] = qt_net_wm_window_type_normal;
    // set _NET_WM_WINDOW_TYPE
    if (curr_wintype > 0)
        XChangeProperty(x11Display(), winId(), qt_net_wm_window_type, XA_ATOM, 32, PropModeReplace,
			(unsigned char *) net_wintypes, curr_wintype);
    else
        XDeleteProperty(x11Display(), winId(), qt_net_wm_window_type);
}

void TQWidget::x11SetWindowTransient( TQWidget* parent )
{
    XSetTransientForHint( x11Display(), winId(), parent->winId());
}

/*!
    Translates the widget coordinate \a pos to global screen
    coordinates. For example, \c{mapToGlobal(TQPoint(0,0))} would give
    the global coordinates of the top-left pixel of the widget.

    \sa mapFromGlobal() mapTo() mapToParent()
*/

TQPoint TQWidget::mapToGlobal( const TQPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( x11Display(), winId(),
			   TQApplication::desktop()->screen(x11Screen())->winId(),
			   pos.x(), pos.y(), &x, &y, &child );
    return TQPoint( x, y );
}

/*!
    Translates the global screen coordinate \a pos to widget
    coordinates.

    \sa mapToGlobal() mapFrom() mapFromParent()
*/

TQPoint TQWidget::mapFromGlobal( const TQPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( x11Display(),
			   TQApplication::desktop()->screen(x11Screen())->winId(),
			   winId(), pos.x(), pos.y(), &x, &y, &child );
    return TQPoint( x, y );
}

/*!
    When a widget gets focus, it should call setMicroFocusHint() with
    some appropriate position and size, \a x, \a y, \a width and \a
    height. This has no \e visual effect, it just provides hints to
    any system-specific input handling tools.

    The \a text argument should be TRUE if this is a position for text
    input.

    In the Windows version of TQt, this method sets the system caret,
    which is used for user Accessibility focus handling.  If \a text
    is TRUE, it also sets the IME composition window in Far East Asian
    language input systems.

    In the X11 version of TQt, if \a text is TRUE, this method sets the
    input method focus point in the preedit (XIM "spot" point) for
    complex language input handling.

    The font \a f is a rendering hint to the currently active input method.
    If \a f is 0 the widget's font is used.

    \sa microFocusHint()
*/
void TQWidget::setMicroFocusHint(int x, int y, int width, int height,
				bool text, TQFont *f )
{
#ifndef TQT_NO_IM
    if ( text ) {
	// trigger input context creation if it hasn't happened already
	createInputContext();

	TQInputContext *qic = getInputContext();
	if(qic) {
	    TQPoint gp = mapToGlobal( TQPoint( x, y ) );
	    qic->setMicroFocus(gp.x(), gp.y(), width, height, f);
	}
    }
#endif

    if ( TQRect( x, y, width, height ) != microFocusHint() ) {
	createExtra();
	extraData()->micro_focus_hint.setRect( x, y, width, height );
    }
}


void TQWidget::setFontSys( TQFont * )
{
    // Nothing
}


void TQWidget::setBackgroundColorDirect( const TQColor &color )
{
    bg_col = color;
    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }
    XSetWindowBackground( x11Display(), winId(), bg_col.pixel(x11Screen()) );
}

static int allow_null_pixmaps = 0;


void TQWidget::setBackgroundPixmapDirect( const TQPixmap &pixmap )
{
    TQPixmap old;
    if ( extra && extra->bg_pix )
	old = *extra->bg_pix;
    if ( !allow_null_pixmaps && pixmap.isNull() ) {
	XSetWindowBackground( x11Display(), winId(), bg_col.pixel(x11Screen()) );
	if ( extra && extra->bg_pix ) {
	    delete extra->bg_pix;
	    extra->bg_pix = 0;
	}
    } else {
	TQPixmap pm = pixmap;
	if (!pm.isNull()) {
	    if ( pm.depth() == 1 && TQPixmap::defaultDepth() > 1 ) {
		pm = TQPixmap( pixmap.size() );
		bitBlt( &pm, 0, 0, &pixmap, 0, 0, pm.width(), pm.height() );
	    }
	}
	if ( extra && extra->bg_pix )
	    delete extra->bg_pix;
	else
	    createExtra();
	extra->bg_pix = new TQPixmap( pm );
	TQ_CHECK_PTR( extra->bg_pix );
	extra->bg_pix->x11SetScreen( x11Screen() );
	XSetWindowBackgroundPixmap( x11Display(), winId(), extra->bg_pix->handle() );
	if ( testWFlags(WType_Desktop) )	// save rootinfo later
	    qt_updated_rootinfo();
    }
}


/*!
    Sets the window-system background of the widget to nothing.

    Note that "nothing" is actually a pixmap that isNull(), thus you
    can check for an empty background by checking backgroundPixmap().

    \sa setBackgroundPixmap(), setBackgroundColor()
*/
void TQWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setErasePixmap(TQPixmap());
    allow_null_pixmaps--;
}


void TQWidget::setBackgroundX11Relative()
{
    XSetWindowBackgroundPixmap( x11Display(), winId(), ParentRelative );
}

void TQWidget::setCursor( const TQCursor &cursor )
{
    if ( cursor.handle() != arrowCursor.handle()
	 || (extra && extra->curs) ) {
	createExtra();
	delete extra->curs;
	extra->curs = new TQCursor(cursor);
    }
    setWState( WState_OwnCursor );
    tqt_x11_enforce_cursor( this );
    XFlush( x11Display() );
}

void TQWidget::unsetCursor()
{
    if ( extra ) {
	delete extra->curs;
	extra->curs = 0;
    }
    if ( !isTopLevel() )
	clearWState( WState_OwnCursor );
    tqt_x11_enforce_cursor( this );
    XFlush( x11Display() );
}

static XTextProperty*
qstring_to_xtp( const TQString& s )
{
    static XTextProperty tp = { 0, 0, 0, 0 };
    static bool free_prop = TRUE; // we can't free tp.value in case it references
    // the data of the static TQCString below.
    if ( tp.value ) {
	if ( free_prop )
	    XFree( tp.value );
	tp.value = 0;
	free_prop = TRUE;
    }

    static const TQTextCodec* mapper = TQTextCodec::codecForLocale();
    int errCode = 0;
    if ( mapper ) {
	TQCString mapped = mapper->fromUnicode(s);
	char* tl[2];
	tl[0] = mapped.data();
	tl[1] = 0;
	errCode = XmbTextListToTextProperty( TQPaintDevice::x11AppDisplay(),
					     tl, 1, XStdICCTextStyle, &tp );
#if defined(QT_DEBUG)
	if ( errCode < 0 )
	    tqDebug( "qstring_to_xtp result code %d", errCode );
#endif
    }
    if ( !mapper || errCode < 0 ) {
	static TQCString qcs;
	qcs = s.ascii();
	tp.value = (uchar*)qcs.data();
	tp.encoding = XA_STRING;
	tp.format = 8;
	tp.nitems = qcs.length();
	free_prop = FALSE;
    }

    // ### If we knew WM could understand unicode, we could use
    // ### a much simpler, cheaper encoding...
    /*
	tp.value = (XChar2b*)s.unicode();
	tp.encoding = XA_UNICODE; // wish
	tp.format = 16;
	tp.nitems = s.length();
    */

    return &tp;
}

void TQWidget::setCaption( const TQString &caption )
{
    if ( TQWidget::caption() == caption )
	return;

    topData()->caption = caption;
    XSetWMName( x11Display(), winId(), qstring_to_xtp(caption) );

    TQCString net_wm_name = caption.utf8();
    XChangeProperty(x11Display(), winId(), qt_net_wm_name, qt_utf8_string, 8,
		    PropModeReplace, (unsigned char *)net_wm_name.data(),
		    net_wm_name.length());

    TQEvent e( TQEvent::CaptionChange );
    TQApplication::sendEvent( this, &e );
}

void TQWidget::setIcon( const TQPixmap &pixmap )
{
    if ( extra && extra->topextra ) {
	delete extra->topextra->icon;
	extra->topextra->icon = 0;
    } else {
	createTLExtra();
    }
    Pixmap icon_pixmap = 0;
    Pixmap mask_pixmap = 0;
    TQPixmap* pm = NULL;
    if ( !pixmap.isNull() ) {
	if (pixmap.depth() == 24) {
	    pm = new TQPixmap( pixmap );
	}
	else {
	    // With most window managers, only 24-bit icon pixmaps are allowed in the WM hints, otherwise corrupt icons will be displayed
	    // Convert provided pixmaps to 24-bit here
	    int w = pixmap.width();
	    int h = pixmap.height();
	    pm = new TQPixmap( pixmap.width(), pixmap.height(), 24 );
	    TQImage iconImage = pixmap.convertToImage();

	    // Load the new 24-bit RGB pixmap with data
	    GC gc;
	    pm->detach();
	    TQt::HANDLE pmHandle = pm->handle();
	    gc = XCreateGC(x11Display(), pmHandle, 0, 0);
	    for (int y = 0; y < h; ++y) {
		TQRgb *ls = (TQRgb *)iconImage.scanLine( y );
		for (int x = 0; x < w; ++x) {
		    TQRgb l = ls[x];
		    int r = int( tqRed( l ) );
		    int g = int( tqGreen( l ) );
		    int b = int( tqBlue( l ) );
		    XSetForeground(x11Display(), gc, (r << 16) | (g << 8) | b );
		    XDrawPoint(x11Display(), pmHandle, gc, x, y);
		}
	    }
	    XFreeGC(x11Display(), gc);
	}

	extra->topextra->icon = pm;
	if ( !pm->mask() ) {
	    pm->setMask( pm->createHeuristicMask() ); // may do detach()
	}
	icon_pixmap = pm->handle();
	if ( pm->mask() ) {
	    mask_pixmap = pm->mask()->handle();
	}
    }
    XWMHints *h = XGetWMHints( x11Display(), winId() );
    XWMHints  wm_hints;
    bool got_hints = h != 0;
    if ( !got_hints ) {
	h = &wm_hints;
	h->flags = 0;
    }
    h->icon_pixmap = icon_pixmap;
    h->icon_mask = mask_pixmap;
    h->flags |= IconPixmapHint | IconMaskHint;
    XSetWMHints( x11Display(), winId(), h );
    if ( got_hints )
	XFree( (char *)h );
    TQEvent e( TQEvent::IconChange );
    TQApplication::sendEvent( this, &e );
}

void TQWidget::setIconText( const TQString &iconText )
{
    if (TQWidget::iconText() == iconText)
        return;

    topData()->iconText = iconText;
    XSetWMIconName( x11Display(), winId(), qstring_to_xtp(iconText) );

    TQCString net_wm_icon_name = iconText.utf8();
    XChangeProperty(x11Display(), winId(), qt_net_wm_icon_name, qt_utf8_string, 8, PropModeReplace,
		    (unsigned char *) net_wm_icon_name.data(), net_wm_icon_name.length());
}

void TQWidget::setMouseTracking( bool enable )
{
    bool gmt = TQApplication::hasGlobalMouseTracking();
    if ( !enable == !testWState(WState_MouseTracking) && !gmt )
	return;
    uint m = (enable || gmt) ? (uint)PointerMotionMask : 0;
    if ( enable )
	setWState( WState_MouseTracking );
    else
	clearWState( WState_MouseTracking );
    if ( testWFlags(WType_Desktop) ) {		// desktop widget?
	TQWidget* main_desktop = find( winId() );
	if ( main_desktop->testWFlags(WPaintDesktop) )
	    XSelectInput( x11Display(), winId(),
			  stdDesktopEventMask | ExposureMask );
	else
	    XSelectInput( x11Display(), winId(), stdDesktopEventMask );
    } else {
	XSelectInput( x11Display(), winId(),
		      m | stdWidgetEventMask );
#if defined (QT_TABLET_SUPPORT)
 	if ( devStylus != NULL ) {
 	    XSelectExtensionEvent( x11Display(), winId(), event_list_stylus,
 				   qt_curr_events_stylus );
 	}
	if ( devEraser != NULL ) {
	    XSelectExtensionEvent( x11Display(), winId(), event_list_eraser,
				   qt_curr_events_eraser );
	}
#endif
    }
}


/*!
    Grabs the mouse input.

    This widget receives all mouse events until releaseMouse() is
    called; other widgets get no mouse events at all. Keyboard
    events are not affected. Use grabKeyboard() if you want to grab
    that.

    \warning Bugs in mouse-grabbing applications very often lock the
    terminal. Use this function with extreme caution, and consider
    using the \c -nograb command line option while debugging.

    It is almost never necessary to grab the mouse when using TQt, as
    TQt grabs and releases it sensibly. In particular, TQt grabs the
    mouse when a mouse button is pressed and keeps it until the last
    button is released.

    Note that only visible widgets can grab mouse input. If
    isVisible() returns FALSE for a widget, that widget cannot call
    grabMouse().

    \sa releaseMouse() grabKeyboard() releaseKeyboard() grabKeyboard()
    focusWidget()
*/

void TQWidget::grabMouse()
{
    if ( isVisible() && !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
#if defined(QT_CHECK_STATE)
	int status =
#endif
	    XGrabPointer( x11Display(), winId(), False,
			  (uint)( ButtonPressMask | ButtonReleaseMask |
				  PointerMotionMask | EnterWindowMask |
				  LeaveWindowMask ),
			  GrabModeAsync, GrabModeAsync,
			  None, None, tqt_x_time );
#if defined(QT_CHECK_STATE)
	if ( status ) {
	    const char *s =
		status == GrabNotViewable ? "\"GrabNotViewable\"" :
		status == AlreadyGrabbed  ? "\"AlreadyGrabbed\"" :
		status == GrabFrozen      ? "\"GrabFrozen\"" :
		status == GrabInvalidTime ? "\"GrabInvalidTime\"" :
		"<?>";
	    tqWarning( "Grabbing the mouse failed with %s", s );
	}
#endif
	mouseGrb = this;
    }
}

/*!
    \overload

    Grabs the mouse input and changes the cursor shape.

    The cursor will assume shape \a cursor (for as long as the mouse
    focus is grabbed) and this widget will be the only one to receive
    mouse events until releaseMouse() is called().

    \warning Grabbing the mouse might lock the terminal.

    \sa releaseMouse(), grabKeyboard(), releaseKeyboard(), setCursor()
*/

void TQWidget::grabMouse( const TQCursor &cursor )
{
    if ( !qt_nograb() ) {
	if ( mouseGrb != this ) {
	    if ( mouseGrb ) {
		mouseGrb->releaseMouse();
	    }
#if defined(QT_CHECK_STATE)
	    int status =
#endif
	        XGrabPointer( x11Display(), winId(), False,
		    (uint)(ButtonPressMask | ButtonReleaseMask |
		    PointerMotionMask | EnterWindowMask | LeaveWindowMask),
		    GrabModeAsync, GrabModeAsync,
		    None, cursor.handle(), tqt_x_time );
#if defined(QT_CHECK_STATE)
	    if ( status ) {
	        const char *s =
		    status == GrabNotViewable ? "\"GrabNotViewable\"" :
		    status == AlreadyGrabbed  ? "\"AlreadyGrabbed\"" :
		    status == GrabFrozen      ? "\"GrabFrozen\"" :
		    status == GrabInvalidTime ? "\"GrabInvalidTime\"" :
						"<?>";
	        tqWarning( "Grabbing the mouse failed with %s", s );
	    }
#endif
	    mouseGrb = this;
	}
    }
}

/*!
    Releases the mouse grab.

    \sa grabMouse(), grabKeyboard(), releaseKeyboard()
*/

void TQWidget::releaseMouse()
{
    if ( !qt_nograb() && mouseGrb == this ) {
	XUngrabPointer( x11Display(),  tqt_x_time );
	XFlush( x11Display() );
	mouseGrb = 0;
    }
}

/*!
    Grabs the keyboard input.

    This widget reveives all keyboard events until releaseKeyboard()
    is called; other widgets get no keyboard events at all. Mouse
    events are not affected. Use grabMouse() if you want to grab that.

    The focus widget is not affected, except that it doesn't receive
    any keyboard events. setFocus() moves the focus as usual, but the
    new focus widget receives keyboard events only after
    releaseKeyboard() is called.

    If a different widget is currently grabbing keyboard input, that
    widget's grab is released first.

    \sa releaseKeyboard() grabMouse() releaseMouse() focusWidget()
*/

void TQWidget::grabKeyboard()
{
    if ( !qt_nograb() ) {
	if ( keyboardGrb != this ) {
	    if ( keyboardGrb ) {
		keyboardGrb->releaseKeyboard();
	    }
	    XGrabKeyboard( x11Display(), winid, False, GrabModeAsync, GrabModeAsync, tqt_x_time );
	    keyboardGrb = this;
	}
    }
}

/*!
    Releases the keyboard grab.

    \sa grabKeyboard(), grabMouse(), releaseMouse()
*/

void TQWidget::releaseKeyboard()
{
    if ( !qt_nograb() && keyboardGrb == this ) {
	XUngrabKeyboard( x11Display(), tqt_x_time );
	keyboardGrb = 0;
    }
}


/*!
    Returns the widget that is currently grabbing the mouse input.

    If no widget in this application is currently grabbing the mouse,
    0 is returned.

    \sa grabMouse(), keyboardGrabber()
*/

TQWidget *TQWidget::mouseGrabber()
{
    return mouseGrb;
}

/*!
    Returns the widget that is currently grabbing the keyboard input.

    If no widget in this application is currently grabbing the
    keyboard, 0 is returned.

    \sa grabMouse(), mouseGrabber()
*/

TQWidget *TQWidget::keyboardGrabber()
{
    return keyboardGrb;
}

/*!
    Sets the top-level widget containing this widget to be the active
    window.

    An active window is a visible top-level window that has the
    keyboard input focus.

    This function performs the same operation as clicking the mouse on
    the title bar of a top-level window. On X11, the result depends on
    the Window Manager. If you want to ensure that the window is
    stacked on top as well you should also call raise(). Note that the
    window must be visible, otherwise setActiveWindow() has no effect.

    On Windows, if you are calling this when the application is not
    currently the active one then it will not make it the active
    window.  It will flash the task bar entry blue to indicate that
    the window has done something. This is because Microsoft do not
    allow an application to interrupt what the user is currently doing
    in another application.

    \sa isActiveWindow(), topLevelWidget(), show()
*/

void TQWidget::setActiveWindow()
{
    TQWidget *tlw = topLevelWidget();
    if ( tlw->isVisible() && !tlw->topData()->embedded && !qt_deferred_map_contains(tlw) ) {
	XSetInputFocus( x11Display(), tlw->winId(), RevertToNone, tqt_x_time);
	focusInputContext();
    }
}


/*!
    Updates the widget unless updates are disabled or the widget is
    hidden.

    This function does not cause an immediate repaint; instead it
    schedules a paint event for processing when TQt returns to the main
    event loop. This permits TQt to optimize for more speed and less
    flicker than a call to repaint() does.

    Calling update() several times normally results in just one
    paintEvent() call.

    TQt normally erases the widget's area before the paintEvent() call.
    If the \c WRepaintNoErase widget flag is set, the widget is
    responsible for painting all its pixels itself.

    \sa repaint(), paintEvent(), setUpdatesEnabled(), erase(),
    setWFlags()
*/

void TQWidget::update()
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) ==
	 WState_Visible )
	TQApplication::postEvent( this, new TQPaintEvent( clipRegion(), !testWFlags(WRepaintNoErase) ) );
}

/*!
    \overload

    Updates a rectangle (\a x, \a y, \a w, \a h) inside the widget
    unless updates are disabled or the widget is hidden.

    This function does not cause an immediate repaint; instead it
    schedules a paint event for processing when TQt returns to the main
    event loop. This permits TQt to optimize for more speed and less
    flicker and a call to repaint() does.

    Calling update() several times normally results in just one
    paintEvent() call.

    If \a w is negative, it is replaced with \c{width() - x}. If \a h
    is negative, it is replaced width \c{height() - y}.

    TQt normally erases the specified area before the paintEvent()
    call. If the \c WRepaintNoErase widget flag is set, the widget is
    responsible for painting all its pixels itself.

    \sa repaint(), paintEvent(), setUpdatesEnabled(), erase()
*/

void TQWidget::update( int x, int y, int w, int h )
{
    if ( w && h &&
	 (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	if ( w != 0 && h != 0 )
	    TQApplication::postEvent( this,
		new TQPaintEvent( clipRegion().intersect(TQRect(x,y,w,h)),
				 !testWFlags( WRepaintNoErase ) ) );
    }
}

/*!
    \overload void TQWidget::update( const TQRect &r )

    Updates a rectangle \a r inside the widget unless updates are
    disabled or the widget is hidden.

    This function does not cause an immediate repaint; instead it
    schedules a paint event for processing when TQt returns to the main
    event loop. This permits TQt to optimize for more speed and less
    flicker and a call to repaint() does.

    Calling update() several times normally results in just one
    paintEvent() call.
*/

/*!
    \overload void TQWidget::repaint( bool erase )

    This version repaints the entire widget.
*/

/*!
    \overload void TQWidget::repaint()

    This version erases and repaints the entire widget.
*/

/*!
    Repaints the widget directly by calling paintEvent() immediately,
    unless updates are disabled or the widget is hidden.

    If \a erase is TRUE, TQt erases the area \a (x, y, w, h) before the
    paintEvent() call.

    If \a w is negative, it is replaced with \c{width() - x}, and if
    \a h is negative, it is replaced width \c{height() - y}.

    We suggest only using repaint() if you need an immediate repaint,
    for example during animation. In almost all circumstances update()
    is better, as it permits TQt to optimize for speed and minimize
    flicker.

    \warning If you call repaint() in a function which may itself be
    called from paintEvent(), you may get infinite recursion. The
    update() function never causes recursion.

    \sa update(), paintEvent(), setUpdatesEnabled(), erase()
*/

void TQWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
	if ( x > crect.width() || y > crect.height() )
	    return;
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	TQRect r(x,y,w,h);
	if ( r.isEmpty() )
	    return; // nothing to do
	TQPaintEvent e( r, erase );
	if ( r != rect() )
	    qt_set_paintevent_clipping( this, r );
	if ( erase && w != 0 && h != 0 ) {
	    if ( backgroundOrigin() == WidgetOrigin )
		XClearArea( x11Display(), winId(), x, y, w, h, False );
	    else
		this->erase( x, y, w, h);
	}
	TQApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping();
    }
}

/*!
    \overload

    Repaints the widget directly by calling paintEvent() directly,
    unless updates are disabled or the widget is hidden.

    Erases the widget region \a reg if \a erase is TRUE.

    Only use repaint if your widget needs to be repainted immediately,
    for example when doing some animation. In all other cases, use
    update(). Calling update() many times in a row will generate a
    single paint event.

    \warning If you call repaint() in a function which may itself be
    called from paintEvent(), you may get infinite recursion. The
    update() function never causes recursion.

    \sa update(), paintEvent(), setUpdatesEnabled(), erase()
*/

void TQWidget::repaint( const TQRegion& reg, bool erase )
{
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
	TQPaintEvent e( reg, erase );
	qt_set_paintevent_clipping( this, reg );
	if ( erase )
	    this->erase(reg);
	TQApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping();
    }
}

/*!
    \overload void TQWidget::repaint( const TQRect &r, bool erase )

    Repaints the widget directly by calling paintEvent() directly,
    unless updates are disabled or the widget is hidden.

    Erases the widget region \a r if \a erase is TRUE.
*/

void TQWidget::setWindowState(uint newstate)
{
    bool needShow = FALSE;
    uint oldstate = windowState();
    if (isTopLevel()) {
        TQTLWExtra *top = topData();

	if ((oldstate & WindowMaximized) != (newstate & WindowMaximized)) {
	    if (qt_net_supports(qt_net_wm_state_max_h) && qt_net_supports(qt_net_wm_state_max_v)) {
		qt_net_change_wm_state(this, (newstate & WindowMaximized),
				       qt_net_wm_state_max_h, qt_net_wm_state_max_v);
	    } else if (! (newstate & WindowFullScreen)) {
		if (newstate & WindowMaximized) {
		    // save original geometry
                    const TQRect normalGeometry = geometry();

		    if (isVisible()) {
			updateFrameStrut();
			const TQRect maxRect = TQApplication::desktop()->availableGeometry(this);
			const TQRect r = top->normalGeometry;
			setGeometry(maxRect.x() + top->fleft,
				    maxRect.y() + top->ftop,
				    maxRect.width() - top->fleft - top->fright,
				    maxRect.height() - top->ftop - top->fbottom);
                        top->normalGeometry = r;
		    }

                    if (top->normalGeometry.width() < 0)
			top->normalGeometry = normalGeometry;
		} else {
		    // restore original geometry
		    setGeometry(top->normalGeometry);
		}
	    }
	}

	if ((oldstate & WindowFullScreen) != (newstate & WindowFullScreen)) {
	    if (qt_net_supports(qt_net_wm_state_fullscreen)) {
		qt_net_change_wm_state(this, (newstate & WindowFullScreen),
				       qt_net_wm_state_fullscreen);
	    } else {
                needShow = isVisible();

		if (newstate & WindowFullScreen) {
                    const TQRect normalGeometry = TQRect(pos(), size());

		    top->savedFlags = getWFlags();
                    reparent(0, WType_TopLevel | WStyle_Customize | WStyle_NoBorder |
			     // preserve some widget flags
			     (getWFlags() & 0xffff0000),
			     mapToGlobal(TQPoint(0, 0)));
                    const TQRect r = top->normalGeometry;
                    setGeometry(tqApp->desktop()->screenGeometry(this));
                    top->normalGeometry = r;

                    if ( top->normalGeometry.width() < 0 )
			top->normalGeometry = normalGeometry;
                } else {
		    reparent( 0, top->savedFlags, mapToGlobal(TQPoint(0, 0)) );

                    if (newstate & WindowMaximized) {
                        // from fullscreen to maximized
			updateFrameStrut();
			const TQRect maxRect = TQApplication::desktop()->availableGeometry(this);
			const TQRect r = top->normalGeometry;
			setGeometry(maxRect.x() + top->fleft,
				    maxRect.y() + top->ftop,
				    maxRect.width() - top->fleft - top->fright,
				    maxRect.height() - top->ftop - top->fbottom);
                        top->normalGeometry = r;
                    } else {
                        // restore original geometry
                        setGeometry(top->normalGeometry);
                    }
		}
	    }
	}

	if ((oldstate & WindowMinimized) != (newstate & WindowMinimized)) {
	    if (isVisible()) {
		if (newstate & WindowMinimized) {
		    XEvent e;
		    e.xclient.type = ClientMessage;
		    e.xclient.message_type = qt_wm_change_state;
		    e.xclient.display = x11Display();
		    e.xclient.window = winid;
		    e.xclient.format = 32;
		    e.xclient.data.l[0] = IconicState;
		    e.xclient.data.l[1] = 0;
		    e.xclient.data.l[2] = 0;
		    e.xclient.data.l[3] = 0;
		    e.xclient.data.l[4] = 0;
		    XSendEvent(x11Display(), RootWindow(x11Display(), x11Screen()),
			       False, (SubstructureNotifyMask|SubstructureRedirectMask), &e);
		} else {
		    XMapWindow(x11Display(), winId());
		}
	    }

	    needShow = FALSE;
	}
    }

    widget_state &= ~(WState_Minimized | WState_Maximized | WState_FullScreen);
    if (newstate & WindowMinimized)
	widget_state |= WState_Minimized;
    if (newstate & WindowMaximized)
	widget_state |= WState_Maximized;
    if (newstate & WindowFullScreen)
	widget_state |= WState_FullScreen;

    if (needShow)
	show();

    if (newstate & WindowActive)
	setActiveWindow();

    TQEvent e(TQEvent::WindowStateChange);
    TQApplication::sendEvent(this, &e);
}

/*!
  \internal
  Platform-specific part of TQWidget::show().
*/

void TQWidget::showWindow()
{
    if ( isTopLevel()  ) {
	XWMHints *h = XGetWMHints( x11Display(), winId() );
	XWMHints  wm_hints;
	bool got_hints = h != 0;
	if ( !got_hints ) {
	    h = &wm_hints;
	    h->flags = 0;
	}
	h->initial_state = testWState(WState_Minimized) ? IconicState : NormalState;
	h->flags |= StateHint;
	XSetWMHints( x11Display(), winId(), h );
	if ( got_hints )
	    XFree( (char *)h );

	if (tqt_x_user_time != CurrentTime) {
	    XChangeProperty(x11Display(), winId(), qt_net_wm_user_time, XA_CARDINAL,
			    32, PropModeReplace, (unsigned char *) &tqt_x_user_time, 1);
	}

	if (!topData()->embedded &&
	    topData()->parentWinId &&
	    topData()->parentWinId != TQPaintDevice::x11AppRootWindow(x11Screen()) &&
	    !isMinimized() ) {
	    qt_deferred_map_add( this );
	    return;
	}

 	if (isMaximized() && !isFullScreen()
	    && !(qt_net_supports(qt_net_wm_state_max_h)
		 && qt_net_supports(qt_net_wm_state_max_v))) {
 	    XMapWindow( x11Display(), winId() );
 	    tqt_wait_for_window_manager(this);

 	    // if the wm was not smart enough to adjust our size, do that manually
 	    updateFrameStrut();
	    TQRect maxRect = TQApplication::desktop()->availableGeometry(this);

 	    TQTLWExtra *top = topData();
 	    TQRect normalRect = top->normalGeometry;

 	    setGeometry(maxRect.x() + top->fleft,
			maxRect.y() + top->ftop,
 			maxRect.width() - top->fleft - top->fright,
 			maxRect.height() - top->ftop - top->fbottom);

	    // restore the original normalGeometry
 	    top->normalGeometry = normalRect;
 	    // internalSetGeometry() clears the maximized flag... make sure we set it back
 	    setWState(WState_Maximized);

 	    return;
 	}

	if (isFullScreen() && !qt_net_supports(qt_net_wm_state_fullscreen)) {
 	    XMapWindow(x11Display(), winId());
 	    tqt_wait_for_window_manager(this);
	    return;
	}
    }
    XMapWindow( x11Display(), winId() );
}


/*!
  \internal
  Platform-specific part of TQWidget::hide().
*/

void TQWidget::hideWindow()
{
    clearWState( WState_Exposed );
    deactivateWidgetCleanup();
    if ( isTopLevel() ) {
	qt_deferred_map_take( this );
	if ( winId() ) // in nsplugin, may be 0
	    XWithdrawWindow( x11Display(), winId(), x11Screen() );

	TQTLWExtra *top = topData();
	crect.moveTopLeft( TQPoint(crect.x() - top->fleft, crect.y() - top->ftop ) );

	// zero the frame strut and mark it dirty
	top->fleft = top->fright = top->ftop = top->fbottom = 0;
	fstrut_dirty = TRUE;

	XFlush( x11Display() );
    } else {
	if ( winId() ) // in nsplugin, may be 0
	    XUnmapWindow( x11Display(), winId() );
    }
}

/*!
    Raises this widget to the top of the parent widget's stack.

    After this call the widget will be visually in front of any
    overlapping sibling widgets.

    \sa lower(), stackUnder()
*/

void TQWidget::raise()
{
    TQWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->append( p->childObjects->take() );
    XRaiseWindow( x11Display(), winId() );
}

/*!
    Lowers the widget to the bottom of the parent widget's stack.

    After this call the widget will be visually behind (and therefore
    obscured by) any overlapping sibling widgets.

    \sa raise(), stackUnder()
*/

void TQWidget::lower()
{
    TQWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( 0, p->childObjects->take() );
    XLowerWindow( x11Display(), winId() );
}


/*!
    Places the widget under \a w in the parent widget's stack.

    To make this work, the widget itself and \a w must be siblings.

    \sa raise(), lower()
*/
void TQWidget::stackUnder( TQWidget* w)
{
    TQWidget *p = parentWidget();
    if ( !w || isTopLevel() || p != w->parentWidget() || this == w )
	return;
    if ( p && p->childObjects && p->childObjects->findRef(w) >= 0 && p->childObjects->findRef(this) >= 0 ) {
	p->childObjects->take();
	p->childObjects->insert( p->childObjects->findRef(w), this );
    }
    Window stack[2];
    stack[0] = w->winId();;
    stack[1] = winId();
    XRestackWindows( x11Display(), stack, 2 );
}



/*
  The global variable qt_widget_tlw_gravity defines the window gravity of
  the next top level window to be created. We do this when setting the
  main widget's geometry and the "-geometry" command line option contains
  a negative position.
*/

int qt_widget_tlw_gravity = NorthWestGravity;

static void do_size_hints( TQWidget* widget, TQWExtra *x )
{
    XSizeHints s;
    s.flags = 0;
    if ( x ) {
	s.x = widget->x();
	s.y = widget->y();
	s.width = widget->width();
	s.height = widget->height();
	if ( x->minw > 0 || x->minh > 0 ) {	// add minimum size hints
	    s.flags |= PMinSize;
	    s.min_width  = x->minw;
	    s.min_height = x->minh;
	}
	if ( x->maxw < TQWIDGETSIZE_MAX || x->maxh < TQWIDGETSIZE_MAX ) {
	    s.flags |= PMaxSize;		// add maximum size hints
	    s.max_width  = x->maxw;
	    s.max_height = x->maxh;
	}
	if ( x->topextra &&
	   (x->topextra->incw > 0 || x->topextra->inch > 0) )
	{					// add resize increment hints
	    s.flags |= PResizeInc | PBaseSize;
	    s.width_inc = x->topextra->incw;
	    s.height_inc = x->topextra->inch;
	    s.base_width = x->topextra->basew;
	    s.base_height = x->topextra->baseh;
	}

	if ( x->topextra && x->topextra->uspos) {
	    s.flags |= USPosition;
	    s.flags |= PPosition;
	}
	if ( x->topextra && x->topextra->ussize) {
	    s.flags |= USSize;
	    s.flags |= PSize;
	}
    }
    s.flags |= PWinGravity;
    s.win_gravity = qt_widget_tlw_gravity;	// usually NorthWest
    // reset in case it was set
    qt_widget_tlw_gravity =
        TQApplication::reverseLayout() ? NorthEastGravity : NorthWestGravity;
    XSetWMNormalHints( widget->x11Display(), widget->winId(), &s );
}


void TQWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
    Display *dpy = x11Display();

    if ( testWFlags(WType_Desktop) )
	return;
    if (isTopLevel()) {
        if (!qt_net_supports(qt_net_wm_state_max_h)
            && !qt_net_supports(qt_net_wm_state_max_v))
            clearWState(WState_Maximized);
        if (!qt_net_supports(qt_net_wm_state_fullscreen))
            clearWState(WState_FullScreen);
	topData()->normalGeometry = TQRect(0, 0, -1, -1);
    } else {
        // for TQWorkspace
        clearWState(WState_Maximized);
        clearWState(WState_FullScreen);
    }
    if ( extra ) {				// any size restrictions?
	w = TQMIN(w,extra->maxw);
	h = TQMIN(h,extra->maxh);
	w = TQMAX(w,extra->minw);
	h = TQMAX(h,extra->minh);
    }
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    TQPoint oldPos( pos() );
    TQSize oldSize( size() );
    TQRect oldGeom( crect );
    TQRect  r( x, y, w, h );

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if ( !isTopLevel() && oldGeom == r )
	return;

    crect = r;
    bool isResize = size() != oldSize;

    if ( isTopLevel() ) {
	if ( isMove )
	    topData()->uspos = 1;
	if ( isResize )
	    topData()->ussize = 1;
	do_size_hints( this, extra );
    }

    if ( isMove ) {
	if (! qt_broken_wm)
	    // pos() is right according to ICCCM 4.1.5
	    XMoveResizeWindow( dpy, winid, pos().x(), pos().y(), w, h );
	else
	    // work around 4Dwm's incompliance with ICCCM 4.1.5
	    XMoveResizeWindow( dpy, winid, x, y, w, h );
    } else if ( isResize )
	XResizeWindow( dpy, winid, w, h );

    if ( isVisible() ) {
	if ( isMove && pos() != oldPos ) {
	    if ( ! qt_broken_wm ) {
		// pos() is right according to ICCCM 4.1.5
		TQMoveEvent e( pos(), oldPos );
		TQApplication::sendEvent( this, &e );
	    } else {
		// work around 4Dwm's incompliance with ICCCM 4.1.5
		TQMoveEvent e( crect.topLeft(), oldGeom.topLeft() );
		TQApplication::sendEvent( this, &e );
	    }
	}
	if ( isResize ) {

	    // set config pending only on resize, see qapplication_x11.cpp, translateConfigEvent()
	    setWState( WState_ConfigPending );

	    TQResizeEvent e( size(), oldSize );
	    TQApplication::sendEvent( this, &e );
	}
    } else {
	if ( isMove && pos() != oldPos ) {
	    if ( ! qt_broken_wm )
		// pos() is right according to ICCCM 4.1.5
		TQApplication::postEvent( this, new TQMoveEvent( pos(), oldPos ) );
	    else
		// work around 4Dwm's incompliance with ICCCM 4.1.5
		TQApplication::postEvent( this, new TQMoveEvent( crect.topLeft(),
							       oldGeom.topLeft() ) );
	}
	if ( isResize )
	    TQApplication::postEvent( this,
				     new TQResizeEvent( size(), oldSize ) );
    }
}


/*!
    \overload

    This function corresponds to setMinimumSize( TQSize(minw, minh) ).
    Sets the minimum width to \a minw and the minimum height to \a
    minh.
*/

void TQWidget::setMinimumSize( int minw, int minh )
{
#if defined(QT_CHECK_RANGE)
    if ( minw < 0 || minh < 0 )
	tqWarning("TQWidget::setMinimumSize: minw=%d, minh=%d. The smallest allowed size is (0,0)", minw, minh);
#endif
    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
	return;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() ) {
	bool resized = testWState( WState_Resized );
	resize( TQMAX(minw,width()), TQMAX(minh,height()) );
	if ( !resized )
	    clearWState( WState_Resized ); // not a user resize
    }
    if ( testWFlags(WType_TopLevel) )
	do_size_hints( this, extra );
    updateGeometry();
}

/*!
    \overload

    This function corresponds to setMaximumSize( TQSize(\a maxw, \a
    maxh) ). Sets the maximum width to \a maxw and the maximum height
    to \a maxh.
*/
void TQWidget::setMaximumSize( int maxw, int maxh )
{
#if defined(QT_CHECK_RANGE)
    if ( maxw > TQWIDGETSIZE_MAX || maxh > TQWIDGETSIZE_MAX ) {
	tqWarning("TQWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 name( "unnamed" ), className(), TQWIDGETSIZE_MAX,
		TQWIDGETSIZE_MAX );
	maxw = TQMIN( maxw, TQWIDGETSIZE_MAX );
	maxh = TQMIN( maxh, TQWIDGETSIZE_MAX );
    }
    if ( maxw < 0 || maxh < 0 ) {
	tqWarning("TQWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		name( "unnamed" ), className(), maxw, maxh );
	maxw = TQMAX( maxw, 0 );
	maxh = TQMAX( maxh, 0 );
    }
#endif
    createExtra();
    if ( extra->maxw == maxw && extra->maxh == maxh )
	return;
    extra->maxw = maxw;
    extra->maxh = maxh;
    if ( maxw < width() || maxh < height() ) {
	bool resized = testWState( WState_Resized );
	resize( TQMIN(maxw,width()), TQMIN(maxh,height()) );
	if ( !resized )
	    clearWState( WState_Resized ); // not a user resize
    }
    if ( testWFlags(WType_TopLevel) )
	do_size_hints( this, extra );
    updateGeometry();
}

/*!
    \overload

    Sets the x (width) size increment to \a w and the y (height) size
    increment to \a h.
*/
void TQWidget::setSizeIncrement( int w, int h )
{
    TQTLWExtra* x = topData();
    if ( x->incw == w && x->inch == h )
	return;
    x->incw = w;
    x->inch = h;
    if ( testWFlags(WType_TopLevel) )
	do_size_hints( this, extra );
}

/*!
    \overload

    This corresponds to setBaseSize( TQSize(\a basew, \a baseh) ). Sets
    the widgets base size to width \a basew and height \a baseh.
*/
void TQWidget::setBaseSize( int basew, int baseh )
{
    createTLExtra();
    TQTLWExtra* x = topData();
    if ( x->basew == basew && x->baseh == baseh )
	return;
    x->basew = basew;
    x->baseh = baseh;
    if ( testWFlags(WType_TopLevel) )
	do_size_hints( this, extra );
}

/*!
    \overload void TQWidget::erase()

    This version erases the entire widget.
*/

/*!
  \overload void TQWidget::erase( const TQRect &r )

  Erases the specified area \a r in the widget without generating a
  \link paintEvent() paint event\endlink.
*/

/*!
    Erases the specified area \a (x, y, w, h) in the widget without
    generating a \link paintEvent() paint event\endlink.

    If \a w is negative, it is replaced with \c{width() - x}. If \a h
    is negative, it is replaced width \c{height() - y}.

    Child widgets are not affected.

    \sa repaint()
*/

void TQWidget::erase( int x, int y, int w, int h )
{
    extern void qt_erase_rect( TQWidget*, const TQRect& ); // in qpainer_x11.cpp
    if ( w < 0 )
	w = crect.width()  - x;
    if ( h < 0 )
	h = crect.height() - y;
    if ( w != 0 && h != 0 )
	qt_erase_rect( this, TQRect(x, y, w, h ) );
}

/*!
    \overload

    Erases the area defined by \a reg, without generating a \link
    paintEvent() paint event\endlink.

    Child widgets are not affected.
*/

void TQWidget::erase( const TQRegion& reg )
{
    extern void qt_erase_region( TQWidget*, const TQRegion& ); // in qpainer_x11.cpp
    qt_erase_region( this, reg );
}

/*!
    Scrolls the widget including its children \a dx pixels to the
    right and \a dy downwards. Both \a dx and \a dy may be negative.

    After scrolling, scroll() sends a paint event for the the part
    that is read but not written. For example, when scrolling 10
    pixels rightwards, the leftmost ten pixels of the widget need
    repainting. The paint event may be delivered immediately or later,
    depending on some heuristics (note that you might have to force
    processing of paint events using TQApplication::sendPostedEvents()
    when using scroll() and move() in combination).

    \sa TQScrollView erase() bitBlt()
*/

void TQWidget::scroll( int dx, int dy )
{
    scroll( dx, dy, TQRect() );
}

/*!
    \overload

    This version only scrolls \a r and does not move the children of
    the widget.

    If \a r is empty or invalid, the result is undefined.

    \sa TQScrollView erase() bitBlt()
*/
void TQWidget::scroll( int dx, int dy, const TQRect& r )
{
    if ( testWState( WState_BlockUpdates ) && !children() )
	return;
    bool valid_rect = r.isValid();
    bool just_update = TQABS( dx ) > width() || TQABS( dy ) > height();
    if ( just_update )
	update();
    TQRect sr = valid_rect?r:clipRegion().boundingRect();
    int x1, y1, x2, y2, w=sr.width(), h=sr.height();
    if ( dx > 0 ) {
	x1 = sr.x();
	x2 = x1+dx;
	w -= dx;
    } else {
	x2 = sr.x();
	x1 = x2-dx;
	w += dx;
    }
    if ( dy > 0 ) {
	y1 = sr.y();
	y2 = y1+dy;
	h -= dy;
    } else {
	y2 = sr.y();
	y1 = y2-dy;
	h += dy;
    }

    if ( dx == 0 && dy == 0 )
	return;

    Display *dpy = x11Display();
    GC gc = tqt_xget_readonly_gc( x11Screen(), FALSE );
    // Want expose events
    if ( w > 0 && h > 0 && !just_update ) {
	XSetGraphicsExposures( dpy, gc, True );
	XCopyArea( dpy, winId(), winId(), gc, x1, y1, w, h, x2, y2);
	XSetGraphicsExposures( dpy, gc, False );
    }

    if ( !valid_rect && children() ) {	// scroll children
	TQPoint pd( dx, dy );
	TQObjectListIt it(*children());
	TQObject *object;
	while ( it ) {				// move all children
	    object = it.current();
	    if ( object->isWidgetType() ) {
		TQWidget *w = (TQWidget *)object;
		w->move( w->pos() + pd );
	    }
	    ++it;
	}
    }

    if ( just_update )
	return;

    // Don't let the server be bogged-down with repaint events
    bool repaint_immediately = qt_sip_count( this ) < 3;

    if ( dx ) {
	int x = x2 == sr.x() ? sr.x()+w : sr.x();
	if ( repaint_immediately )
	    repaint( x, sr.y(), TQABS(dx), sr.height(), !testWFlags(WRepaintNoErase) );
	else
	    XClearArea( dpy, winid, x, sr.y(), TQABS(dx), sr.height(), True );
    }
    if ( dy ) {
	int y = y2 == sr.y() ? sr.y()+h : sr.y();
	if ( repaint_immediately )
	    repaint( sr.x(), y, sr.width(), TQABS(dy), !testWFlags(WRepaintNoErase) );
	else
	    XClearArea( dpy, winid, sr.x(), y, sr.width(), TQABS(dy), True );
    }

    qt_insert_sip( this, dx, dy ); // #### ignores r
}


/*!
    \overload void TQWidget::drawText( const TQPoint &pos, const TQString& str )

    Draws the string \a str at position \a pos.
*/

/*!
    Draws the string \a str at position \a(x, y).

    The \a y position is the base line position of the text. The text
    is drawn using the default font and the default foreground color.

    This function is provided for convenience. You will generally get
    more flexible results and often higher speed by using a a \link
    TQPainter painter\endlink instead.

    \sa setFont(), foregroundColor(), TQPainter::drawText()
*/

void TQWidget::drawText( int x, int y, const TQString &str )
{
    if ( testWState(WState_Visible) ) {
	TQPainter paint;
	paint.begin( this );
	paint.drawText( x, y, str );
	paint.end();
    }
}


/*!
    Internal implementation of the virtual TQPaintDevice::metric()
    function.

    Use the TQPaintDeviceMetrics class instead.

    \a m is the metric to get.
*/

int TQWidget::metric( int m ) const
{
    int val;
    if ( m == TQPaintDeviceMetrics::PdmWidth ) {
	val = crect.width();
    } else if ( m == TQPaintDeviceMetrics::PdmHeight ) {
	val = crect.height();
    } else {
	Display *dpy = x11Display();
	int scr = x11Screen();
	switch ( m ) {
	    case TQPaintDeviceMetrics::PdmDpiX:
	    case TQPaintDeviceMetrics::PdmPhysicalDpiX:
		val = TQPaintDevice::x11AppDpiX( scr );
		break;
	    case TQPaintDeviceMetrics::PdmDpiY:
	    case TQPaintDeviceMetrics::PdmPhysicalDpiY:
		val = TQPaintDevice::x11AppDpiY( scr );
		break;
	    case TQPaintDeviceMetrics::PdmWidthMM:
		val = (DisplayWidthMM(dpy,scr)*crect.width())/
		      DisplayWidth(dpy,scr);
		break;
	    case TQPaintDeviceMetrics::PdmHeightMM:
		val = (DisplayHeightMM(dpy,scr)*crect.height())/
		      DisplayHeight(dpy,scr);
		break;
	    case TQPaintDeviceMetrics::PdmNumColors:
		val = x11Cells();
		break;
	    case TQPaintDeviceMetrics::PdmDepth:
		val = x11Depth();
		break;
	    default:
		val = 0;
#if defined(QT_CHECK_RANGE)
		tqWarning( "TQWidget::metric: Invalid metric command" );
#endif
	}
    }
    return val;
}

void TQWidget::createSysExtra()
{
    extra->xDndProxy = 0;
    extra->children_use_dnd = FALSE;
    extra->compress_events = TRUE;
}

void TQWidget::deleteSysExtra()
{
}

void TQWidget::createTLSysExtra()
{
#if defined(TQT_NO_IM_EXTENSIONS)
    // created lazily
    extra->topextra->xic = 0;
#endif
#ifndef TQT_NO_XSYNC
    extra->topextra->syncCounter = 0;
    extra->topextra->syncRequestValue[0] = 0;
    extra->topextra->syncRequestValue[1] = 0;
#endif
}

void TQWidget::deleteTLSysExtra()
{
    // don't destroy input context here. it will be destroyed in
    // TQWidget::destroy() destroyInputContext();
}

/*
   examine the children of our parent up the tree and set the
   children_use_dnd extra data appropriately... this is used to keep DND enabled
   for widgets that are reparented and don't have DND enabled, BUT *DO* have
   children (or children of children ...) with DND enabled...
*/
void TQWidget::checkChildrenDnd()
{
    TQWidget *widget = this;
    const TQObjectList *children;
    const TQObject *object;
    const TQWidget *child;
    while (widget && ! widget->isDesktop()) {
	// note: this isn't done for the desktop widget

	bool children_use_dnd = FALSE;
	children = widget->children();
	if ( children ) {
	    TQObjectListIt it(*children);
	    while ( (object = it.current()) ) {
		++it;
		if ( object->isWidgetType() ) {
		    child = (const TQWidget *) object;
		    children_use_dnd = (children_use_dnd ||
					child->acceptDrops() ||
					(child->extra &&
					 child->extra->children_use_dnd));
		}
	    }
	}

	widget->createExtra();
	widget->extra->children_use_dnd = children_use_dnd;

	widget = widget->parentWidget();
    }
}


#ifndef TQT_NO_XSYNC
// create a window's XSyncCounter
void TQWidget::createSyncCounter()
{
    if( !qt_use_xsync || !isTopLevel() || topData()->syncCounter )
        return;
    XSyncValue zero;
    XSyncIntToValue( &zero, 0 );
    topData()->syncCounter = XSyncCreateCounter( x11Display(), zero );
}

// destroy a window's XSyncCounter
void TQWidget::destroySyncCounter()
{
    if( !qt_use_xsync || !extra || !extra->topextra
        || !extra->topextra->syncCounter )
        return;
    XSyncDestroyCounter( x11Display(), extra->topextra->syncCounter );
    extra->topextra->syncCounter = 0;
}

// increment a window's XSyncCounter
void TQWidget::incrementSyncCounter()
{
    if( qt_use_xsync && topData()->syncCounter &&
        !(topData()->syncRequestValue[0] == 0 &&
         topData()->syncRequestValue[1] == 0) ) {
        XSyncValue val;
        XSyncIntsToValue( &val, topData()->syncRequestValue[ 0 ], topData()->syncRequestValue[ 1 ] );
        XSyncSetCounter( x11Display(), topData()->syncCounter, val );
        topData()->syncRequestValue[0] = topData()->syncRequestValue[1] = 0;
    }
}

// handle _NET_WM_SYNC_REQUEST
void TQWidget::handleSyncRequest( void* ev )
{
    XEvent* xev = (XEvent*)ev;
    topData()->syncRequestValue[ 0 ] = xev->xclient.data.l[ 2 ];
    topData()->syncRequestValue[ 1 ] = xev->xclient.data.l[ 3 ];
}
#endif  // TQT_NO_XSYNC


/*!
    \property TQWidget::acceptDrops
    \brief whether drop events are enabled for this widget

    Setting this property to TRUE announces to the system that this
    widget \e may be able to accept drop events.

    If the widget is the desktop (TQWidget::isDesktop()), this may
    fail if another application is using the desktop; you can call
    acceptDrops() to test if this occurs.

    \warning
    Do not modify this property in a Drag&Drop event handler.
*/
bool TQWidget::acceptDrops() const
{
    return testWState( WState_DND );
}

void TQWidget::setAcceptDrops( bool on )
{
    if ( testWState(WState_DND) != on ) {
	if ( qt_dnd_enable( this, on ) ) {
	    if ( on )
		setWState( WState_DND );
	    else
		clearWState( WState_DND );
	}

	checkChildrenDnd();
    }
}

/*!
    \overload

    Causes only the parts of the widget which overlap \a region to be
    visible. If the region includes pixels outside the rect() of the
    widget, window system controls in that area may or may not be
    visible, depending on the platform.

    Note that this effect can be slow if the region is particularly
    complex.

    \sa setMask(), clearMask()
*/

void TQWidget::setMask( const TQRegion& region )
{
    XShapeCombineRegion( x11Display(), winId(), ShapeBounding, 0, 0,
			 region.handle(), ShapeSet );
}

/*!
    Causes only the pixels of the widget for which \a bitmap has a
    corresponding 1 bit to be visible. Use TQt::color0 to draw
    transparent regions and TQt::color1 to draw opaque regions of the
    bitmap.

    If the region includes pixels outside the rect() of the widget,
    window system controls in that area may or may not be visible,
    depending on the platform.

    Note that this effect can be slow if the region is particularly
    complex.

    See \c examples/tux for an example of masking for transparency.

    \sa setMask(), clearMask()
*/

void TQWidget::setMask( const TQBitmap &bitmap )
{
    TQBitmap bm = bitmap;
    if ( bm.x11Screen() != x11Screen() )
	bm.x11SetScreen( x11Screen() );
    XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0,
		       bm.handle(), ShapeSet );
}

/*!
    Removes any mask set by setMask().

    \sa setMask()
*/

void TQWidget::clearMask()
{
    XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0,
		       None, ShapeSet );
}

/*!\reimp
 */
void TQWidget::setName( const char *name )
{
    TQObject::setName( name );
    if ( isTopLevel() ) {
	XChangeProperty( x11Display(), winId(),
			 tqt_window_role, XA_STRING, 8, PropModeReplace,
			 (unsigned char *)name, tqstrlen( name ) );
    }
}


/*!
  \internal

  Computes the frame rectangle when needed.  This is an internal function, you
  should never call this.
*/

void TQWidget::updateFrameStrut() const
{
    TQWidget *that = (TQWidget *) this;

    if (! isVisible() || isDesktop()) {
	that->fstrut_dirty = (! isVisible());
	return;
    }

    Atom type_ret;
    Window l = winId(), w = winId(), p, r; // target window, it's parent, root
    Window *c;
    int i_unused;
    unsigned int nc;
    unsigned char *data_ret;
    unsigned long l_unused;

    while (XQueryTree(TQPaintDevice::x11AppDisplay(), w, &r, &p, &c, &nc)) {
	if (c && nc > 0)
	    XFree(c);

	if (! p) {
	    tqWarning("TQWidget::updateFrameStrut(): ERROR - no parent");
	    return;
	}

	// if the parent window is the root window, an Enlightenment virtual root or
	// a NET WM virtual root window, stop here
	data_ret = 0;
	if (p == r ||
	    (XGetWindowProperty(TQPaintDevice::x11AppDisplay(), p,
				qt_enlightenment_desktop, 0, 1, False, XA_CARDINAL,
				&type_ret, &i_unused, &l_unused, &l_unused,
				&data_ret) == Success &&
	     type_ret == XA_CARDINAL)) {
	    if (data_ret)
		XFree(data_ret);

	    break;
	} else if (qt_net_supports(qt_net_virtual_roots) && qt_net_virtual_root_list) {
	    int i = 0;
	    while (qt_net_virtual_root_list[i] != 0) {
		if (qt_net_virtual_root_list[i++] == p)
		    break;
	    }
	}

	l = w;
	w = p;
    }

    // we have our window
    int transx, transy;
    XWindowAttributes wattr;
    if (XTranslateCoordinates(TQPaintDevice::x11AppDisplay(), l, w,
			      0, 0, &transx, &transy, &p) &&
	XGetWindowAttributes(TQPaintDevice::x11AppDisplay(), w, &wattr)) {
	TQTLWExtra *top = that->topData();
	top->fleft = transx;
	top->ftop = transy;
	top->fright = wattr.width - crect.width() - top->fleft;
	top->fbottom = wattr.height - crect.height() - top->ftop;

	// add the border_width for the window managers frame... some window managers
	// do not use a border_width of zero for their frames, and if we the left and
	// top strut, we ensure that pos() is absolutely correct.  frameGeometry()
	// will still be incorrect though... perhaps i should have foffset as well, to
	// indicate the frame offset (equal to the border_width on X).
	// - Brad
	top->fleft += wattr.border_width;
	top->fright += wattr.border_width;
	top->ftop += wattr.border_width;
	top->fbottom += wattr.border_width;
    }

    that->fstrut_dirty = 0;
}


/*!
    This function returns the widget holding the TQInputContext
    instance for this widget. The instance is used for text input to
    this widget, switching input method, etc.

    By default, this function delegates the role of returning input
    context holder widget to TQApplication::locateICHolderWidget().

    This definition enables application developer to change the
    mapping of widgets to TQInputContext instance simply by overriding
    TQApplication::locateICHolderWidget().

    \sa TQApplication::locateICHolderWidget()
*/
TQWidget *TQWidget::icHolderWidget()
{
    return tqApp->locateICHolderWidget(this);
}


/*!
    This function returns the TQInputContext instance for this widget.
    This instance is used for text input to this widget, etc.
    It is simply the accessor function.
*/
TQInputContext *TQWidget::getInputContext()
{
    TQInputContext *qic = 0;

#ifndef TQT_NO_IM
// #if !defined(TQT_NO_IM_EXTENSIONS)
    if ( isInputMethodEnabled() ) {
#if !defined(TQT_NO_IM_EXTENSIONS)
	qic = icHolderWidget()->ic;
#else
//    {
	// icHolderWidget is always topLevelWidget
	TQTLWExtra *topdata = icHolderWidget()->topData();
	qic = (TQInputContext *)topdata->xic;
#endif
    }
#endif // TQT_NO_IM

    return qic;
}


/*!
    This function replaces the TQInputContext instance used for text
    input to this widget. The \a identifierName is the identifier name
    of newly choosed input method.
*/
void TQWidget::changeInputContext( const TQString& identifierName )
{
#ifndef TQT_NO_IM
    TQWidget *icWidget = icHolderWidget();
#if !defined(TQT_NO_IM_EXTENSIONS)
    TQInputContext **qicp = &icWidget->ic;
#else
    TQInputContext **qicp = (TQInputContext **)&icWidget->topData()->xic;
#endif

    if( *qicp )
	delete *qicp;
    // an input context that has the identifierName is generated.
    TQInputContext *qic = TQInputContextFactory::create( identifierName, icWidget );
    *qicp = qic;
    if ( qic ) {
	TQObject::connect( qic, TQ_SIGNAL(imEventGenerated(TQObject *,TQIMEvent *)),
			  tqApp, TQ_SLOT(postIMEvent(TQObject *,TQIMEvent *)) );
	TQObject::connect( qic, TQ_SIGNAL(deletionRequested()),
			  icWidget, TQ_SLOT(destroyInputContext()) );
    }
#else
    (void) identifierName; /* unused */
#endif // TQT_NO_IM
}


/*!
  \internal
    This is an internal function, you should never call this.

    This function is called to generate an input context
    according to a configuration for default input method

    When TQT_NO_IM_EXTENSIONS is not set, input context is 
    generated only when isInputMethodEnabled() returns TRUE.
*/
void TQWidget::createInputContext()
{
// #if !defined(TQT_NO_IM_EXTENSIONS)
    if( !isInputMethodEnabled() || TQApplication::closingDown() )
	return;
// #endif

    TQWidget *icWidget = icHolderWidget();
#ifndef TQT_NO_IM
#if !defined(TQT_NO_IM_EXTENSIONS)
    TQInputContext **qicp = &icWidget->ic;
#else
    TQInputContext **qicp = (TQInputContext **)&icWidget->topData()->xic;
#endif
  
    if ( ! *qicp ) {
	// an input context of the default input method is generated.
	TQInputContext *qic = TQInputContextFactory::create( TQApplication::defaultInputMethod(), icWidget );

	*qicp = qic;
	if ( qic ) {
	    TQObject::connect( qic, TQ_SIGNAL(imEventGenerated(TQObject *,TQIMEvent *)),
			      tqApp, TQ_SLOT(postIMEvent(TQObject *,TQIMEvent *)) );
	    TQObject::connect( qic, TQ_SIGNAL(deletionRequested()),
			      icWidget, TQ_SLOT(destroyInputContext()) );
  	}
    }
#endif // TQT_NO_IM
}


/*!
    \internal
  
    This slot is used to destroy the input context that belonging
    to the widget itself, so icHolderWidget()->ic is not fetched.

    \sa TQInputContext::deletionRequested()
*/
void TQWidget::destroyInputContext()
{
#ifndef TQT_NO_IM
#if !defined(TQT_NO_IM_EXTENSIONS)
    TQInputContext **qicp = &ic;
#else
    if ( ! extra || ! extra->topextra )
	return;

    TQInputContext **qicp = (TQInputContext **)&extra->topextra->xic;
#endif

    if( *qicp )
	delete *qicp;

    *qicp = 0;
#endif // TQT_NO_IM
}


/*!
    This function is called when text widgets need to be neutral state to
    execute text operations properly. See qlineedit.cpp and qtextedit.cpp as
    example.

    Ordinary reset that along with changing focus to another widget,
    moving the cursor, etc, is implicitly handled via
    unfocusInputContext() because whether reset or not when such
    situation is a responsibility of input methods. So we delegate the
    responsibility to the input context via unfocusInputContext(). See
    'Preedit preservation' section of the class description of
    TQInputContext for further information.

    \sa TQInputContext, unfocusInputContext(), TQInputContext::unsetFocus()
*/
void TQWidget::resetInputContext()
{
#ifndef TQT_NO_IM
    // trigger input context creation if it hasn't happened already
    createInputContext();

    TQInputContext *qic = getInputContext();
    if( qic )
	qic->reset();
#endif // TQT_NO_IM
}


/*!
    \internal
    This is an internal function, you should never call this.
  
    This function is called to focus associated input context. The
    code intends to eliminate duplicate focus for the context even if
    the context is shared between widgets

    \sa TQInputContext::setFocus()
 */
void TQWidget::focusInputContext()
{
#ifndef TQT_NO_IM
    TQWidget* tlw = topLevelWidget();

    if (!tlw->isPopup() || isInputMethodEnabled()) {
	// trigger input context creation if it hasn't happened already
	createInputContext();

	TQInputContext *qic = getInputContext();
	if ( qic ) {
	    if( qic->focusWidget() != this ) {
		qic->setFocusWidget( this );
		qic->setFocus();
	    }
	}
    }
#endif // TQT_NO_IM
}


/*!
    \internal
    This is an internal function, you should never call this.

    This function is called to remove focus from associated input
    context.

    \sa TQInputContext::unsetFocus()
 */
void TQWidget::unfocusInputContext()
{
#ifndef TQT_NO_IM
    // trigger input context creation if it hasn't happened already
    createInputContext();

    TQInputContext *qic = getInputContext();
    if ( qic ) {
	// may be caused reset() in some input methods
	qic->unsetFocus();
	qic->setFocusWidget( 0 );
    }
#endif // TQT_NO_IM
}
  

/*!
    This function is called to send mouse event to associated input
    context by derived text widgets. A derived text widget must be
    calculate \a x as character offset at the mouse cursor in the
    preedit.

    \sa TQInputContext::mouseHandler()
 */
void TQWidget::sendMouseEventToInputContext( int x, TQEvent::Type type,
					    TQt::ButtonState button,
					    TQt::ButtonState state )
{
#ifndef TQT_NO_IM
    // trigger input context creation if it hasn't happened already
    createInputContext();
  
    TQInputContext *qic = getInputContext();
    if ( qic ) {
	// may be causing reset() in some input methods
	qic->mouseHandler( x, type, button, state );
    }
#endif // TQT_NO_IM
}
  

void TQWidget::setWindowOpacity(double)
{
}

double TQWidget::windowOpacity() const
{
    return 1.0;
}
