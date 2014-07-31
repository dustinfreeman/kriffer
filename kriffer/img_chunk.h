#pragma once

#include <riffer.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "jpg\jpgd.h"
#include "jpg\jpge.h"

#define PADDING_FACTOR 4
#define NUM_CLR_CHANNELS 4

namespace kfr {
	using namespace rfr;

	struct ImgChunk : Chunk { 
		//a struct to hold uncompressed image data, which is not written to disk.
		//makes no assumption about image format.

		unsigned char* image;
		unsigned int image_size;
		int width;
		int height;
		int bpp;

		bool valid_compression;
		
		ImgChunk(std::string _tag_name = NULL_TAG) : Chunk(_tag_name) {
			image = nullptr;
			valid_compression = false;
		}

		~ImgChunk() {
			delete[] image;
		}

		kfr::ImgChunk copy() {
			//returns a copy of this ImgChunk
			kfr::ImgChunk new_image_chunk(*this);

			unsigned char* copy_image = new unsigned char[image_size];
			memcpy(copy_image, this->image, image_size);

			new_image_chunk.image = copy_image;
			return new_image_chunk;
		}

		void assign_image(void* _image, int _size) {
			//copies the image into local, so
			// _image should be deleted when this is finished if
			// there is no intention to use it elsewhere.
			if (image)
				delete image;
			
			image_size = _size;
			image = new unsigned char[image_size];
			memcpy(image, _image, image_size);

			width = (*this->get_parameter<int>("width"));
			height = (*this->get_parameter<int>("height"));

			bpp = image_size / width / height;
			//std::cout << "BPP: " << bpp << "\n";
		}

		cv::Mat get_mat_image(int smaller_width, int smaller_height, int cv_data_type = CV_8UC4) {
			//returns image, assuming given dims are equal to, or less than this image's dims
			// with cropping.
			
			if (smaller_width > width || smaller_height > height) {
				std::cout << "image not as expected, returning blank. " << width << "," << height << " \n";
				return cv::Mat(smaller_height, smaller_width, CV_8UC4);
			}
			
			cv::Mat full_img = cv::Mat(height, width, cv_data_type, (void*)this->image);

			//same size.
			if (smaller_width == width && smaller_height == height)
				return full_img;

			cv::Rect croppedROI( (width - smaller_width)/2, (height - smaller_height)/2, smaller_width, smaller_height);

			cv::Mat cropped_img = full_img(croppedROI);

			return cropped_img;
		}
	};
}