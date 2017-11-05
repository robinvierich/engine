#pragma once

#include "frame.h"

class OvrRenderer {

public:
	void init(HWND hWnd);
	void shutdown();

	void update(Frame& frame, int windowWidth, int windowHeight);
}