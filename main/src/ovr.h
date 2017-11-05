#pragma once

#include "frame.h"

void initOvr();

void setupOvrFrame(Frame&);
void submitOvrFrame();

void shutdownOvr();
