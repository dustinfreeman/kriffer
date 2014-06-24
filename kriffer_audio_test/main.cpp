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

	Sleep(400);

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
	std::cout << "Recording to... " << kprocessor->get_wav_filename();
	std::cout << "Press 's' to stop capturing \n";

	wchar_t ch;
	do
	{
		ch = _getwch();
		kprocessor->update();
	} while (L'S' != towupper(ch));

	//cleanup.
	kprocessor->stop_audio();
	kprocessor->stop();

	return 0;
}