/****************************************************************************
**
** Utility program for embedding binary data into a C/C++ source code.
** It reads a binary file and generates a C array with the binary data.
** The C code is written to standard output.
**
** Created : 951017
**
** Copyright (C) 1995-2008 Trolltech ASA.  All rights reserved.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that this copyright notice appears in all copies.
** No representations are made about the suitability of this software for any
** purpose. It is provided "as is" without express or implied warranty.
**
*****************************************************************************/

#include <ntqstring.h>
#include <ntqfile.h>
#include <ntqfileinfo.h>
#include <ntqptrlist.h>
#include <ntqtextstream.h>
#include <ntqdatetime.h>
#include <ntqimage.h>
#include <ntqdict.h>
#include <ntqdir.h>
#include <ctype.h>
#include <stdlib.h>

static void embedData( const TQByteArray &input, TQFile *output );
static void embedData( const uchar* input, int size, TQFile *output );
static void embedData( const TQRgb* input, int size, TQFile *output );
static TQString convertFileNameToCIdentifier( const char * );

static const char header[] = "/* Generated by qembed */\n";

struct Embed {
    uint size;
    TQString name;
    TQString cname;
};

struct EmbedImage {
    int width;
    int height;
    int depth;
    int numColors;
    TQRgb* colorTable;
    TQString name;
    TQString cname;
    bool alpha;
};

int main( int argc, char **argv )
{
    if ( argc < 2 ) {
	tqWarning( "Usage:\n\t%s [--images] files", argv[0] );
	return 1;
    }

    TQFile output;
    bool output_hdr = FALSE;
    bool images = FALSE;
    output.open( IO_WriteOnly, stdout );
    TQTextStream out( &output );

    TQPtrList<EmbedImage> list_image;
    TQPtrList<Embed> list;
    list.setAutoDelete( TRUE );
    list_image.setAutoDelete( TRUE );

    long l = rand();
    out << "#ifndef _QEMBED_" << l << endl;
    out << "#define _QEMBED_" << l << endl;

    TQStringList args;
    for ( int i = 1; i < argc; ++i ) {
	TQString file( argv[i] );
#ifdef Q_WS_WIN
	// Since wildcards are not expanded automatically for us on Windows, we need to do 
	// it ourselves
	if ( file.contains( '*' ) || file.contains( '?' ) ) {
	    TQDir d;
	    const TQFileInfoList *fiList = d.entryInfoList( file, TQDir::Files );
	    TQFileInfoListIterator it(*fiList);
	    while ( it.current() ) {
		args << (*it)->filePath();
		++it;
	    }
	} else
#endif
	    args << file;
    }
    for ( TQStringList::Iterator it = args.begin(); it != args.end(); ++it ) {
	TQString arg = (*it);
	if ( arg == "--images" ) {
	    if ( !images ) {
		out << "#include <ntqimage.h>\n";
		out << "#include <ntqdict.h>\n";
		images = TRUE;
	    }
	} else {
	    TQFile f( *it );
	    if ( !f.open(IO_ReadOnly) ) {
		tqWarning( "Cannot open file %s, ignoring it", (*it).latin1() );
		continue;
	    }
	    TQByteArray a( f.size() );
	    if ( f.size() == 0
		 || f.readBlock(a.data(), f.size()) != (int)f.size() ) {
		tqWarning( "Cannot read file %s, ignoring it", (*it).latin1() );
		continue;
	    }
	    if ( images ) {
		TQImage img;
		if ( !img.loadFromData(a) ) {
		    tqWarning( "Cannot read image from file %s, ignoring it", (*it).latin1() );
		    continue;
		}
		EmbedImage *e = new EmbedImage;
		e->width = img.width();
		e->height = img.height();
		e->depth = img.depth();
		e->numColors = img.numColors();
		e->colorTable = new TQRgb[e->numColors];
		e->alpha = img.hasAlphaBuffer();
		memcpy(e->colorTable, img.colorTable(), e->numColors*sizeof(TQRgb));
		TQFileInfo fi( (*it) );
		e->name = fi.baseName();
		e->cname = convertFileNameToCIdentifier( e->name.latin1() );
		list_image.append( e );
		TQString s;
		if ( e->depth == 32 ) {
		    out << s.sprintf( "static const TQRgb %s_data[] = {",
				   (const char *)e->cname );
		    embedData( (TQRgb*)img.bits(), e->width*e->height, &output );
		} else {
		    if ( e->depth == 1 )
			img = img.convertBitOrder(TQImage::BigEndian);
		    out << s.sprintf( "static const unsigned char %s_data[] = {",
				   (const char *)e->cname );
		    embedData( img.bits(), img.numBytes(), &output );
		}
		out << "\n};\n\n";
		if ( e->numColors ) {
		    out << s.sprintf( "static const TQRgb %s_ctable[] = {",
				   (const char *)e->cname );
		    embedData( e->colorTable, e->numColors, &output );
		    out << "\n};\n\n";
		}
	    } else {
		Embed *e = new Embed;
		e->size = f.size();
		e->name = (*it);
		e->cname = convertFileNameToCIdentifier( (*it) );
		list.append( e );
		TQString s;
		out << s.sprintf( "static const unsigned int  %s_len = %d;\n",
			       (const char *)e->cname, e->size );
		out << s.sprintf( "static const unsigned char %s_data[] = {",
			       (const char *)e->cname );
		embedData( a, &output );
		out << "\n};\n\n";
	    }
	    if ( !output_hdr ) {
		output_hdr = TRUE;
		out << header;
	    }
	}
    }

    if ( list.count() > 0 ) {
	out << "#include <ntqcstring.h>\n";
	if ( !images )
	    out << "#include <ntqdict.h>\n";

	out << "static struct Embed {\n"
	       "    unsigned int size;\n"
	       "    const unsigned char *data;\n"
	       "    const char *name;\n"
	       "} embed_vec[] = {\n";
	Embed *e = list.first();
	while ( e ) {
	    out << "    { " << e->size << ", " << e->cname << "_data, "
		 << "\"" << e->name << "\" },\n";
	    e = list.next();
	}
	out << "    { 0, 0, 0 }\n};\n";

	out << "\n"
"static const TQByteArray& qembed_findData( const char* name )\n"
"{\n"
"    static TQDict<TQByteArray> dict;\n"
"    TQByteArray* ba = dict.find( name );\n"
"    if ( !ba ) {\n"
"	for ( int i = 0; embed_vec[i].data; i++ ) {\n"
"	    if ( strcmp(embed_vec[i].name, name) == 0 ) {\n"
"		ba = new TQByteArray;\n"
"		ba->setRawData( (char*)embed_vec[i].data,\n"
"				embed_vec[i].size );\n"
"		dict.insert( name, ba );\n"
"		break;\n"
"	    }\n"
"	}\n"
"	if ( !ba ) {\n"
"	    static TQByteArray dummy;\n"
"	    return dummy;\n"
"	}\n"
"    }\n"
"    return *ba;\n"
"}\n\n";
    }

    if ( list_image.count() > 0 ) {
	out << "static struct EmbedImage {\n"
	       "    int width, height, depth;\n"
	       "    const unsigned char *data;\n"
	       "    int numColors;\n"
	       "    const TQRgb *colorTable;\n"
	       "    bool alpha;\n"
	       "    const char *name;\n"
	       "} embed_image_vec[] = {\n";
	EmbedImage *e = list_image.first();
	while ( e ) {
	    out << "    { "
		<< e->width << ", "
		<< e->height << ", "
		<< e->depth << ", "
		<< "(const unsigned char*)" << e->cname << "_data, "
		<< e->numColors << ", ";
	    if ( e->numColors )
		out << e->cname << "_ctable, ";
	    else
		out << "0, ";
	    if ( e->alpha )
		out << "TRUE, ";
	    else
		out << "FALSE, ";
	    out << "\"" << e->name << "\" },\n";
	    e = list_image.next();
	}
	out << "    { 0, 0, 0, 0, 0, 0, 0, 0 }\n};\n";

	out << "\n"
"static const TQImage& qembed_findImage( const TQString& name )\n"
"{\n"
"    static TQDict<TQImage> dict;\n"
"    TQImage* img = dict.find( name );\n"
"    if ( !img ) {\n"
"	for ( int i = 0; embed_image_vec[i].data; i++ ) {\n"
"	    if ( strcmp(embed_image_vec[i].name, name.latin1()) == 0 ) {\n"
"		img = new TQImage((uchar*)embed_image_vec[i].data,\n"
"			    embed_image_vec[i].width,\n"
"			    embed_image_vec[i].height,\n"
"			    embed_image_vec[i].depth,\n"
"			    (TQRgb*)embed_image_vec[i].colorTable,\n"
"			    embed_image_vec[i].numColors,\n"
"			    TQImage::BigEndian );\n"
"		if ( embed_image_vec[i].alpha )\n"
"		    img->setAlphaBuffer( TRUE );\n"
"		dict.insert( name, img );\n"
"		break;\n"
"	    }\n"
"	}\n"
"	if ( !img ) {\n"
"	    static TQImage dummy;\n"
"	    return dummy;\n"
"	}\n"
"    }\n"
"    return *img;\n"
"}\n\n";
    }

    out << "#endif" << endl;

    return 0;
}


