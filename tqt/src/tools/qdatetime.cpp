/****************************************************************************
**
** Implementation of date and time classes
**
** Created : 940124
**
** Copyright (C) 1992-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the tools module of the TQt GUI Toolkit.
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

#include "qplatformdefs.h"

#include "ntqdatetime.h"
#include "ntqdatastream.h"
#include "ntqregexp.h"

#include <stdio.h>
#ifndef Q_OS_TEMP
#include <time.h>
#endif

#if defined(Q_OS_WIN32)
#include <windows.h>
#endif

static const uint FIRST_DAY	= 2361222;	// Julian day for 1752-09-14
static const int  FIRST_YEAR	= 1752;		// ### wrong for many countries
static const uint SECS_PER_DAY	= 86400;
static const uint MSECS_PER_DAY = 86400000;
static const uint SECS_PER_HOUR = 3600;
static const uint MSECS_PER_HOUR= 3600000;
static const uint SECS_PER_MIN	= 60;
static const uint MSECS_PER_MIN = 60000;

static const short monthDays[] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static const char * const qt_shortMonthNames[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#ifndef TQT_NO_DATESTRING
/*****************************************************************************
  Some static function used by TQDate, TQTime and TQDateTime
 *****************************************************************************/

// Replaces tokens by their value. See TQDateTime::toString() for a list of valid tokens
static TQString getFmtString( const TQString& f, const TQTime* dt = 0, const TQDate* dd = 0, bool am_pm = FALSE )
{
    if ( f.isEmpty() )
	return TQString::null;

    TQString buf = f;

    if ( dt ) {
	if ( f == "h" ) {
	    if ( ( am_pm ) && ( dt->hour() > 12 ) )
		buf = TQString::number( dt->hour() - 12 );
	    else if ( ( am_pm ) && ( dt->hour() == 0 ) )
		buf = "12";
	    else
		buf = TQString::number( dt->hour() );
	} else if ( f == "hh" ) {
	    if ( ( am_pm ) && ( dt->hour() > 12 ) )
		buf = TQString::number( dt->hour() - 12 ).rightJustify( 2, '0', TRUE );
	    else if ( ( am_pm ) && ( dt->hour() == 0 ) )
		buf = "12";
	    else
		buf = TQString::number( dt->hour() ).rightJustify( 2, '0', TRUE );
	} else if ( f == "m" ) {
	    buf = TQString::number( dt->minute() );
	} else if ( f == "mm" ) {
	    buf = TQString::number( dt->minute() ).rightJustify( 2, '0', TRUE );
	} else if ( f == "s" ) {
	    buf = TQString::number( dt->second() );
	} else if ( f == "ss" ) {
	    buf = TQString::number( dt->second() ).rightJustify( 2, '0', TRUE );
	} else if ( f == "z" ) {
	    buf = TQString::number( dt->msec() );
	} else if ( f == "zzz" ) {
	    buf = TQString::number( dt->msec() ).rightJustify( 3, '0', TRUE );
	} else if ( f == "ap" ) {
	    buf = dt->hour() < 12 ? "am" : "pm";
	} else if ( f == "AP" ) {
	    buf = dt->hour() < 12 ? "AM" : "PM";
	}
    }

    if ( dd ) {
	if ( f == "d" ) {
	    buf = TQString::number( dd->day() );
	} else if ( f == "dd" ) {
	    buf = TQString::number( dd->day() ).rightJustify( 2, '0', TRUE );
	} else if ( f == "M" ) {
	    buf = TQString::number( dd->month() );
	} else if ( f == "MM" ) {
	    buf = TQString::number( dd->month() ).rightJustify( 2, '0', TRUE );
#ifndef TQT_NO_TEXTDATE
	} else if ( f == "ddd" ) {
	    buf = dd->shortDayName( dd->dayOfWeek() );
	} else if ( f == "dddd" ) {
	    buf = dd->longDayName( dd->dayOfWeek() );
	} else if ( f == "MMM" ) {
	    buf = dd->shortMonthName( dd->month() );
	} else if ( f == "MMMM" ) {
	    buf = dd->longMonthName( dd->month() );
#endif
	} else if ( f == "yy" ) {
	    buf = TQString::number( dd->year() ).right( 2 );
	} else if ( f == "yyyy" ) {
	    buf = TQString::number( dd->year() );
	}
    }

    return buf;
}

// Parses the format string and uses getFmtString to get the values for the tokens. Ret
static TQString fmtDateTime( const TQString& f, const TQTime* dt = 0, const TQDate* dd = 0 )
{
    if ( f.isEmpty() ) {
	return TQString::null;
    }

    if ( dt && !dt->isValid() )
	return TQString::null;
    if ( dd && !dd->isValid() )
	return TQString::null;

    bool ap = ( f.contains( "AP" ) || f.contains( "ap" ) );

    TQString buf;
    TQString frm;
    TQChar status = '0';

    for ( int i = 0; i < (int)f.length(); ++i ) {

	if ( f[ i ] == status ) {
	    if ( ( ap ) && ( ( f[ i ] == 'P' ) || ( f[ i ] == 'p' ) ) )
		status = '0';
	    frm += f[ i ];
	} else {
	    buf += getFmtString( frm, dt, dd, ap );
	    frm = TQString::null;
	    if ( ( f[ i ] == 'h' ) || ( f[ i ] == 'm' ) || ( f[ i ] == 's' ) || ( f[ i ] == 'z' ) ) {
		status = f[ i ];
		frm += f[ i ];
	    } else if ( ( f[ i ] == 'd' ) || ( f[ i ] == 'M' ) || ( f[ i ] == 'y' ) ) {
		status = f[ i ];
		frm += f[ i ];
	    } else if ( ( ap ) && ( f[ i ] == 'A' ) ) {
		status = 'P';
		frm += f[ i ];
	    } else  if( ( ap ) && ( f[ i ] == 'a' ) ) {
		status = 'p';
		frm += f[ i ];
	    } else {
		buf += f[ i ];
		status = '0';
	    }
	}
    }

    buf += getFmtString( frm, dt, dd, ap );

    return buf;
}
#endif // TQT_NO_DATESTRING

/*****************************************************************************
  TQDate member functions
 *****************************************************************************/

/*!
    \class TQDate ntqdatetime.h
    \reentrant
    \brief The TQDate class provides date functions.

    \ingroup time
    \mainclass

    A TQDate object contains a calendar date, i.e. year, month, and day
    numbers, in the modern Western (Gregorian) calendar. It can read
    the current date from the system clock. It provides functions for
    comparing dates and for manipulating dates, e.g. by adding a
    number of days or months or years.

    A TQDate object is typically created either by giving the year,
    month and day numbers explicitly, or by using the static function
    currentDate(), which creates a TQDate object containing the system
    clock's date. An explicit date can also be set using setYMD(). The
    fromString() function returns a TQDate given a string and a date
    format which is used to interpret the date within the string.

    The year(), month(), and day() functions provide access to the
    year, month, and day numbers. Also, dayOfWeek() and dayOfYear()
    functions are provided. The same information is provided in
    textual format by the toString(), shortDayName(), longDayName(),
    shortMonthName() and longMonthName() functions.

    TQDate provides a full set of operators to compare two TQDate
    objects where smaller means earlier and larger means later.

    You can increment (or decrement) a date by a given number of days
    using addDays(). Similarly you can use addMonths() and addYears().
    The daysTo() function returns the number of days between two
    dates.

    The daysInMonth() and daysInYear() functions return how many days
    there are in this date's month and year, respectively. The
    leapYear() function indicates whether this date is in a leap year.

    Note that TQDate should not be used for date calculations for dates
    prior to the introduction of the Gregorian calendar. This calendar
    was adopted by England from the 14<sup><small>th</small></sup>
    September 1752 (hence this is the earliest valid TQDate), and
    subsequently by most other Western countries, until 1923.

    The end of time is reached around the year 8000, by which time we
    expect TQt to be obsolete.

    \sa TQTime TQDateTime TQDateEdit TQDateTimeEdit
*/

/*!
    \enum TQt::DateFormat

    \value TextDate (default) TQt format
    \value ISODate ISO 8601 extended format (YYYY-MM-DD, or with time,
    YYYY-MM-DDTHH:MM:SS)
    \value LocalDate locale dependent format
*/


/*!
    \enum TQt::TimeSpec

    \value LocalTime Locale dependent time (Timezones and Daylight Savings Time)
    \value UTC Coordinated Universal Time, replaces Greenwich Time
*/

/*!
    \fn TQDate::TQDate()

    Constructs a null date. Null dates are invalid.

    \sa isNull(), isValid()
*/


/*!
    Constructs a date with year \a y, month \a m and day \a d.

    \a y must be in the range 1752..8000, \a m must be in the range
    1..12, and \a d must be in the range 1..31.

    \warning If \a y is in the range 0..99, it is interpreted as
    1900..1999.

    \sa isValid()
*/

TQDate::TQDate( int y, int m, int d )
{
    jd = 0;
    setYMD( y, m, d );
}


/*!
    \fn bool TQDate::isNull() const

    Returns TRUE if the date is null; otherwise returns FALSE. A null
    date is invalid.

    \sa isValid()
*/


