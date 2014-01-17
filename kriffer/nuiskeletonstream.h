#include <nuistream.h>

struct NuiSkeletonStream : NuiStream {
	NuiSkeletonStream(INuiSensor * _pNuiSensor) { 
		pNuiSensor = _pNuiSensor;
	}

	void init() {
		// Create an event that will be signaled when depth data is available
		frameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		DWORD flags = 0;
		//http://msdn.microsoft.com/en-us/library/nuisensor.inuisensor.nuiskeletontrackingenable.aspx
		/*(m_seated ? NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT : 0) | (m_near ? NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE : 0)
        | (ChooserModeDefault != m_chooserMode ? NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS : 0);*/
		pNuiSensor->NuiSkeletonTrackingEnable(frameEvent, flags);
	}
};