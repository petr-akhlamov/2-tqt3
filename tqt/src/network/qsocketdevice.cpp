/****************************************************************************
**
** Implementation of TQSocketDevice class.
**
** Created : 970521
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the network module of the TQt GUI Toolkit.
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

#include "ntqsocketdevice.h"
#ifndef TQT_NO_NETWORK

#include "ntqwindowdefs.h"
#include <string.h>


//#define TQSOCKETDEVICE_DEBUG


class TQSocketDevicePrivate
{
public:
    TQSocketDevicePrivate( TQSocketDevice::Protocol p )
	: protocol(p)
    { }

    TQSocketDevice::Protocol protocol;
};


/*!
    \class TQSocketDevice ntqsocketdevice.h
    \brief The TQSocketDevice class provides a platform-independent low-level socket API.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">TQt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class provides a low level API for working with sockets.  Users of
    this class are assumed to have networking experience. For most users the
    TQSocket class provides a much easier and high level alternative, but
    certain things (like UDP) can't be done with TQSocket and if you need a
    platform-independent API for those, TQSocketDevice is the right choice.

    The essential purpose of the class is to provide a TQIODevice that
    works on sockets, wrapped in a platform-independent API.

    When calling connect() or bind(), TQSocketDevice detects the
    protocol family (IPv4, IPv6) automatically. Passing the protocol
    family to TQSocketDevice's constructor or to setSocket() forces
    creation of a socket device of a specific protocol. If not set, the
    protocol will be detected at the first call to connect() or bind().

    \sa TQSocket, TQSocketNotifier, TQHostAddress
*/


/*!
    \enum TQSocketDevice::Protocol

    This enum type describes the protocol family of the socket. Possible values
    are:

    \value IPv4 The socket is an IPv4 socket.
    \value IPv6 The socket is an IPv6 socket.
    \value Unknown The protocol family of the socket is not known. This can
	   happen if you use TQSocketDevice with an already existing socket; it
	   tries to determine the protocol family, but this can fail if the
	   protocol family is not known to TQSocketDevice.

    \sa protocol() setSocket()
*/

/*!
    \enum TQSocketDevice::Error

    This enum type describes the error states of TQSocketDevice.

    \value NoError  No error has occurred.

    \value AlreadyBound  The device is already bound, according to bind().

    \value Inaccessible  The operating system or firewall prohibited
			the action.

    \value NoResources  The operating system ran out of a resource.

    \value InternalError  An internal error occurred in TQSocketDevice.

    \value Impossible  An attempt was made to do something which makes
    no sense. For example:
    \code
    ::close( sd->socket() );
    sd->writeBlock( someData, 42 );
    \endcode
    The libc ::close() closes the socket, but TQSocketDevice is not aware
    of this. So when you call writeBlock(), the impossible happens.

    \value NoFiles  The operating system will not let TQSocketDevice open
    another file.

    \value ConnectionRefused  A connection attempt was rejected by the
    peer.

    \value NetworkFailure  There is a network failure.

    \value UnknownError  The operating system did something
    unexpected.
*/

/*!
    \enum TQSocketDevice::Type

    This enum type describes the type of the socket:
    \value Stream  a stream socket (TCP, usually)
    \value Datagram  a datagram socket (UDP, usually)
*/


/*!
    Creates a TQSocketDevice object for the existing socket \a socket.

    The \a type argument must match the actual socket type; use \c
    TQSocketDevice::Stream for a reliable, connection-oriented TCP
    socket, or \c TQSocketDevice::Datagram for an unreliable,
    connectionless UDP socket.
*/
TQSocketDevice::TQSocketDevice( int socket, Type type )
    : fd( socket ), t( type ), p( 0 ), pp( 0 ), e( NoError ),
      d(new TQSocketDevicePrivate(Unknown))
{
#if defined(TQSOCKETDEVICE_DEBUG)
    tqDebug( "TQSocketDevice: Created TQSocketDevice %p (socket %x, type %d)",
	   this, socket, type );
#endif
    init();
    setSocket( socket, type );
}

/*!
    Creates a TQSocketDevice object for a stream or datagram socket.

    The \a type argument must be either \c TQSocketDevice::Stream for a
    reliable, connection-oriented TCP socket, or \c
    TQSocketDevice::Datagram for an unreliable UDP socket.

    The socket is created as an IPv4 socket.

    \sa blocking() protocol()
*/
TQSocketDevice::TQSocketDevice( Type type )
    : fd( -1 ), t( type ), p( 0 ), pp( 0 ), e( NoError ),
      d(new TQSocketDevicePrivate(IPv4))
{
#if defined(TQSOCKETDEVICE_DEBUG)
    tqDebug( "TQSocketDevice: Created TQSocketDevice object %p, type %d",
	    this, type );
#endif
    init();
    setSocket( createNewSocket(), type );
}