/*!
    Returns TRUE if this date is valid; otherwise returns FALSE.

    \sa isNull()
*/

bool TQDate::isValid() const
{
    return jd >= FIRST_DAY;
}


/*!
    Returns the year (1752..8000) of this date.

    \sa month(), day()
*/

int TQDate::year() const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    return y;
}

/*!
    Returns the month (January=1..December=12) of this date.

    \sa year(), day()
*/

int TQDate::month() const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    return m;
}

/*!
    Returns the day of the month (1..31) of this date.

    \sa year(), month(), dayOfWeek()
*/

int TQDate::day() const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    return d;
}

/*!
    Returns the weekday (Monday=1..Sunday=7) for this date.

    \sa day(), dayOfYear()
*/

int TQDate::dayOfWeek() const
{
    return ( jd % 7 ) + 1;
}

/*!
    Returns the day of the year (1..365) for this date.

    \sa day(), dayOfWeek()
*/

int TQDate::dayOfYear() const
{
    return jd - gregorianToJulian(year(), 1, 1) + 1;
}

/*!
    Returns the number of days in the month (28..31) for this date.

    \sa day(), daysInYear()
*/

int TQDate::daysInMonth() const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    if ( m == 2 && leapYear(y) )
	return 29;
    else
	return monthDays[m];
}

/*!
    Returns the number of days in the year (365 or 366) for this date.

    \sa day(), daysInMonth()
*/

int TQDate::daysInYear() const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    return leapYear( y ) ? 366 : 365;
}

/*!
    Returns the week number (1 to 53), and stores the year in \a
    *yearNumber unless \a yearNumber is null (the default).

    Returns 0 if the date is invalid.

    In accordance with ISO 8601, weeks start on Monday and the first
    Thursday of a year is always in week 1 of that year. Most years
    have 52 weeks, but some have 53.

    \a *yearNumber is not always the same as year(). For example, 1
    January 2000 has week number 52 in the year 1999, and 31 December
    2002 has week number 1 in the year 2003.

    \legalese

    Copyright (c) 1989 The Regents of the University of California.
    All rights reserved.

    Redistribution and use in source and binary forms are permitted
    provided that the above copyright notice and this paragraph are
    duplicated in all such forms and that any documentation,
    advertising materials, and other materials related to such
    distribution and use acknowledge that the software was developed
    by the University of California, Berkeley.  The name of the
    University may not be used to endorse or promote products derived
    from this software without specific prior written permission.
    THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

    \sa isValid()
*/

int TQDate::weekNumber( int *yearNumber ) const
{
    if ( !isValid() )
	return 0;

    int	year = TQDate::year();
    int	yday = dayOfYear() - 1;
    int	wday = dayOfWeek();
    if (wday == 7)
    	wday = 0;
    int	w;

    for (;;) {
	int len;
	int bot;
	int top;

	len = leapYear(year) ? 366 : 365;
	/*
	** What yday (-3 ... 3) does
	** the ISO year begin on?
	*/
	bot = ((yday + 11 - wday) % 7) - 3;
	/*
	** What yday does the NEXT
	** ISO year begin on?
	*/
	top = bot - (len % 7);
	if (top < -3)
    	    top += 7;
	top += len;
	if (yday >= top) {
	    ++year;
    	    w = 1;
	    break;
	}
	if (yday >= bot) {
	    w = 1 + ((yday - bot) / 7);
	    break;
	}
	--year;
	yday += leapYear(year) ? 366 : 365;
    }
    if (yearNumber != 0)
    	*yearNumber = year;
    return w;
}

/*!
  \fn TQString TQDate::monthName( int month )
  \obsolete

  Use shortMonthName() instead.
*/
#ifndef TQT_NO_TEXTDATE
/*!
    Returns the name of the \a month.

    1 = "Jan", 2 = "Feb", ... 12 = "Dec"

    The month names will be localized according to the system's locale
    settings.

    \sa toString(), longMonthName(), shortDayName(), longDayName()
*/

TQString TQDate::shortMonthName( int month )
{
#if defined(QT_CHECK_RANGE)
    if ( month < 1 || month > 12 ) {
	tqWarning( "TQDate::shortMonthName: Parameter out ouf range" );
	month = 1;
    }
#endif
#ifndef TQ_WS_WIN
    char buffer[255];
    tm tt;
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_mon = month - 1;
    if ( strftime( buffer, sizeof( buffer ), "%b", &tt ) )
	return TQString::fromLocal8Bit( buffer );
#else
    SYSTEMTIME st;
    memset( &st, 0, sizeof(SYSTEMTIME) );
    st.wYear = 2000;
    st.wMonth = month;
    st.wDay = 1;
    const wchar_t mmm_t[] = L"MMM"; // workaround for Borland
    QT_WA( {
	TCHAR buf[255];
	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, mmm_t, buf, 255 ) )
	    return TQString::fromUcs2( (ushort*)buf );
    } , {
	char buf[255];
	if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, "MMM", (char*)&buf, 255 ) )
	    return TQString::fromLocal8Bit( buf );
    } );
#endif
    return TQString::null;
}

/*!
    Returns the long name of the \a month.

    1 = "January", 2 = "February", ... 12 = "December"

    The month names will be localized according to the system's locale
    settings.

    \sa toString(), shortMonthName(), shortDayName(), longDayName()
*/

TQString TQDate::longMonthName( int month )
{
#if defined(QT_CHECK_RANGE)
    if ( month < 1 || month > 12 ) {
	tqWarning( "TQDate::longMonthName: Parameter out ouf range" );
	month = 1;
    }
#endif
#ifndef TQ_WS_WIN
    char buffer[255];
    tm tt;
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_mon = month - 1;
    if ( strftime( buffer, sizeof( buffer ), "%B", &tt ) )
	return TQString::fromLocal8Bit( buffer );
#else
    SYSTEMTIME st;
    memset( &st, 0, sizeof(SYSTEMTIME) );
    st.wYear = 2000;
    st.wMonth = month;
    st.wDay = 1 ;
    const wchar_t mmmm_t[] = L"MMMM"; // workaround for Borland
    QT_WA( {
	TCHAR buf[255];
	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, mmmm_t, buf, 255 ) )
	    return TQString::fromUcs2( (ushort*)buf );
    } , {
	char buf[255];
	if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, "MMMM", (char*)&buf, 255 ) )
	    return TQString::fromLocal8Bit( buf );
    } )
#endif

    return TQString::null;
}

/*!
  \fn TQString TQDate::dayName( int weekday )
  \obsolete

  Use shortDayName() instead.
*/

/*!
    Returns the name of the \a weekday.

    1 = "Mon", 2 = "Tue", ... 7 = "Sun"

    The day names will be localized according to the system's locale
    settings.

    \sa toString(), shortMonthName(), longMonthName(), longDayName()
*/

TQString TQDate::shortDayName( int weekday )
{
#if defined(QT_CHECK_RANGE)
    if ( weekday < 1 || weekday > 7 ) {
	tqWarning( "TQDate::shortDayName: Parameter out of range" );
	weekday = 1;
    }
#endif
#ifndef TQ_WS_WIN
    char buffer[255];
    tm tt;
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_wday = ( weekday == 7 ) ? 0 : weekday;
    if ( strftime( buffer, sizeof( buffer ), "%a", &tt ) )
	return TQString::fromLocal8Bit( buffer );
#else
    SYSTEMTIME st;
    memset( &st, 0, sizeof(SYSTEMTIME) );
    st.wYear = 2001;
    st.wMonth = 10;
    st.wDayOfWeek = ( weekday == 7 ) ? 0 : weekday;
    st.wDay = 21 + st.wDayOfWeek;
    const wchar_t ddd_t[] = L"ddd"; // workaround for Borland
    QT_WA( {
	TCHAR buf[255];
	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, ddd_t, buf, 255 ) )
	    return TQString::fromUcs2( (ushort*)buf );
    } , {
	char buf[255];
	if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, "ddd", (char*)&buf, 255 ) )
	    return TQString::fromLocal8Bit( buf );
    } );
#endif

    return TQString::null;
}

/*!
    Returns the long name of the \a weekday.

    1 = "Monday", 2 = "Tuesday", ... 7 = "Sunday"

    The day names will be localized according to the system's locale
    settings.

    \sa toString(), shortDayName(), shortMonthName(), longMonthName()
*/

TQString TQDate::longDayName( int weekday )
{
#if defined(QT_CHECK_RANGE)
    if ( weekday < 1 || weekday > 7 ) {
	tqWarning( "TQDate::longDayName: Parameter out of range" );
	weekday = 1;
    }
#endif
#ifndef TQ_WS_WIN
    char buffer[255];
    tm tt;
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_wday = ( weekday == 7 ) ? 0 : weekday;
    if ( strftime( buffer, sizeof( buffer ), "%A", &tt ) )
	return TQString::fromLocal8Bit( buffer );
#else
    SYSTEMTIME st;
    memset( &st, 0, sizeof(SYSTEMTIME) );
    st.wYear = 2001;
    st.wMonth = 10;
    st.wDayOfWeek = ( weekday == 7 ) ? 0 : weekday;
    st.wDay = 21 + st.wDayOfWeek;
    const wchar_t dddd_t[] = L"dddd"; // workaround for Borland
    QT_WA( {
	TCHAR buf[255];
	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, dddd_t, buf, 255 ) )
	    return TQString::fromUcs2( (ushort*)buf );
    } , {
	char buf[255];
	if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, "dddd", (char*)&buf, 255 ) )
	    return TQString::fromLocal8Bit( buf );
    } );
