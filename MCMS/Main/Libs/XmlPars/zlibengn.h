//
// This header file defines the ZlibEngine class used
// perform file compression and decompression using
// Zlib.
//
// The ZlibEngine is a Tiny Software (tm) project.
// You may use the code in this project without restriction.
// Contact markn@tiny.com for more information.
//

#if !defined( _ZLIBENGN_H )
#define _ZLIBENGN_H

//
// All of the Zlib code is compiled in C modules.  Fortunately,
// I can wrap the entire header in an 'extern "C"' declaration,
// and it will then link properly!
//

extern "C" {
#include <zlib.h>
}
class CStrArray;



class ZlibEngine : public z_stream {
    public :
        ZlibEngine(CStrArray *pStrArray);
	virtual ~ZlibEngine();
        void compress( char **pArray,
                      const char *output,
                      int level  ,int size);
		void decompress(int Size,char *pStream);
 		void DeflateInit(int CompressionLevel);
		void DeflateEnd();
		void Deflate(int Size,char *pStream);
		int GetOutStreamLen();
		void DeflateReset();
		void DeflateCleanup();

		//
// These three functions are only used internally.
//
    protected :
        //int percent();
        //int load_input();
        int flush_output();
//
// Derived classes can provide versions of this
// virtual fns in order to customize their
// program's user interface.  The abort flag
// can be set by those functions.
//
    protected :
        //virtual void progress( int percent ){};
        virtual void status( char *message ){};
       
//
// The remaining data members are used internally/
//
    protected :
        //FILE *fin;
        //FILE *fout;
   //     long length;
        int err;
        //enum { input_length = 10000 };
        //unsigned char input_buffer[ input_length ];
        //enum { output_length = 10000 };
        //unsigned char output_buffer[ output_length ];
		char *pOut;
		int length;
		CStrArray *m_pStrArray;
};

//
// I define one error code in addition to those
// found in zlib.h
//
#define Z_USER_ABORT (-7)

#endif  // #if !defined( _ZLIBENGN_H )
