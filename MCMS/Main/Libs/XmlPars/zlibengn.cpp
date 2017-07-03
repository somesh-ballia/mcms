

#include "zlibengn.h"
#include "psosxml.h"


//
// The constructor initializes a couple of members
// of the z_stream class.  See the Zlib documentation
// for details on what those members do
//

ZlibEngine::ZlibEngine(CStrArray *pStrArray)
{
    zalloc = 0;  //z_stream member
    zfree = 0;   //z_stream member
    opaque = 0;  //z_stream member
	pOut=new char[COMMPRESS_BUFFER];
	length=0;
	m_pStrArray=pStrArray;
	err = Z_OK;
}

//
// compress() is the public function used to compress
// a single file.  It has to take care of opening the
// input and output files and setting up the buffers for
// Zlib.  It then calls deflate() repeatedly until all
// input and output processing has been done, and finally
// closes the files and cleans up the Zlib structures.
//


ZlibEngine::~ZlibEngine()
{
	delete pOut;
	
}

void ZlibEngine::DeflateInit(int CompressionLevel)
{
	err = Z_OK;
    avail_in = 0;
	avail_out = COMMPRESS_BUFFER;
    next_out = (unsigned char *)pOut;
	deflateInit( this, CompressionLevel/*level*/ );
}

void ZlibEngine::DeflateCleanup()
{
	for ( ; ; ) 
	{
        err = deflate( this, Z_FINISH );
        if ( !flush_output() )
            break;
        if ( err != Z_OK )
            break;
    }


    deflateEnd( this );

    if ( err != Z_OK && err != Z_STREAM_END )
        status( "Zlib Error" );
    else 
	{
        status( "Success" );
        err = Z_OK;
    }
}

void ZlibEngine::DeflateEnd()
{
 
	for ( ; ; ) 
	{
        err = deflate( this, Z_FINISH );
        if ( !flush_output() )
            break;
        if ( err != Z_OK )
            break;
    }


    //deflateEnd( this );

    if ( err != Z_OK && err != Z_STREAM_END )
        status( "Zlib Error" );
    else 
	{
        status( "Success" );
        err = Z_OK;
    }
//    fclose( fout );
//    fout = 0;

}

void ZlibEngine::Deflate(int Size,char *pStream)
{
	next_in=(unsigned char *)pStream;
	avail_in=Size;
	/*err = deflate( this, Z_NO_FLUSH );
    flush_output();*/
	for ( ; ; ) 
	{
        err = deflate( this, Z_NO_FLUSH );
        if ( !flush_output() )
            break;
        if ( err != Z_OK )
            break;
    }


}

int ZlibEngine::flush_output()
{
    unsigned int count = COMMPRESS_BUFFER - avail_out;
    if ( count ) 
	{
		if(m_pStrArray)
			m_pStrArray->Add(pOut,count);
		avail_out = COMMPRESS_BUFFER;
		next_out = (unsigned char *)pOut;
		length+=count;
    }
    return count;
}

int ZlibEngine::GetOutStreamLen()
{
	return length;
}
void ZlibEngine::DeflateReset()
{
	err = Z_OK;
    avail_in = 0;
	avail_out = COMMPRESS_BUFFER;
    next_out = (unsigned char *)pOut;
	deflateReset(this);
}



//
// decompress has to do most of the same chores as compress().
// The only major difference it has is the absence of the level
// parameter.  The level isn't needed when decompressing data
// using the deflate algorithm.
//

void ZlibEngine::decompress(int Size,char *pStream)
{
    err = Z_OK;
	next_in=(unsigned char *)pStream;
	avail_in=Size;
    avail_out = COMMPRESS_BUFFER;
    next_out = (unsigned char *)pOut;

    inflateInit( this );
    for ( ; ; ) 
	{
        err = inflate( this, Z_NO_FLUSH );
        flush_output();
        if ( err != Z_OK )
            break;
    }
    for ( ; ; ) 
	{
        err = inflate( this, Z_FINISH );
        if ( !flush_output() )
            break;
        if ( err != Z_OK )
            break;
    }
 
    inflateEnd( this );
    if ( err != Z_OK && err != Z_STREAM_END )
        status( "Zlib Error" );
    else {
        status( "Success" );
        err = Z_OK;
    }


}
//
//  This function is called so as to provide the progress()
//  virtual function with a reasonable figure to indicate
//  how much processing has been done.  Note that the length
//  member is initialized when the input file is opened.
//
 /*int ZlibEngine::percent()
{
   if ( length == 0 )
        return 100;
    else if ( length > 10000000L )
        return ( total_in / ( length / 100 ) );
    else
        return ( total_in * 100 / length );
}*/

//
//  Every time Zlib consumes all of the data in the
//  input buffer, this function gets called to reload.
//  The avail_in member is part of z_stream, and is
//  used to keep track of how much input is available.
//  I churn the Windows message loop to ensure that
//  the process can be aborted by a button press or
//  other Windows event.
//
/*int ZlibEngine::load_input()
{

    if ( avail_in == 0 ) {
        next_in = input_buffer;
        avail_in = fread( input_buffer, 1, input_length, fin );
    }
    return avail_in;
}*/

//
//  Every time Zlib filsl the output buffer with data,
//  this function gets called.  Its job is to write
//  that data out to the output file, then update
//  the z_stream member avail_out to indicate that more
//  space is now available.  I churn the Windows message
//  loop to ensure that the process can be aborted by a
//  button press or other Windows event.
//