#endif

    return TQString::null;
}
#endif //TQT_NO_TEXTDATE

#ifndef TQT_NO_DATESTRING

#if !defined(TQT_NO_SPRINTF)
/*!
    \overload

    Returns the date as a string. The \a f parameter determines the
    format of the string.

    If \a f is \c TQt::TextDate, the string format is "Sat May 20 1995"
    (using the shortDayName() and shortMonthName() functions to
    generate the string, so the day and month names are locale
    specific).

    If \a f is \c TQt::ISODate, the string format corresponds to the
    ISO 8601 specification for representations of dates, which is
    YYYY-MM-DD where YYYY is the year, MM is the month of the year
    (between 01 and 12), and DD is the day of the month between 01 and
    31.

    If \a f is \c TQt::LocalDate, the string format depends on the
    locale settings of the system.

    If the date is an invalid date, then TQString::null will be returned.

    \sa shortDayName(), shortMonthName()
*/
TQString TQDate::toString( TQt::DateFormat f ) const
{
    if ( !isValid() )
	return TQString::null;
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    switch ( f ) {
    case TQt::LocalDate:
	{
#ifndef TQ_WS_WIN
	    tm tt;
	    memset( &tt, 0, sizeof( tm ) );
	    char buf[255];
	    tt.tm_mday = day();
	    tt.tm_mon = month() - 1;
	    tt.tm_year = year() - 1900;

	    static const char * avoidEgcsWarning = "%x";
	    if ( strftime( buf, sizeof(buf), avoidEgcsWarning, &tt ) )
		return TQString::fromLocal8Bit( buf );
#else
	    SYSTEMTIME st;
	    memset( &st, 0, sizeof(SYSTEMTIME) );
	    st.wYear = year();
	    st.wMonth = month();
	    st.wDay = day();
	    QT_WA( {
		TCHAR buf[255];
		if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, 0, buf, 255 ) )
		    return TQString::fromUcs2( (ushort*)buf );
	    } , {
		char buf[255];
		if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, 0, (char*)&buf, 255 ) )
		    return TQString::fromLocal8Bit( buf );
	    } );
#endif
	    return TQString::null;
	}
    default:
#ifndef TQT_NO_TEXTDATE
    case TQt::TextDate:
	{
	    TQString buf = shortDayName( dayOfWeek() );
	    buf += ' ';
	    buf += shortMonthName( m );
	    TQString t;
	    t.sprintf( " %d %d", d, y );
	    buf += t;
	    return buf;
	}
#endif
    case TQt::ISODate:
	{
	    TQString month( TQString::number( m ).rightJustify( 2, '0' ) );
	    TQString day( TQString::number( d ).rightJustify( 2, '0' ) );
	    return TQString::number( y ) + "-" + month + "-" + day;
	}
    }
}
#endif //TQT_NO_SPRINTF

/*!
    Returns the date as a string. The \a format parameter determines
    the format of the result string.

    These expressions may be used:

    \table
    \header \i Expression \i Output
    \row \i d \i the day as number without a leading zero (1-31)
    \row \i dd \i the day as number with a leading zero (01-31)
    \row \i ddd
	 \i the abbreviated localized day name (e.g. 'Mon'..'Sun').
	    Uses TQDate::shortDayName().
    \row \i dddd
	 \i the long localized day name (e.g. 'Monday'..'Sunday').
	    Uses TQDate::longDayName().
    \row \i M \i the month as number without a leading zero (1-12)
    \row \i MM \i the month as number with a leading zero (01-12)
    \row \i MMM
	 \i the abbreviated localized month name (e.g. 'Jan'..'Dec').
	    Uses TQDate::shortMonthName().
    \row \i MMMM
	 \i the long localized month name (e.g. 'January'..'December').
	    Uses TQDate::longMonthName().
    \row \i yy \i the year as two digit number (00-99)
    \row \i yyyy \i the year as four digit number (1752-8000)
    \endtable

    All other input characters will be ignored.

    Example format strings (assuming that the TQDate is the
    20<sup><small>th</small></sup> July 1969):
    \table
    \header \i Format \i Result
    \row \i dd.MM.yyyy	  \i11 20.07.1969
    \row \i ddd MMMM d yy \i11 Sun July 20 69
    \endtable

    If the date is an invalid date, then TQString::null will be returned.

    \sa TQDateTime::toString() TQTime::toString()

*/
TQString TQDate::toString( const TQString& format ) const
{
    return fmtDateTime( format, 0, this );
}
#endif //TQT_NO_DATESTRING

/*!
    Sets the date's year \a y, month \a m and day \a d.

    \a y must be in the range 1752..8000, \a m must be in the range
    1..12, and \a d must be in the range 1..31.

    \warning If \a y is in the range 0..99, it is interpreted as
    1900..1999.

    Returns TRUE if the date is valid; otherwise returns FALSE.
*/

bool TQDate::setYMD( int y, int m, int d )
{
    if ( year() == y && month() == m && day() == d )
	return isValid();
    if ( !isValid(y,m,d) ) {
#if defined(QT_CHECK_RANGE)
	 tqWarning( "TQDate::setYMD: Invalid date %04d-%02d-%02d", y, m, d );
#endif
	 return FALSE;
    }
    jd = gregorianToJulian( y, m, d );
    return TRUE;
}

/*!
    Returns a TQDate object containing a date \a ndays later than the
    date of this object (or earlier if \a ndays is negative).

    \sa addMonths() addYears() daysTo()
*/

TQDate TQDate::addDays( int ndays ) const
{
    TQDate d;
    d.jd = jd + ndays;
    return d;
}

/*!
    Returns a TQDate object containing a date \a nmonths later than the
    date of this object (or earlier if \a nmonths is negative).

    \sa addDays() addYears()
*/

TQDate TQDate::addMonths( int nmonths ) const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );

    while ( nmonths != 0 ) {
	if ( nmonths < 0 && nmonths + 12 <= 0 ) {
	    y--;
	    nmonths+=12;
	} else if ( nmonths < 0 ) {
	    m+= nmonths;
	    nmonths = 0;
	    if ( m <= 0 ) {
		--y;
		m+=12;
	    }
	} else if ( nmonths - 12 >= 0 ) {
	    y++;
	    nmonths-=12;
	} else if ( m == 12 ) {
	    y++;
	    m = 0;
	} else {
	    m+= nmonths;
	    nmonths = 0;
	    if ( m > 12 ) {
		++y;
		m -= 12;
	    }
	}
    }

    TQDate tmp(y,m,1);

    if( d > tmp.daysInMonth() )
	d = tmp.daysInMonth();

    TQDate date(y, m, d);
    return date;

}

/*!
    Returns a TQDate object containing a date \a nyears later than the
    date of this object (or earlier if \a nyears is negative).

    \sa addDays(), addMonths()
*/

TQDate TQDate::addYears( int nyears ) const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    y += nyears;

    TQDate tmp(y,m,1);

    if( d > tmp.daysInMonth() )
	d = tmp.daysInMonth();

    TQDate date(y, m, d);
    return date;
}



/*!
    Returns the number of days from this date to \a d (which is
    negative if \a d is earlier than this date).

    Example:
    \code
	TQDate d1( 1995, 5, 17 );  // May 17th 1995
	TQDate d2( 1995, 5, 20 );  // May 20th 1995
	d1.daysTo( d2 );          // returns 3
	d2.daysTo( d1 );          // returns -3
    \endcode

    \sa addDays()
*/

int TQDate::daysTo( const TQDate &d ) const
{
    return d.jd - jd;
}


/*!
    \fn bool TQDate::operator==( const TQDate &d ) const

    Returns TRUE if this date is equal to \a d; otherwise returns FALSE.
*/

/*!
    \fn bool TQDate::operator!=( const TQDate &d ) const

    Returns TRUE if this date is different from \a d; otherwise returns FALSE.
*/

/*!
    \fn bool TQDate::operator<( const TQDate &d ) const

    Returns TRUE if this date is earlier than \a d, otherwise returns FALSE.
*/

/*!
    \fn bool TQDate::operator<=( const TQDate &d ) const

    Returns TRUE if this date is earlier than or equal to \a d,
    otherwise returns FALSE.
*/

/*!
    \fn bool TQDate::operator>( const TQDate &d ) const

    Returns TRUE if this date is later than \a d, otherwise returns FALSE.
*/

/*!
    \fn bool TQDate::operator>=( const TQDate &d ) const

    Returns TRUE if this date is later than or equal to \a d,
    otherwise returns FALSE.
*/

/*!
    \overload
    Returns the current date, as reported by the system clock.

    \sa TQTime::currentTime(), TQDateTime::currentDateTime()
*/

TQDate TQDate::currentDate()
{
    return currentDate( TQt::LocalTime );
}

