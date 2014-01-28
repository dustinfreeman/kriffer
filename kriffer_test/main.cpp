//A series of tests for kriffer, code that stores Kinect data in riffer.
#include <iostream>
#include <vector>
#include <string>

#include <assert.h>

//NOTE: string and int concatenation is easier in C++11 with std::to_string

#include <time.h>

#include <riffer.h>
#include <kriffer.h>

//Test helper functions =============================
bool byte_compare(const char* array1, const char* array2, int length) {
	//just in case this functionality doesn't exist already.
	for (int i = 0; i < length; i++) {
		if (array1[i] != array2[i])
			return false;
	}
	return true;
}

void test_kinect_write() {
	int num_kinects = kfr::get_num_kinects();
	if (num_kinects < 1) {
		std::cout << "No Kinects found.\n";
		return;
	} else {
		std::cout << "Found " << num_kinects << " Kinects.\n";
	}

	std::vector<kfr::KProcessor*> kp;

	for (int i = 0; i < num_kinects; i++) {
		std::ostringstream capture_file_name;
		capture_file_name << "capture" << i << ".knt";

		kp.push_back(new kfr::KProcessor(i, capture_file_name.str())); //get ith Kinect

		break; //for now, break after 1.
	}
	//now, the KinectProcessor should be writing frames to the capture session!
	
	int test_duration = 10; //seconds.
	time_t start;	time_t end;
	time(&start);	time(&end);
	std::cout << "Running update() for " << test_duration << " seconds.\n";
	while (difftime(end,start) < test_duration) {
		time(&end);
		for (int i = 0; i < kp.size(); i++) {
			std::string new_frames = kp[i]->update();
			std::cout << new_frames;
		}
		std::cout << ",";
	}
	std::cout << "\n";

	for (int i = 0; i < kp.size(); i++) {
		kp[i]->stop();
	}
}

void test_kinect_read_write() {
	//open KProcessor without kinect.
	//create colour images and artifically add jpeg to k capture session
	//retrieve images and test for similarity.
	kfr::KProcessor kp(-1);

	//create image
	kfr::ImgChunk* chunk = new kfr::ImgChunk("colour frame"); 
	int64_t timestamp = 1;
	chunk->add_parameter("timestamp", timestamp);

	int width = 640; int height = 480;
	chunk->add_parameter("width", width);
	chunk->add_parameter("height", height);

	int img_length = width*height*4;
	char* image_bytes = new char[img_length]; //4 bpp
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			char intensity = 255*((width-x) + y)/(width + height);
            image_bytes[4*(x + y*height) + 0] = intensity;
            image_bytes[4*(x + y*height) + 1] = intensity;
            image_bytes[4*(x + y*height) + 2] = intensity;
            image_bytes[4*(x + y*height) + 3] = 255;
		} 	
	}
	chunk->assign_image((BYTE*)image_bytes, img_length);

	//add image to capture session
	kp.cs->add(*chunk);

	//fetch and compare
	kfr::ImgChunk* fetched_chunk = kp.get_colour(timestamp);

	std::cout << "fetched_chunk timestamp " << fetched_chunk->get_parameter<int64_t>("timestamp") << "\n";

	unsigned int comp_length;
	unsigned int comp_length_2;
	const char* comp = chunk->get_parameter_by_tag_as_char_ptr<char*>(rfr::tags::get_tag("colour image"), &comp_length);
	const char* comp2 = fetched_chunk->get_parameter_by_tag_as_char_ptr<char*>(rfr::tags::get_tag("colour image"), &comp_length_2);
	if (comp_length != comp_length_2) {
		std::cout << "Warning: compressed sizes differ\n";
	}
	assert(byte_compare(comp, comp2, comp_length));

	//wait, I'm an idiot. It's lossy compression; this should never succeed.
	//assert(byte_compare((char*)chunk->image, (char*)fetched_chunk->image, chunk->image_size));

	kp.stop();
}

int main() {
	//test_kinect_write();

	test_kinect_read_write();

	std::cout << "Finished all tests.\n";
	while(true) { }
}