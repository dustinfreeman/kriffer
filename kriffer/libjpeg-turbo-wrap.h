//Built from libjpeg-turbo's example.c

#include <iostream>

/*
* Include file for users of JPEG library.
* You will need to have included system headers that define at least
* the typedefs FILE and size_t before you can include jpeglib.h.
* (stdio.h is sufficient on ANSI-conforming systems.)
* You may also wish to include "jerror.h".
*/

#include "../../libjpeg-turbo/include/jpeglib.h"

/*
* <setjmp.h> is used for the optional error recovery mechanism shown in
* the second part of the example.
*/

#include <setjmp.h>

namespace jpeg {
	// ==== buffer dest help functions
	//http://stackoverflow.com/a/4559932/2518451
	std::vector<JOCTET> *output_buffer;
	#define BLOCK_SIZE 16384

	void my_init_destination(j_compress_ptr cinfo)
	{
		output_buffer->resize(BLOCK_SIZE);
		cinfo->dest->next_output_byte = &((*output_buffer)[0]);
		cinfo->dest->free_in_buffer = output_buffer->size();
	}

	boolean my_empty_output_buffer(j_compress_ptr cinfo)
	{
		size_t oldsize = output_buffer->size();
		output_buffer->resize(oldsize + BLOCK_SIZE);
		cinfo->dest->next_output_byte = &((*output_buffer)[oldsize]);
		cinfo->dest->free_in_buffer = output_buffer->size() - oldsize;
		return true;
	}

	void my_term_destination(j_compress_ptr cinfo)
	{
		output_buffer->resize(output_buffer->size() - cinfo->dest->free_in_buffer);
	}
	// ====

	bool compress(std::vector<JOCTET> *_output_buffer, int *olen,
		unsigned char* image_buffer,
		int image_width, int image_height, 
		int colour_channels, int quality = 95) {
		//(obuf, *olen, img_chunk->image, width, height, NUM_CLR_CHANNELS);

		/* This struct contains the JPEG compression parameters and pointers to
		* working space (which is allocated as needed by the JPEG library).
		* It is possible to have several such structures, representing multiple
		* compression/decompression processes, in existence at once.  We refer
		* to any one struct (and its associated working data) as a "JPEG object".
		*/
		struct jpeg_compress_struct cinfo;
		/* This struct represents a JPEG error handler.  It is declared separately
		* because applications often want to supply a specialized error handler
		* (see the second half of this file for an example).  But here we just
		* take the easy way out and use the standard error handler, which will
		* print a message on stderr and call exit() if compression fails.
		* Note that this struct must live as long as the main JPEG parameter
		* struct, to avoid dangling-pointer problems.
		*/
		struct jpeg_error_mgr jerr;
		/* More stuff */
		JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
		int row_stride;		/* physical row width in image buffer */

		/* Step 1: allocate and initialize JPEG compression object */

		/* We have to set up the error handler first, in case the initialization
		* step fails.  (Unlikely, but it could happen if you are out of memory.)
		* This routine fills in the contents of struct jerr, and returns jerr's
		* address which we place into the link field in cinfo.
		*/
		cinfo.err = jpeg_std_error(&jerr);
		/* Now we can initialize the JPEG compression object. */
		jpeg_create_compress(&cinfo);

		/* Step 2: specify data destination (eg, a file) */
		/* Note: steps 2 and 3 can be done in either order. */

		//Differ from example.c here by initting a buffer for output,  
		// as opposed to a file.
		//these calls replace jpeg_stdio_dest(&cinfo, outfile);
		output_buffer = _output_buffer;
		cinfo.dest->init_destination = &my_init_destination;
		cinfo.dest->empty_output_buffer = &my_empty_output_buffer;
		cinfo.dest->term_destination = &my_term_destination;

		/* Step 3: set parameters for compression */

		/* First we supply a description of the input image.
		* Four fields of the cinfo struct must be filled in:
		*/
		cinfo.image_width = image_width; 	/* image width and height, in pixels */
		cinfo.image_height = image_height;
		cinfo.input_components = colour_channels;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
		/* Now use the library's routine to set default compression parameters.
		* (You must set at least cinfo.in_color_space before calling this,
		* since the defaults depend on the source color space.)
		*/
		jpeg_set_defaults(&cinfo);
		/* Now you can set any non-default parameters you wish to.
		* Here we just illustrate the use of quality (quantization table) scaling:
		*/
		jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

		/* Step 4: Start compressor */

		/* TRUE ensures that we will write a complete interchange-JPEG file.
		* Pass TRUE unless you are very sure of what you're doing.
		*/
		jpeg_start_compress(&cinfo, TRUE);

		/* Step 5: while (scan lines remain to be written) */
		/*           jpeg_write_scanlines(...); */

		/* Here we use the library's state variable cinfo.next_scanline as the
		* loop counter, so that we don't have to keep track ourselves.
		* To keep things simple, we pass one scanline per call; you can pass
		* more if you wish, though.
		*/
		row_stride = image_width * colour_channels;	/* JSAMPLEs per row in image_buffer */
	
		while (cinfo.next_scanline < cinfo.image_height) {
			/* jpeg_write_scanlines expects an array of pointers to scanlines.
			* Here the array is only one element long, but you could pass
			* more than one scanline at a time if that's more convenient.
			*/
			row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
			(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		/* Step 6: Finish compression */

		jpeg_finish_compress(&cinfo);
		*olen = output_buffer->size();

		/* Step 7: release JPEG compression object */

		/* This is an important step since it will release a good deal of memory. */
		jpeg_destroy_compress(&cinfo);

		/* And we're done! */
		return true;
	}

}; //end namespace jpeg