TQString convertFileNameToCIdentifier( const char *s )
{
    TQString r = s;
    int len = r.length();
    if ( len > 0 && !isalpha( (char)r[0].latin1() ) )
	r[0] = '_';
    for ( int i = 1; i < len; i++ ) {
	if ( !isalnum( (char)r[i].latin1() ) )
	    r[i] = '_';
    }
    return r;
}


void embedData( const TQByteArray &input, TQFile *output )
{
    embedData((uchar*)input.data(), input.size(), output);
}

void embedData( const uchar* input, int nbytes, TQFile *output )
{
    static const char hexdigits[] = "0123456789abcdef";
    TQString s;
    for ( int i=0; i<nbytes; i++ ) {
	if ( (i%14) == 0 ) {
	    s += "\n    ";
	    output->writeBlock( (const char*)s, s.length() );
	    s.truncate( 0 );
	}
	uint v = input[i];
	s += "0x";
	s += hexdigits[(v >> 4) & 15];
	s += hexdigits[v & 15];
	if ( i < nbytes-1 )
	    s += ',';
    }
    if ( s.length() )
	output->writeBlock( (const char*)s, s.length() );
}

void embedData( const TQRgb* input, int n, TQFile *output )
{
    TQString s;
    for ( int i = 0; i < n; i++ ) {
	if ( (i % 14) == 0 ) {
	    s += "\n    ";
	    output->writeBlock( (const char*)s, s.length() );
	    s.truncate( 0 );
	}
	TQRgb v = input[i];
	s += "0x";
	s += TQString::number( v, 16 );
	if ( i < n-1 )
	    s += ',';
    }
    if ( s.length() )
	output->writeBlock( (const char*)s, s.length() );
}
