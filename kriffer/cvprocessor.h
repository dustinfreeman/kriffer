#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "processor.h"
#include "img_chunk.h"

namespace kfr {

	using namespace rfr;

	class CVProcessor : public Processor {
	public:
		cv::VideoCapture* cap;
		int c_index;

		void register_tags() {
			//ensures tags are registered with riffer
			tags::register_tag("width", "WDTH", INT_TYPE);
			tags::register_tag("height", "HGHT", INT_TYPE);
			tags::register_tag("timestamp", "MTMP", INT_64_TYPE);
			
			tags::register_tag("colour frame", "CLUR", CHUNK_TYPE);
			tags::register_tag("colour image", "CLRI", CHAR_PTR_TYPE);
		}

		CVProcessor(int _c_index, std::string _folder = "./", std::string _filename = "capture.dat", bool overwrite = true) 
			: Processor(_folder, _filename, overwrite) {
			c_index = _c_index;
			_last_colour = nullptr;

			register_tags();
			cs->index_by("timestamp");

			if (c_index >= 0) {
				//cap = new cv::VideoCapture(); //default
				cap = new cv::VideoCapture(c_index);
				//cv::waitKey(300);
			}
		}

		std::string update() {
			cv::Mat frame;
			if (!cap->isOpened()) {
				return "";
			}
			*cap >> frame;
			//frame will come in with 3 channels. We need 4.
			cv::Mat frame_4 = cv::Mat(frame.rows, frame.cols, CV_8UC4);

			int from_to[] = { 0,0, 1,1, 2,2};//, 3,3 };
			cv::mixChannels(&frame, 1, &frame_4, 1, from_to, 3);

			ImgChunk* colourChunk = new ImgChunk("colour frame");
			add_current_time(colourChunk);

			colourChunk->add_parameter("width",frame.cols);
			colourChunk->add_parameter("height",frame.rows);

			colourChunk->assign_image(frame_4.data, frame_4.cols*frame_4.rows*NUM_CLR_CHANNELS);

			int olen;
			char* comp_img = Processor::compress_image(colourChunk, "colour image", &olen, "JPEG");

			if(colourChunk->valid_compression) {
				cs->add(*colourChunk);

				//set last colour
				if (_last_colour != nullptr)
					delete _last_colour;
				_last_colour = colourChunk;
			} else {
				std::cout << "Problem with jpge compression. \n";
			}

			return "c";
		}

		void stop() {
			Processor::stop();
		}
	};
};