/*!
  Returns the current date, as reported by the system clock, for the
  TimeSpec \a ts. The default TimeSpec is LocalTime.

  \sa TQTime::currentTime(), TQDateTime::currentDateTime(), TQt::TimeSpec
*/
TQDate TQDate::currentDate( TQt::TimeSpec ts )
{
    TQDate d;
#if defined(Q_OS_WIN32)
    SYSTEMTIME t;
    memset( &t, 0, sizeof(SYSTEMTIME) );
    if ( ts == TQt::LocalTime )
	GetLocalTime( &t );
    else
	GetSystemTime( &t );
    d.jd = gregorianToJulian( t.wYear, t.wMonth, t.wDay );
#else
    // posix compliant system
    time_t ltime;
    time( &ltime );
    tm *t;

#  if defined(TQT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant versions of localtime() and gmtime() where available
    tm res;
    if ( ts == TQt::LocalTime )
	t = localtime_r( &ltime, &res );
    else
	t = gmtime_r( &ltime, &res );
#  else
    if ( ts == TQt::LocalTime )
	t = localtime( &ltime );
    else
	t = gmtime( &ltime );
#  endif // TQT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS

    d.jd = gregorianToJulian( t->tm_year + 1900, t->tm_mon + 1, t->tm_mday );
#endif
    return d;
}

#ifndef TQT_NO_DATESTRING
/*!
    Returns the TQDate represented by the string \a s, using the format
    \a f, or an invalid date if the string cannot be parsed.

    Note for \c TQt::TextDate: It is recommended that you use the
    English short month names (e.g. "Jan"). Although localized month
    names can also be used, they depend on the user's locale settings.

    \warning \c TQt::LocalDate cannot be used here.
*/
TQDate TQDate::fromString( const TQString& s, TQt::DateFormat f )
{
    if ( ( s.isEmpty() ) || ( f == TQt::LocalDate ) ) {
#if defined(QT_CHECK_RANGE)
	tqWarning( "TQDate::fromString: Parameter out of range" );
#endif
	TQDate d;
	d.jd = 0;
	return d;
    }
    switch ( f ) {
    case TQt::ISODate:
	{
	    int year( s.mid( 0, 4 ).toInt() );
	    int month( s.mid( 5, 2 ).toInt() );
	    int day( s.mid( 8, 2 ).toInt() );
	    if ( year && month && day )
		return TQDate( year, month, day );
	}
	break;
    default:
#ifndef TQT_NO_TEXTDATE
    case TQt::TextDate:
	{
	    /*
	      This will fail gracefully if the input string doesn't
	      contain any space.
	    */
	    int monthPos = s.find( ' ' ) + 1;
	    int dayPos = s.find( ' ', monthPos ) + 1;

	    TQString monthName( s.mid(monthPos, dayPos - monthPos - 1) );
	    int month = -1;

	    // try English names first
	    for ( int i = 0; i < 12; i++ ) {
		if ( monthName == qt_shortMonthNames[i] ) {
		    month = i + 1;
		    break;
		}
	    }

	    // try the localized names
	    if ( month == -1 ) {
		for ( int i = 0; i < 12; i++ ) {
		    if ( monthName == shortMonthName( i + 1 ) ) {
			month = i + 1;
			break;
		    }
		}
	    }
#if defined(QT_CHECK_RANGE)
	    if ( month < 1 || month > 12 ) {
		tqWarning( "TQDate::fromString: Parameter out of range" );
		TQDate d;
		d.jd = 0;
		return d;
	    }
#endif
	    int day = s.mid( dayPos, 2 ).stripWhiteSpace().toInt();
	    int year = s.right( 4 ).toInt();
	    return TQDate( year, month, day );
	}
#else
	break;
#endif
    }
    return TQDate();
}
#endif //TQT_NO_DATESTRING

/*!
    \overload

    Returns TRUE if the specified date (year \a y, month \a m and day
    \a d) is valid; otherwise returns FALSE.

    Example:
    \code
    TQDate::isValid( 2002, 5, 17 );  // TRUE   May 17th 2002 is valid
    TQDate::isValid( 2002, 2, 30 );  // FALSE  Feb 30th does not exist
    TQDate::isValid( 2004, 2, 29 );  // TRUE   2004 is a leap year
    TQDate::isValid( 1202, 6, 6 );   // FALSE  1202 is pre-Gregorian
    \endcode

    \warning A \a y value in the range 00..99 is interpreted as
    1900..1999.

    \sa isNull(), setYMD()
*/

bool TQDate::isValid( int y, int m, int d )
{
    if ( y >= 0 && y <= 99 )
	y += 1900;
    else if ( y < FIRST_YEAR || (y == FIRST_YEAR && (m < 9 ||
						    (m == 9 && d < 14))) )
	return FALSE;
    return (d > 0 && m > 0 && m <= 12) &&
	   (d <= monthDays[m] || (d == 29 && m == 2 && leapYear(y)));
}

/*!
    Returns TRUE if the specified year \a y is a leap year; otherwise
    returns FALSE.
*/

bool TQDate::leapYear( int y )
{
    return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
}

/*!
  \internal
  Converts a Gregorian date to a Julian day.
  This algorithm is taken from Communications of the ACM, Vol 6, No 8.
  \sa julianToGregorian()
*/

