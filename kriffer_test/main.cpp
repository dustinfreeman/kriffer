//A series of tests for kriffer, code that stores Kinect data in riffer.
#include <iostream>
#include <vector>
#include <string>

//NOTE: string and int concatenation is easier in C++11 with std::to_string

#include <time.h>

#include <riffer.h>
#include <kriffer.h>

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

}

int main() {
	test_kinect_write();

	test_kinect_read_write();

	std::cout << "Finished all tests.\n";
	while(true) { }
}