#include <stdio.h>
#include "window.h"

CWindow WINDOW;


GLuint testTex;


void cycle(){
	static DWORD t2 = 0;
	static short delay = 30;

	//calcs
	//draw
	WINDOW.clearWindow();
	WINDOW.drawImg(testTex,30,30, 150, 150, 0,0, 1.0f/3, 1);
	//WINDOW.drawLine(1,0,0, 10,0, 10,290);
	WINDOW.swapBuffers();
	//check input
	if(WINDOW._keys[VK_ESCAPE]){
        WINDOW.shutdown();
	}
	//time
	DWORD t1 = GetTickCount();
	if((t2 - t1) < delay){
		Sleep(delay - (t2-t1));
	}
	t2 = t1 + (delay - (t2-t1));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	//initWindow
	if(!WINDOW.initWnd(120,120, 640, 480, hInstance))
		return 1;
    testTex = WINDOW.makeTexture("hero.png");
    if(testTex == -1)
        return 1;

	//start cycle
	WINDOW.startMainLoop(cycle);

	return 0;
}