uint TQDate::gregorianToJulian( int y, int m, int d )
{
    uint c, ya;
    if ( y <= 99 )
	y += 1900;
    if ( m > 2 ) {
	m -= 3;
    } else {
	m += 9;
	y--;
    }
    c = y;					// NOTE: Sym C++ 6.0 bug
    c /= 100;
    ya = y - 100*c;
    return 1721119 + d + (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5;
}

/*!
  \internal
  Converts a Julian day to a Gregorian date.
  This algorithm is taken from Communications of the ACM, Vol 6, No 8.
  \sa gregorianToJulian()
*/

void TQDate::julianToGregorian( uint jd, int &y, int &m, int &d )
{
    uint x;
    uint j = jd - 1721119;
    y = (j*4 - 1)/146097;
    j = j*4 - 146097*y - 1;
    x = j/4;
    j = (x*4 + 3) / 1461;
    y = 100*y + j;
    x = (x*4) + 3 - 1461*j;
    x = (x + 4)/4;
    m = (5*x - 3)/153;
    x = 5*x - 3 - 153*m;
    d = (x + 5)/5;
    if ( m < 10 ) {
	m += 3;
    } else {
	m -= 9;
	y++;
    }
}


/*****************************************************************************
  TQTime member functions
 *****************************************************************************/

/*!
    \class TQTime ntqdatetime.h
    \reentrant

    \brief The TQTime class provides clock time functions.

    \ingroup time
    \mainclass

    A TQTime object contains a clock time, i.e. the number of hours,
    minutes, seconds, and milliseconds since midnight. It can read the
    current time from the system clock and measure a span of elapsed
    time. It provides functions for comparing times and for
    manipulating a time by adding a number of (milli)seconds.

    TQTime uses the 24-hour clock format; it has no concept of AM/PM.
    It operates in local time; it knows nothing about time zones or
    daylight savings time.

    A TQTime object is typically created either by giving the number of
    hours, minutes, seconds, and milliseconds explicitly, or by using
    the static function currentTime(), which creates a TQTime object
    that contains the system's clock time. Note that the accuracy
    depends on the accuracy of the underlying operating system; not
    all systems provide 1-millisecond accuracy.

    The hour(), minute(), second(), and msec() functions provide
    access to the number of hours, minutes, seconds, and milliseconds
    of the time. The same information is provided in textual format by
    the toString() function.

    TQTime provides a full set of operators to compare two TQTime
    objects. One time is considered smaller than another if it is
    earlier than the other.

    The time a given number of seconds or milliseconds later than a
    given time can be found using the addSecs() or addMSecs()
    functions. Correspondingly, the number of (milli)seconds between
    two times can be found using the secsTo() or msecsTo() functions.

    TQTime can be used to measure a span of elapsed time using the
    start(), restart(), and elapsed() functions.

    \sa TQDate, TQDateTime
*/

/*!
    \fn TQTime::TQTime()

    Constructs the time 0 hours, minutes, seconds and milliseconds,
    i.e. 00:00:00.000 (midnight). This is a valid time.

    \sa isValid()
*/

/*!
    Constructs a time with hour \a h, minute \a m, seconds \a s and
    milliseconds \a ms.

    \a h must be in the range 0..23, \a m and \a s must be in the
    range 0..59, and \a ms must be in the range 0..999.

    \sa isValid()
*/

TQTime::TQTime( int h, int m, int s, int ms )
{
    setHMS( h, m, s, ms );
}


/*!
    \fn bool TQTime::isNull() const

    Returns TRUE if the time is equal to 00:00:00.000; otherwise
    returns FALSE. A null time is valid.

    \sa isValid()
*/

/*!
    Returns TRUE if the time is valid; otherwise returns FALSE. The
    time 23:30:55.746 is valid, whereas 24:12:30 is invalid.

    \sa isNull()
*/

bool TQTime::isValid() const
{
    return ds < MSECS_PER_DAY;
}


/*!
    Returns the hour part (0..23) of the time.
*/

int TQTime::hour() const
{
    return ds / MSECS_PER_HOUR;
}

/*!
    Returns the minute part (0..59) of the time.
*/

int TQTime::minute() const
{
    return (ds % MSECS_PER_HOUR)/MSECS_PER_MIN;
}

/*!
    Returns the second part (0..59) of the time.
*/

int TQTime::second() const
{
    return (ds / 1000)%SECS_PER_MIN;
}

/*!
    Returns the millisecond part (0..999) of the time.
*/

int TQTime::msec() const
{
    return ds % 1000;
}

#ifndef TQT_NO_DATESTRING
#ifndef TQT_NO_SPRINTF
/*!
    \overload

    Returns the time as a string. Milliseconds are not included. The
    \a f parameter determines the format of the string.

    If \a f is \c TQt::TextDate, the string format is HH:MM:SS; e.g. 1
    second before midnight would be "23:59:59".

    If \a f is \c TQt::ISODate, the string format corresponds to the
    ISO 8601 extended specification for representations of dates,
    which is also HH:MM:SS.

    If \a f is TQt::LocalDate, the string format depends on the locale
    settings of the system.

    If the time is an invalid time, then TQString::null will be returned.
*/

TQString TQTime::toString( TQt::DateFormat f ) const
{
    if ( !isValid() )
	return TQString::null;

    switch ( f ) {
    case TQt::LocalDate:
	{
#ifndef TQ_WS_WIN
	    tm tt;
	    memset( &tt, 0, sizeof( tm ) );
	    char buf[255];
	    tt.tm_sec = second();
	    tt.tm_min = minute();
	    tt.tm_hour = hour();
	    if ( strftime( buf, sizeof(buf), "%X", &tt ) )
		return TQString::fromLocal8Bit( buf );
#else
	    SYSTEMTIME st;
	    memset( &st, 0, sizeof(SYSTEMTIME) );
	    st.wHour = hour();
	    st.wMinute = minute();
	    st.wSecond = second();
	    st.wMilliseconds = 0;
	    QT_WA( {
		TCHAR buf[255];
		if ( GetTimeFormat( LOCALE_USER_DEFAULT, 0, &st, 0, buf, 255 ) )
		    return TQString::fromUcs2( (ushort*)buf );
	    } , {
		char buf[255];
		if ( GetTimeFormatA( LOCALE_USER_DEFAULT, 0, &st, 0, (char*)&buf, 255 ) )
		    return TQString::fromLocal8Bit( buf );
	    } );
#endif
	    return TQString::null;
	}
    default:
    case TQt::ISODate:
    case TQt::TextDate:
	TQString buf;
	buf.sprintf( "%.2d:%.2d:%.2d", hour(), minute(), second() );
	return buf;
    }
}
#endif

/*!
    Returns the time as a string. The \a format parameter determines
    the format of the result string.

    These expressions may be used:

    \table
    \header \i Expression \i Output
    \row \i h
	 \i the hour without a leading zero (0..23 or 1..12 if AM/PM display)
    \row \i hh
	 \i the hour with a leading zero (00..23 or 01..12 if AM/PM display)
    \row \i m \i the minute without a leading zero (0..59)
    \row \i mm \i the minute with a leading zero (00..59)
    \row \i s \i the second whithout a leading zero (0..59)
    \row \i ss \i the second whith a leading zero (00..59)
    \row \i z \i the milliseconds without leading zeroes (0..999)
    \row \i zzz \i the milliseconds with leading zeroes (000..999)
    \row \i AP
	 \i use AM/PM display. \e AP will be replaced by either "AM" or "PM".
    \row \i ap
	 \i use am/pm display. \e ap will be replaced by either "am" or "pm".
    \endtable

    All other input characters will be ignored.

    Example format strings (assuming that the TQTime is 14:13:09.042)

    \table
    \header \i Format \i Result
    \row \i hh:mm:ss.zzz    \i11 14:13:09.042
    \row \i h:m:s ap	    \i11 2:13:9 pm
    \endtable

    If the time is an invalid time, then TQString::null will be returned.

    \sa TQDate::toString() TQDateTime::toString()
*/
TQString TQTime::toString( const TQString& format ) const
{
    return fmtDateTime( format, this, 0 );
}
#endif //TQT_NO_DATESTRING
/*!
    Sets the time to hour \a h, minute \a m, seconds \a s and
    milliseconds \a ms.

    \a h must be in the range 0..23, \a m and \a s must be in the
    range 0..59, and \a ms must be in the range 0..999. Returns TRUE
    if the set time is valid; otherwise returns FALSE.

    \sa isValid()
*/

bool TQTime::setHMS( int h, int m, int s, int ms )
{
    if ( !isValid(h,m,s,ms) ) {
#if defined(QT_CHECK_RANGE)
	tqWarning( "TQTime::setHMS Invalid time %02d:%02d:%02d.%03d", h, m, s,
		 ms );
#endif
	ds = MSECS_PER_DAY;		// make this invalid
	return FALSE;
    }
    ds = (h*SECS_PER_HOUR + m*SECS_PER_MIN + s)*1000 + ms;
    return TRUE;
}

/*!
    Returns a TQTime object containing a time \a nsecs seconds later
    than the time of this object (or earlier if \a nsecs is negative).

    Note that the time will wrap if it passes midnight.

    Example:
    \code
    TQTime n( 14, 0, 0 );                // n == 14:00:00
    TQTime t;
    t = n.addSecs( 70 );                // t == 14:01:10
    t = n.addSecs( -70 );               // t == 13:58:50
    t = n.addSecs( 10*60*60 + 5 );      // t == 00:00:05
    t = n.addSecs( -15*60*60 );         // t == 23:00:00
    \endcode

    \sa addMSecs(), secsTo(), TQDateTime::addSecs()
*/

TQTime TQTime::addSecs( int nsecs ) const
{
    return addMSecs( nsecs * 1000 );
}

/*!
    Returns the number of seconds from this time to \a t (which is
    negative if \a t is earlier than this time).

    Because TQTime measures time within a day and there are 86400
    seconds in a day, the result is always between -86400 and 86400.

    \sa addSecs() TQDateTime::secsTo()
*/

int TQTime::secsTo( const TQTime &t ) const
{
    return ((int)t.ds - (int)ds)/1000;
}

/*!
    Returns a TQTime object containing a time \a ms milliseconds later
    than the time of this object (or earlier if \a ms is negative).

    Note that the time will wrap if it passes midnight. See addSecs()
    for an example.

    \sa addSecs(), msecsTo()
*/

TQTime TQTime::addMSecs( int ms ) const
{
    TQTime t;
    if ( ms < 0 ) {
	// % not well-defined for -ve, but / is.
	int negdays = (MSECS_PER_DAY-ms) / MSECS_PER_DAY;
	t.ds = ((int)ds + ms + negdays*MSECS_PER_DAY)
		% MSECS_PER_DAY;
    } else {
	t.ds = ((int)ds + ms) % MSECS_PER_DAY;
    }
    return t;
}

/*!
    Returns the number of milliseconds from this time to \a t (which
    is negative if \a t is earlier than this time).

    Because TQTime measures time within a day and there are 86400
    seconds in a day, the result is always between -86400000 and
    86400000 msec.

    \sa secsTo()
*/

int TQTime::msecsTo( const TQTime &t ) const
{
    return (int)t.ds - (int)ds;
}


/*!
    \fn bool TQTime::operator==( const TQTime &t ) const

    Returns TRUE if this time is equal to \a t; otherwise returns FALSE.
*/

/*!
    \fn bool TQTime::operator!=( const TQTime &t ) const

    Returns TRUE if this time is different from \a t; otherwise returns FALSE.
*/

/*!
    \fn bool TQTime::operator<( const TQTime &t ) const

    Returns TRUE if this time is earlier than \a t; otherwise returns FALSE.
*/

/*!
    \fn bool TQTime::operator<=( const TQTime &t ) const

    Returns TRUE if this time is earlier than or equal to \a t;
    otherwise returns FALSE.
*/

/*!
    \fn bool TQTime::operator>( const TQTime &t ) const

    Returns TRUE if this time is later than \a t; otherwise returns FALSE.
*/

/*!
    \fn bool TQTime::operator>=( const TQTime &t ) const

    Returns TRUE if this time is later than or equal to \a t;
    otherwise returns FALSE.
*/



/*!
    \overload

    Returns the current time as reported by the system clock.

    Note that the accuracy depends on the accuracy of the underlying
    operating system; not all systems provide 1-millisecond accuracy.
*/

TQTime TQTime::currentTime()
{
    return currentTime( TQt::LocalTime );
}

/*!
  Returns the current time as reported by the system clock, for the
  TimeSpec \a ts. The default TimeSpec is LocalTime.

  Note that the accuracy depends on the accuracy of the underlying
  operating system; not all systems provide 1-millisecond accuracy.

  \sa TQt::TimeSpec
*/
TQTime TQTime::currentTime( TQt::TimeSpec ts )
{
    TQTime t;
    currentTime( &t, ts );
    return t;
}

#ifndef TQT_NO_DATESTRING
/*!
    Returns the representation \a s as a TQTime using the format \a f,
    or an invalid time if this is not possible.

    \warning Note that \c TQt::LocalDate cannot be used here.
*/
TQTime TQTime::fromString( const TQString& s, TQt::DateFormat f )
{
    if ( ( s.isEmpty() ) || ( f == TQt::LocalDate ) ) {
#if defined(QT_CHECK_RANGE)
	tqWarning( "TQTime::fromString: Parameter out of range" );
#endif
	TQTime t;
	t.ds = MSECS_PER_DAY;
	return t;
    }

    int hour( s.mid( 0, 2 ).toInt() );
    int minute( s.mid( 3, 2 ).toInt() );
    int second( s.mid( 6, 2 ).toInt() );
    int msec( s.mid( 9, 3 ).toInt() );
    return TQTime( hour, minute, second, msec );
}
#endif

/*!
  \internal
  \obsolete

  Fetches the current time and returns TRUE if the time is within one
  minute after midnight, otherwise FALSE. The return value is used by
  TQDateTime::currentDateTime() to ensure that the date there is correct.
*/

bool TQTime::currentTime( TQTime *ct )
{
    return currentTime( ct, TQt::LocalTime );
}


/*!
  \internal

  Fetches the current time, for the TimeSpec \a ts, and returns TRUE
  if the time is within one minute after midnight, otherwise FALSE. The
  return value is used by TQDateTime::currentDateTime() to ensure that
  the date there is correct. The default TimeSpec is LocalTime.

  \sa TQt::TimeSpec
*/
bool TQTime::currentTime( TQTime *ct, TQt::TimeSpec ts )
{
    if ( !ct ) {
#if defined(QT_CHECK_NULL)
	tqWarning( "TQTime::currentTime(TQTime *): Null pointer not allowed" );
#endif
	return FALSE;
    }

#if defined(Q_OS_WIN32)
    SYSTEMTIME t;
    if ( ts == TQt::LocalTime ) {
	GetLocalTime( &t );
    } else {
	GetSystemTime( &t );
    }
    ct->ds = (uint)( MSECS_PER_HOUR*t.wHour + MSECS_PER_MIN*t.wMinute +
		     1000*t.wSecond + t.wMilliseconds );
#elif defined(Q_OS_UNIX)
    // posix compliant system
    struct timeval tv;
    gettimeofday( &tv, 0 );
    time_t ltime = tv.tv_sec;
    tm *t;

#  if defined(TQT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant versions of localtime() and gmtime() where available
    tm res;
    if ( ts == TQt::LocalTime )
	t = localtime_r( &ltime, &res );
    else
	t = gmtime_r( &ltime, &res );
#  else
    if ( ts == TQt::LocalTime )
	t = localtime( &ltime );
    else
	t = gmtime( &ltime );
#  endif // TQT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS

    ct->ds = (uint)( MSECS_PER_HOUR * t->tm_hour + MSECS_PER_MIN * t->tm_min +
		     1000 * t->tm_sec + tv.tv_usec / 1000 );
#else
    time_t ltime; // no millisecond resolution
    ::time( &ltime );
    tm *t;
    if ( ts == TQt::LocalTime )
	localtime( &ltime );
    else
	gmtime( &ltime );
    ct->ds = (uint) ( MSECS_PER_HOUR * t->tm_hour + MSECS_PER_MIN * t->tm_min +
		      1000 * t->tm_sec );
#endif
    // 00:00.00 to 00:00.59.999 is considered as "midnight or right after"
    return ct->ds < (uint) MSECS_PER_MIN;
}

/*!
    \overload

    Returns TRUE if the specified time is valid; otherwise returns
    FALSE.

    The time is valid if \a h is in the range 0..23, \a m and \a s are
    in the range 0..59, and \a ms is in the range 0..999.

    Example:
    \code
    TQTime::isValid(21, 10, 30); // returns TRUE
    TQTime::isValid(22, 5,  62); // returns FALSE
    \endcode
*/

bool TQTime::isValid( int h, int m, int s, int ms )
{
    return (uint)h < 24 && (uint)m < 60 && (uint)s < 60 && (uint)ms < 1000;
}


/*!
    Sets this time to the current time. This is practical for timing:

    \code
    TQTime t;
    t.start();
    some_lengthy_task();
    tqDebug( "Time elapsed: %d ms", t.elapsed() );
    \endcode

    \sa restart(), elapsed(), currentTime()
*/

void TQTime::start()
{
    *this = currentTime();
}

/*!
    Sets this time to the current time and returns the number of
    milliseconds that have elapsed since the last time start() or
    restart() was called.

    This function is guaranteed to be atomic and is thus very handy
    for repeated measurements. Call start() to start the first
    measurement and then restart() for each later measurement.

    Note that the counter wraps to zero 24 hours after the last call
    to start() or restart().

    \warning If the system's clock setting has been changed since the
    last time start() or restart() was called, the result is
    undefined. This can happen when daylight savings time is turned on
    or off.

    \sa start(), elapsed(), currentTime()
*/

int TQTime::restart()
{
    TQTime t = currentTime();
    int n = msecsTo( t );
    if ( n < 0 )				// passed midnight
	n += 86400*1000;
    *this = t;
    return n;
}

/*!
    Returns the number of milliseconds that have elapsed since the
    last time start() or restart() was called.

    Note that the counter wraps to zero 24 hours after the last call
    to start() or restart.

    Note that the accuracy depends on the accuracy of the underlying
    operating system; not all systems provide 1-millisecond accuracy.

    \warning If the system's clock setting has been changed since the
    last time start() or restart() was called, the result is
    undefined. This can happen when daylight savings time is turned on
    or off.

    \sa start(), restart()
*/

int TQTime::elapsed() const
{
    int n = msecsTo( currentTime() );
    if ( n < 0 )				// passed midnight
	n += 86400*1000;
    return n;
}


/*****************************************************************************
  TQDateTime member functions
 *****************************************************************************/

/*!
    \class TQDateTime ntqdatetime.h
    \reentrant
    \brief The TQDateTime class provides date and time functions.

    \ingroup time
    \mainclass

    A TQDateTime object contains a calendar date and a clock time (a
    "datetime"). It is a combination of the TQDate and TQTime classes.
    It can read the current datetime from the system clock. It
    provides functions for comparing datetimes and for manipulating a
    datetime by adding a number of seconds, days, months or years.

    A TQDateTime object is typically created either by giving a date
    and time explicitly in the constructor, or by using the static
    function currentDateTime(), which returns a TQDateTime object set
    to the system clock's time. The date and time can be changed with
    setDate() and setTime(). A datetime can also be set using the
    setTime_t() function, which takes a POSIX-standard "number of
    seconds since 00:00:00 on January 1, 1970" value. The fromString()
    function returns a TQDateTime given a string and a date format
    which is used to interpret the date within the string.

    The date() and time() functions provide access to the date and
    time parts of the datetime. The same information is provided in
    textual format by the toString() function.

    TQDateTime provides a full set of operators to compare two
    TQDateTime objects where smaller means earlier and larger means
    later.

    You can increment (or decrement) a datetime by a given number of
    seconds using addSecs() or days using addDays(). Similarly you can
    use addMonths() and addYears(). The daysTo() function returns the
    number of days between two datetimes, and secsTo() returns the
    number of seconds between two datetimes.

    The range of a datetime object is constrained to the ranges of the
    TQDate and TQTime objects which it embodies.

    \sa TQDate TQTime TQDateTimeEdit
*/


/*!
    \fn TQDateTime::TQDateTime()

    Constructs a null datetime (i.e. null date and null time). A null
    datetime is invalid, since the date is invalid.

    \sa isValid()
*/


/*!
    Constructs a datetime with date \a date and null (but valid) time
    (00:00:00.000).
*/

TQDateTime::TQDateTime( const TQDate &date )
    : d(date)
{
}

/*!
    Constructs a datetime with date \a date and time \a time.
*/

TQDateTime::TQDateTime( const TQDate &date, const TQTime &time )
    : d(date), t(time)
{
}


/*!
    \fn bool TQDateTime::isNull() const

    Returns TRUE if both the date and the time are null; otherwise
    returns FALSE. A null datetime is invalid.

    \sa TQDate::isNull(), TQTime::isNull()
*/

/*!
    \fn bool TQDateTime::isValid() const

    Returns TRUE if both the date and the time are valid; otherwise
    returns FALSE.

    \sa TQDate::isValid(), TQTime::isValid()
*/

/*!
    \fn TQDate TQDateTime::date() const

    Returns the date part of the datetime.

    \sa setDate(), time()
*/

/*!
    \fn TQTime TQDateTime::time() const

    Returns the time part of the datetime.

    \sa setTime(), date()
*/

/*!
    \fn void TQDateTime::setDate( const TQDate &date )

    Sets the date part of this datetime to \a date.

    \sa date(), setTime()
*/

/*!
    \fn void TQDateTime::setTime( const TQTime &time )

    Sets the time part of this datetime to \a time.

    \sa time(), setDate()
*/


/*!
    Returns the datetime as the number of seconds that have passed
    since 1970-01-01T00:00:00, Coordinated Universal Time (UTC).

    On systems that do not support timezones, this function will
    behave as if local time were UTC.

    \sa setTime_t()
*/

time_t TQDateTime::toTime_t() const
{
    tm brokenDown;
    brokenDown.tm_sec = t.second();
    brokenDown.tm_min = t.minute();
    brokenDown.tm_hour = t.hour();
    brokenDown.tm_mday = d.day();
    brokenDown.tm_mon = d.month() - 1;
    brokenDown.tm_year = d.year() - 1900;
    brokenDown.tm_isdst = -1;
    time_t secsSince1Jan1970UTC = mktime( &brokenDown );
    if ( secsSince1Jan1970UTC < -1 )
	secsSince1Jan1970UTC = -1;
    return secsSince1Jan1970UTC;
}

/*!
    \overload

    Convenience function that sets the date and time to local time
    based on the given UTC time.
*/

void TQDateTime::setTime_t( time_t secsSince1Jan1970UTC )
{
    setTime_t( secsSince1Jan1970UTC, TQt::LocalTime );
}

/*!
    Sets the date and time to \a ts time (\c TQt::LocalTime or \c
    TQt::UTC) given the number of seconds that have passed since
    1970-01-01T00:00:00, Coordinated Universal Time (UTC). On systems
    that do not support timezones this function will behave as if
    local time were UTC.

    On Windows, only a subset of \a secsSince1Jan1970UTC values are
    supported, as Windows starts counting from 1980.

    \sa toTime_t()
*/
void TQDateTime::setTime_t( time_t secsSince1Jan1970UTC, TQt::TimeSpec ts )
{
    time_t tmp = secsSince1Jan1970UTC;
    tm *brokenDown = 0;

#if defined(Q_OS_UNIX) && defined(TQT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // posix compliant system
    // use the reentrant versions of localtime() and gmtime() where available
    tm res;
    if ( ts == TQt::LocalTime )
	brokenDown = localtime_r( &tmp, &res );
    if ( !brokenDown ) {
	brokenDown = gmtime_r( &tmp, &res );
	if ( !brokenDown ) {
	    d.jd = TQDate::gregorianToJulian( 1970, 1, 1 );
	    t.ds = 0;
	    return;
	}
    }
#else
    if ( ts == TQt::LocalTime )
	brokenDown = localtime( &tmp );
    if ( !brokenDown ) {
	brokenDown = gmtime( &tmp );
	if ( !brokenDown ) {
	    d.jd = TQDate::gregorianToJulian( 1970, 1, 1 );
	    t.ds = 0;
	    return;
	}
    }
#endif

    d.jd = TQDate::gregorianToJulian( brokenDown->tm_year + 1900,
				     brokenDown->tm_mon + 1,
				     brokenDown->tm_mday );
    t.ds = MSECS_PER_HOUR * brokenDown->tm_hour +
	   MSECS_PER_MIN * brokenDown->tm_min +
	   1000 * brokenDown->tm_sec;
}
#ifndef TQT_NO_DATESTRING
#ifndef TQT_NO_SPRINTF
/*!
    \overload

    Returns the datetime as a string. The \a f parameter determines
    the format of the string.

    If \a f is \c TQt::TextDate, the string format is "Wed May 20
    03:40:13 1998" (using TQDate::shortDayName(), TQDate::shortMonthName(),
    and TQTime::toString() to generate the string, so the day and month
    names will have localized names).

    If \a f is \c TQt::ISODate, the string format corresponds to the
    ISO 8601 extended specification for representations of dates and
    times, which is YYYY-MM-DDTHH:MM:SS.

    If \a f is \c TQt::LocalDate, the string format depends on the
    locale settings of the system.

    If the format \a f is invalid or the datetime is invalid, toString()
    returns a null string.

    \sa TQDate::toString() TQTime::toString()
*/

TQString TQDateTime::toString( TQt::DateFormat f ) const
{
    if ( !isValid() )
	return TQString::null;

    if ( f == TQt::ISODate ) {
	return d.toString( TQt::ISODate ) + "T" + t.toString( TQt::ISODate );
    }
#ifndef TQT_NO_TEXTDATE
    else if ( f == TQt::TextDate ) {
#ifndef TQ_WS_WIN
	TQString buf = d.shortDayName( d.dayOfWeek() );
	buf += ' ';
	buf += d.shortMonthName( d.month() );
	buf += ' ';
	buf += TQString().setNum( d.day() );
	buf += ' ';
#else
	TQString buf;
	TQString winstr;
	QT_WA( {
	    TCHAR out[255];
	    GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_ILDATE, out, 255 );
	    winstr = TQString::fromUcs2( (ushort*)out );
	} , {
	    char out[255];
	    GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_ILDATE, (char*)&out, 255 );
	    winstr = TQString::fromLocal8Bit( out );
	} );
	switch ( winstr.toInt() ) {
	case 1:
	    buf = d.shortDayName( d.dayOfWeek() ) + " " + TQString().setNum( d.day() ) + ". " + d.shortMonthName( d.month() ) + " ";
	    break;
	default:
	    buf = d.shortDayName( d.dayOfWeek() ) + " " + d.shortMonthName( d.month() ) + " " + TQString().setNum( d.day() ) + " ";
	    break;
	}
#endif
	buf += t.toString();
	buf += ' ';
	buf += TQString().setNum( d.year() );
	return buf;
    }
