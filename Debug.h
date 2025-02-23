#pragma once

#ifndef DEBUGGING_MODE
	//#define DEBUGGING_MODE 
	#ifdef DEBUGGING_MODE
		#include <iostream>
		//std::ofstream DEBUG_FILE("logs/debug.dat", std::ios::out);
		#define DEBUG_OUT(message) std::cout << message << std::endl
	#else
		#define DEBUG_OUT(message) if(false) false
	#endif
#endif