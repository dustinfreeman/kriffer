#include <NuiApi.h>

#include <riffer.h>

namespace kfr {
	using namespace rfr;

	struct KProcessor {
		CaptureSession cs;

		KProcessor(int k_index, CaptureSession _cs) {
			cs = _cs;

			//TODO open kinect
		}

		void stop() {

			//stop pushing to capture session and wrap up.
		}
	};
};