#endif
    else if ( f == TQt::LocalDate ) {
	return d.toString( TQt::LocalDate ) + " " + t.toString( TQt::LocalDate );
    }
    return TQString::null;
}
#endif

/*!
    Returns the datetime as a string. The \a format parameter
    determines the format of the result string.

    These expressions may be used for the date:

    \table
    \header \i Expression \i Output
    \row \i d \i the day as number without a leading zero (1-31)
    \row \i dd \i the day as number with a leading zero (01-31)
    \row \i ddd
	    \i the abbreviated localized day name (e.g. 'Mon'..'Sun').
	    Uses TQDate::shortDayName().
    \row \i dddd
	    \i the long localized day name (e.g. 'Monday'..'Sunday').
	    Uses TQDate::longDayName().
    \row \i M \i the month as number without a leading zero (1-12)
    \row \i MM \i the month as number with a leading zero (01-12)
    \row \i MMM
	    \i the abbreviated localized month name (e.g. 'Jan'..'Dec').
	    Uses TQDate::shortMonthName().
    \row \i MMMM
	    \i the long localized month name (e.g. 'January'..'December').
	    Uses TQDate::longMonthName().
    \row \i yy \i the year as two digit number (00-99)
    \row \i yyyy \i the year as four digit number (1752-8000)
    \endtable

    These expressions may be used for the time:

    \table
    \header \i Expression \i Output
    \row \i h
	    \i the hour without a leading zero (0..23 or 1..12 if AM/PM display)
    \row \i hh
	    \i the hour with a leading zero (00..23 or 01..12 if AM/PM display)
    \row \i m \i the minute without a leading zero (0..59)
    \row \i mm \i the minute with a leading zero (00..59)
    \row \i s \i the second whithout a leading zero (0..59)
    \row \i ss \i the second whith a leading zero (00..59)
    \row \i z \i the milliseconds without leading zeroes (0..999)
    \row \i zzz \i the milliseconds with leading zeroes (000..999)
    \row \i AP
	    \i use AM/PM display. \e AP will be replaced by either "AM" or "PM".
    \row \i ap
	    \i use am/pm display. \e ap will be replaced by either "am" or "pm".
    \endtable

    All other input characters will be ignored.

    Example format strings (assumed that the TQDateTime is
    21<small><sup>st</sup></small> May 2001 14:13:09)

    \table
    \header \i Format \i Result
    \row \i dd.MM.yyyy	    \i11 21.05.2001
    \row \i ddd MMMM d yy   \i11 Tue May 21 01
    \row \i hh:mm:ss.zzz    \i11 14:13:09.042
    \row \i h:m:s ap	    \i11 2:13:9 pm
    \endtable

    If the datetime is an invalid datetime, then TQString::null will be returned.

    \sa TQDate::toString() TQTime::toString()
*/
TQString TQDateTime::toString( const TQString& format ) const
{
    return fmtDateTime( format, &t, &d );
}
#endif //TQT_NO_DATESTRING

