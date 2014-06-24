//A series of tests for kriffer, code that stores Kinect data in riffer.
#include <iostream>
#include <string>

#include <kriffer.h>


int main() {

	//Open one of the Kinect Processors
	int kinect_type;
	kfr::KProcessor *kprocessor = nullptr;
	while (true) {
		std::cout << "Open Kinect (1 - Kinect 1; 2 - Kinect 2): \n";
		std::cin >> kinect_type;

		if (kinect_type == 1) {
			kprocessor = new kfr::K1Processor(0, RFR_DEFAULT_FOLDER, RFR_DEFAULT_FILENAME, kfr::CAPTURE_AUDIO);
		} else if (kinect_type == 2) {
			kprocessor = new kfr::K2Processor(0, RFR_DEFAULT_FOLDER, RFR_DEFAULT_FILENAME, kfr::CAPTURE_AUDIO);
		}
		else {
			std::cout << "Invalid Kinect Type \n";
			return -1;
		}

		if (kprocessor != nullptr)
			break;
	}

	std::cout << "Opening Kinect " << kinect_type << "\n";

	Sleep(1000);

	//start audio
	char command;
	std::cout << "Press r to start recording \n";
	std::cin >> command;
	if (command == 'r') {
		kprocessor->start_audio_index(0);
	}
	else {
		std::cout << "Invalid key \n";
		return -1;
	}

	//loop
	int test_duration = 5;
	std::cout << "Recording for " << test_duration << "seconds to... " << kprocessor->get_wav_filename() << "\n";

	time_t start;	time_t end;
	time(&start);	time(&end);
	while (difftime(end, start) < test_duration) {
		time(&end);
		kprocessor->update();
		Sleep(10);
	}

	std::cout << "...done.\n";

	//cleanup.
	kprocessor->stop_audio();
	kprocessor->stop();

	return 0;
}