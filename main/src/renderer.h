#pragma once

#include "frame.h"

void initRenderer(HWND hWnd);
void shutdownRenderer();

void updateRenderer(Frame& frame, int windowWidth, int windowHeight);