/*!
    Returns a TQDateTime object containing a datetime \a ndays days
    later than the datetime of this object (or earlier if \a ndays is
    negative).

    \sa daysTo(), addMonths(), addYears(), addSecs()
*/

TQDateTime TQDateTime::addDays( int ndays ) const
{
    return TQDateTime( d.addDays(ndays), t );
}

/*!
    Returns a TQDateTime object containing a datetime \a nmonths months
    later than the datetime of this object (or earlier if \a nmonths
    is negative).

    \sa daysTo(), addDays(), addYears(), addSecs()
*/

TQDateTime TQDateTime::addMonths( int nmonths ) const
{
    return TQDateTime( d.addMonths(nmonths), t );
}

/*!
    Returns a TQDateTime object containing a datetime \a nyears years
    later than the datetime of this object (or earlier if \a nyears is
    negative).

    \sa daysTo(), addDays(), addMonths(), addSecs()
*/

TQDateTime TQDateTime::addYears( int nyears ) const
{
    return TQDateTime( d.addYears(nyears), t );
}

/*!
    Returns a TQDateTime object containing a datetime \a nsecs seconds
    later than the datetime of this object (or earlier if \a nsecs is
    negative).

    \sa secsTo(), addDays(), addMonths(), addYears()
*/

TQDateTime TQDateTime::addSecs( int nsecs ) const
{
    uint dd = d.jd;
    int  tt = t.ds;
    int  sign = 1;
    if ( nsecs < 0 ) {
	nsecs = -nsecs;
	sign = -1;
    }
    if ( nsecs >= (int)SECS_PER_DAY ) {
	dd += sign*(nsecs/SECS_PER_DAY);
	nsecs %= SECS_PER_DAY;
    }
    tt += sign*nsecs*1000;
    if ( tt < 0 ) {
	tt = MSECS_PER_DAY - tt - 1;
	dd -= tt / MSECS_PER_DAY;
	tt = tt % MSECS_PER_DAY;
	tt = MSECS_PER_DAY - tt - 1;
    } else if ( tt >= (int)MSECS_PER_DAY ) {
	dd += ( tt / MSECS_PER_DAY );
	tt = tt % MSECS_PER_DAY;
    }
    TQDateTime ret;
    ret.t.ds = tt;
    ret.d.jd = dd;
    return ret;
}

