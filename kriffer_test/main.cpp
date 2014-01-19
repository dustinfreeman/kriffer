//A series of tests for kriffer, code that stores Kinect data in riffer.
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
//NOTE: string and int concatenation is easier in C++11 with std::to_string

#include <windows.h> //for threading.
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

	std::vector<kfr::KProcessor> kp;
	std::vector<rfr::CaptureSession> cs;

	for (int i = 0; i < num_kinects; i++) {
		std::ostringstream capture_file_name;
		capture_file_name << "capture" << i << ".knt";

		cs.push_back(rfr::CaptureSession(capture_file_name.str()));

		kp.push_back(kfr::KProcessor(i, cs[i])); //get ith Kinect and assign to ith capture session.

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
			std::string new_frames = kp[i].update();
			std::cout << new_frames;
		}
		std::cout << ",";
	}
	std::cout << "\n";

	for (int i = 0; i < kp.size(); i++) {
		kp[i].stop();
		cs[i].close();
	}
}

int main() {
	test_kinect_write();

	std::cout << "Finished all tests.\n";
	while(true) { }
}