/*!
    Creates a TQSocketDevice object for a stream or datagram socket.

    The \a type argument must be either \c TQSocketDevice::Stream for a
    reliable, connection-oriented TCP socket, or \c
    TQSocketDevice::Datagram for an unreliable UDP socket.

    The \a protocol indicates whether the socket should be of type IPv4
    or IPv6. Passing \c Unknown is not meaningful in this context and you
    should avoid using (it creates an IPv4 socket, but your code is not easily
    readable).

    The argument \a dummy is necessary for compatibility with some
    compilers.

    \sa blocking() protocol()
*/
TQSocketDevice::TQSocketDevice( Type type, Protocol protocol, int )
    : fd( -1 ), t( type ), p( 0 ), pp( 0 ), e( NoError ),
      d(new TQSocketDevicePrivate(protocol))
{
#if defined(TQSOCKETDEVICE_DEBUG)
    tqDebug( "TQSocketDevice: Created TQSocketDevice object %p, type %d",
	    this, type );
#endif
    init();
    setSocket( createNewSocket(), type );
}

/*!
    Destroys the socket device and closes the socket if it is open.
*/
TQSocketDevice::~TQSocketDevice()
{
    close();
    delete d;
    d = 0;
#if defined(TQSOCKETDEVICE_DEBUG)
    tqDebug( "TQSocketDevice: Destroyed TQSocketDevice %p", this );
#endif
}


/*!
    Returns TRUE if this is a valid socket; otherwise returns FALSE.

    \sa socket()
*/
bool TQSocketDevice::isValid() const
{
    return fd != -1;
}


/*!
    \fn Type TQSocketDevice::type() const

    Returns the socket type which is either \c TQSocketDevice::Stream
    or \c TQSocketDevice::Datagram.

    \sa socket()
*/
TQSocketDevice::Type TQSocketDevice::type() const
{
    return t;
}

/*!
    Returns the socket's protocol family, which is one of \c Unknown, \c IPv4,
    or \c IPv6.

    TQSocketDevice either creates a socket with a well known protocol family or
    it uses an already existing socket. In the first case, this function
    returns the protocol family it was constructed with. In the second case, it
    tries to determine the protocol family of the socket; if this fails, it
    returns \c Unknown.

    \sa Protocol setSocket()
*/
TQSocketDevice::Protocol TQSocketDevice::protocol() const
{
    if ( d->protocol == Unknown )
	d->protocol = getProtocol();
    return d->protocol;
}

/*!
    Returns the socket number, or -1 if it is an invalid socket.

    \sa isValid(), type()
*/
int TQSocketDevice::socket() const
{
    return fd;
}


/*!
    Sets the socket device to operate on the existing socket \a
    socket.

    The \a type argument must match the actual socket type; use \c
    TQSocketDevice::Stream for a reliable, connection-oriented TCP
    socket, or \c TQSocketDevice::Datagram for an unreliable,
    connectionless UDP socket.

    Any existing socket is closed.

    \sa isValid(), close()
*/
void TQSocketDevice::setSocket( int socket, Type type )
{
    if ( fd != -1 )			// close any open socket
	close();
#if defined(TQSOCKETDEVICE_DEBUG)
    tqDebug( "TQSocketDevice::setSocket: socket %x, type %d", socket, type );
#endif
    t = type;
    fd = socket;
    d->protocol = Unknown;
    e = NoError;
    setFlags( IO_Sequential );
    resetStatus();
    open( IO_ReadWrite );
    fetchConnectionParameters();
}


/*!
    \reimp

    Opens the socket using the specified TQIODevice file \a mode. This
    function is called from the TQSocketDevice constructors and from
    the setSocket() function. You should not call it yourself.

    \sa close().
*/
bool TQSocketDevice::open( int mode )
{
    if ( isOpen() || !isValid() )
	return FALSE;
#if defined(TQSOCKETDEVICE_DEBUG)
    tqDebug( "TQSocketDevice::open: mode %x", mode );
#endif
    setMode( mode & IO_ReadWrite );
    setState( IO_Open );
    return TRUE;
}


/*!
    \reimp

    The current TQSocketDevice implementation does not buffer at all,
    so this is a no-op.
*/
void TQSocketDevice::flush()
{
}