/*!
    Returns the number of days from this datetime to \a dt (which is
    negative if \a dt is earlier than this datetime).

    \sa addDays(), secsTo()
*/

int TQDateTime::daysTo( const TQDateTime &dt ) const
{
    return d.daysTo( dt.d );
}

/*!
    Returns the number of seconds from this datetime to \a dt (which
    is negative if \a dt is earlier than this datetime).

    Example:
    \code
    TQDateTime dt = TQDateTime::currentDateTime();
    TQDateTime xmas( TQDate(dt.date().year(),12,24), TQTime(17,00) );
    tqDebug( "There are %d seconds to Christmas", dt.secsTo(xmas) );
    \endcode

    \sa addSecs(), daysTo(), TQTime::secsTo()
*/

int TQDateTime::secsTo( const TQDateTime &dt ) const
{
    return t.secsTo(dt.t) + d.daysTo(dt.d)*SECS_PER_DAY;
}


/*!
    Returns TRUE if this datetime is equal to \a dt; otherwise returns FALSE.

    \sa operator!=()
*/

bool TQDateTime::operator==( const TQDateTime &dt ) const
{
    return  t == dt.t && d == dt.d;
}

/*!
    Returns TRUE if this datetime is different from \a dt; otherwise
    returns FALSE.

    \sa operator==()
*/

bool TQDateTime::operator!=( const TQDateTime &dt ) const
{
    return  t != dt.t || d != dt.d;
}

/*!
    Returns TRUE if this datetime is earlier than \a dt; otherwise
    returns FALSE.
*/

bool TQDateTime::operator<( const TQDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t < dt.t : FALSE;
}

/*!
    Returns TRUE if this datetime is earlier than or equal to \a dt;
    otherwise returns FALSE.
*/

bool TQDateTime::operator<=( const TQDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t <= dt.t : FALSE;
}

/*!
    Returns TRUE if this datetime is later than \a dt; otherwise
    returns FALSE.
*/

bool TQDateTime::operator>( const TQDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t > dt.t : FALSE;
}

/*!
    Returns TRUE if this datetime is later than or equal to \a dt;
    otherwise returns FALSE.
*/

bool TQDateTime::operator>=( const TQDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t >= dt.t : FALSE;
}

/*!
    \overload

    Returns the current datetime, as reported by the system clock.

    \sa TQDate::currentDate(), TQTime::currentTime()
*/

TQDateTime TQDateTime::currentDateTime()
{
    return currentDateTime( TQt::LocalTime );
}

/*!
  Returns the current datetime, as reported by the system clock, for the
  TimeSpec \a ts. The default TimeSpec is LocalTime.

  \sa TQDate::currentDate(), TQTime::currentTime(), TQt::TimeSpec
*/

TQDateTime TQDateTime::currentDateTime( TQt::TimeSpec ts )
{
    TQDateTime dt;
    TQTime t;
    dt.setDate( TQDate::currentDate(ts) );
    if ( TQTime::currentTime(&t, ts) )         // midnight or right after?
	dt.setDate( TQDate::currentDate(ts) ); // fetch date again
    dt.setTime( t );
    return dt;
}

#ifndef TQT_NO_DATESTRING
/*!
    Returns the TQDateTime represented by the string \a s, using the
    format \a f, or an invalid datetime if this is not possible.

    Note for \c TQt::TextDate: It is recommended that you use the
    English short month names (e.g. "Jan"). Although localized month
    names can also be used, they depend on the user's locale settings.

    \warning Note that \c TQt::LocalDate cannot be used here.
*/
TQDateTime TQDateTime::fromString( const TQString& s, TQt::DateFormat f )
{
    if ( ( s.isEmpty() ) || ( f == TQt::LocalDate ) ) {
#if defined(QT_CHECK_RANGE)
	tqWarning( "TQDateTime::fromString: Parameter out of range" );
#endif
	TQDateTime dt;
	dt.d.jd = 0;
	return dt;
    }
    if ( f == TQt::ISODate ) {
	return TQDateTime( TQDate::fromString( s.mid(0,10), TQt::ISODate ),
			  TQTime::fromString( s.mid(11), TQt::ISODate ) );
    }
#if !defined(TQT_NO_REGEXP) && !defined(TQT_NO_TEXTDATE)
    else if ( f == TQt::TextDate ) {
        const int firstSpace = s.find(' ');
	TQString monthName( s.mid( firstSpace + 1, 3 ) );
	int month = -1;
	// Assume that English monthnames are the default
	for ( int i = 0; i < 12; ++i ) {
	    if ( monthName == qt_shortMonthNames[i] ) {
		month = i + 1;
		break;
	    }
	}
	// If English names can't be found, search the localized ones
	if ( month == -1 ) {
	    for ( int i = 1; i <= 12; ++i ) {
		if ( monthName == TQDate::shortMonthName( i ) ) {
		    month = i;
		    break;
		}
	    }
	}
#if defined(QT_CHECK_RANGE)
	if ( month < 1 || month > 12 ) {
	    tqWarning( "TQDateTime::fromString: Parameter out of range" );
	    TQDateTime dt;
	    dt.d.jd = 0;
	    return dt;
	}
#endif
	int day = s.mid( firstSpace + 5, 2 ).simplifyWhiteSpace().toInt();
	int year = s.right( 4 ).toInt();
	TQDate date( year, month, day );
	TQTime time;
	int hour, minute, second;
	int pivot = s.find( TQRegExp(TQString::fromLatin1("[0-9][0-9]:[0-9][0-9]:[0-9][0-9]")) );
	if ( pivot != -1 ) {
	    hour = s.mid( pivot, 2 ).toInt();
	    minute = s.mid( pivot+3, 2 ).toInt();
	    second = s.mid( pivot+6, 2 ).toInt();
	    time.setHMS( hour, minute, second );
	}
	return TQDateTime( date, time );
    }
#endif //TQT_NO_REGEXP
    return TQDateTime();
}
#endif //TQT_NO_DATESTRING


/*****************************************************************************
  Date/time stream functions
 *****************************************************************************/

#ifndef TQT_NO_DATASTREAM
/*!
    \relates TQDate

    Writes the date, \a d, to the data stream, \a s.

    \sa \link datastreamformat.html Format of the TQDataStream operators \endlink
*/

TQDataStream &operator<<( TQDataStream &s, const TQDate &d )
{
    return s << (TQ_UINT32)(d.jd);
}

/*!
    \relates TQDate

    Reads a date from the stream \a s into \a d.

    \sa \link datastreamformat.html Format of the TQDataStream operators \endlink
*/

TQDataStream &operator>>( TQDataStream &s, TQDate &d )
{
    TQ_UINT32 jd;
    s >> jd;
    d.jd = jd;
    return s;
}

/*!
    \relates TQTime

    Writes time \a t to the stream \a s.

    \sa \link datastreamformat.html Format of the TQDataStream operators \endlink
*/

TQDataStream &operator<<( TQDataStream &s, const TQTime &t )
{
    return s << (TQ_UINT32)(t.ds);
}

/*!
    \relates TQTime

    Reads a time from the stream \a s into \a t.

    \sa \link datastreamformat.html Format of the TQDataStream operators \endlink
*/

TQDataStream &operator>>( TQDataStream &s, TQTime &t )
{
    TQ_UINT32 ds;
    s >> ds;
    t.ds = ds;
    return s;
}

/*!
    \relates TQDateTime

    Writes the datetime \a dt to the stream \a s.

    \sa \link datastreamformat.html Format of the TQDataStream operators \endlink
*/

TQDataStream &operator<<( TQDataStream &s, const TQDateTime &dt )
{
    return s << dt.d << dt.t;
}

/*!
    \relates TQDateTime

    Reads a datetime from the stream \a s into \a dt.

    \sa \link datastreamformat.html Format of the TQDataStream operators \endlink
*/

TQDataStream &operator>>( TQDataStream &s, TQDateTime &dt )
{
    s >> dt.d >> dt.t;
    return s;
}
#endif //TQT_NO_DATASTREAM