/*!
    \reimp

    The size is meaningless for a socket, therefore this function returns 0.
*/
TQIODevice::Offset TQSocketDevice::size() const
{
    return 0;
}


/*!
    \reimp

    The read/write index is meaningless for a socket, therefore this
    function returns 0.
*/
TQIODevice::Offset TQSocketDevice::at() const
{
    return 0;
}


/*!
    \reimp

    The read/write index is meaningless for a socket, therefore this
    function does nothing and returns TRUE.
*/
bool TQSocketDevice::at( Offset )
{
    return TRUE;
}


/*!
    \reimp

    Returns TRUE if no data is currently available at the socket;
    otherwise returns FALSE.
*/
bool TQSocketDevice::atEnd() const
{
    return bytesAvailable() <= 0;
}


/*!
    \reimp

    \warning getch() is implemented as a one-byte readBlock(), so it
    may be very slow if you call it more than a few times.

    \sa putch() readBlock()
*/
int TQSocketDevice::getch()
{
    char buf[2];
    return  readBlock(buf,1) == 1 ? buf[0] : -1;
}


/*!
    \reimp

    \warning putch() is implemented as a one-byte writeBlock(), so it
    may be very slow if you call it more than a few times.

    \sa getch()
*/
int TQSocketDevice::putch( int ch )
{
    char buf[2];
    buf[0] = ch;
    return writeBlock(buf, 1) == 1 ? ch : -1;
}


/*!
    \reimp

    This implementation of ungetch returns -1 (error). A socket is a
    sequential device and does not allow any ungetch operation.
*/
int TQSocketDevice::ungetch( int )
{
    return -1;
}


/*!
    Returns TRUE if the address of this socket can be used by other
    sockets at the same time, and FALSE if this socket claims
    exclusive ownership.

    \sa setAddressReusable()
*/
bool TQSocketDevice::addressReusable() const
{
    return option( ReuseAddress );
}


/*!
    Sets the address of this socket to be usable by other sockets too
    if \a enable is TRUE, and to be used exclusively by this socket if
    \a enable is FALSE.

    When a socket is reusable, other sockets can use the same port
    number (and IP address), which is generally useful. Of course
    other sockets cannot use the same
    (address,port,peer-address,peer-port) 4-tuple as this socket, so
    there is no risk of confusing the two TCP connections.

    \sa addressReusable()
*/
void TQSocketDevice::setAddressReusable( bool enable )
{
    setOption( ReuseAddress, enable );
}


/*!
    Returns the size of the operating system receive buffer.

    \sa setReceiveBufferSize()
*/
int TQSocketDevice::receiveBufferSize() const
{
    return option( ReceiveBuffer );
}


/*!
    Sets the size of the operating system receive buffer to \a size.

    The operating system receive buffer size effectively limits two
    things: how much data can be in transit at any one moment, and how
    much data can be received in one iteration of the main event loop.

    The default is operating system-dependent. A socket that receives
    large amounts of data is probably best with a buffer size of
    49152.
*/
void TQSocketDevice::setReceiveBufferSize( uint size )
{
    setOption( ReceiveBuffer, size );
}


/*!
    Returns the size of the operating system send buffer.

    \sa setSendBufferSize()
*/
int TQSocketDevice::sendBufferSize() const
{
    return option( SendBuffer );
}


/*!
    Sets the size of the operating system send buffer to \a size.

    The operating system send buffer size effectively limits how much
    data can be in transit at any one moment.

    The default is operating system-dependent. A socket that sends
    large amounts of data is probably best with a buffer size of
    49152.
*/
void TQSocketDevice::setSendBufferSize( uint size )
{
    setOption( SendBuffer, size );
}


/*!
    Returns the port number of this socket device. This may be 0 for a
    while, but is set to something sensible as soon as a sensible
    value is available.

    Note that TQt always uses native byte order, i.e. 67 is 67 in TQt;
    there is no need to call htons().
*/
TQ_UINT16 TQSocketDevice::port() const
{
    return p;
}


/*!
    Returns the address of this socket device. This may be 0.0.0.0 for
    a while, but is set to something sensible as soon as a sensible
    value is available.
*/
TQHostAddress TQSocketDevice::address() const
{
    return a;
}


/*!
    Returns the first error seen.
*/
TQSocketDevice::Error TQSocketDevice::error() const
{
    return e;
}


/*!
    Allows subclasses to set the error state to \a err.
*/
void TQSocketDevice::setError( Error err )
{
    e = err;
}
#endif //TQT_NO_NETWORK

