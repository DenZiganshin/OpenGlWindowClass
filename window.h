#ifndef WINDOWH
#define WINDOWH

#define GL_BGRA_EXT 0x80E1

#include <windows.h>
#include <io.h>
#include <gl/gl.h>
#include <stdio.h>
//#include <Uxtheme.h>
#include <string>
#include <gdiplus.h>


class CWindow{
	HWND _hwnd;
	HDC _hdc;
	bool _exit;
	int _WndWidth, _WndHeight,
		_initX, _initY;

public: //array for keyboard
	BYTE _keys[256];
private:
	typedef void (*mainLoop)();
	mainLoop _loopFunc;
public:
	CWindow();
	bool initWnd(int x,int y,int w,int h,HINSTANCE hInstance);

	void startMainLoop(mainLoop f);
	void shutdown();
	int makeTexture(std::string name, BYTE **outData = NULL);

	void drawImg(GLuint texId, float x, float y, float w, float h);
	void drawImg(GLuint texId, float x, float y, float w, float h, float imgX, float imgY, float imgW, float imgH);
    void drawRect(float r, float g, float b, float x, float y, float w, float h);
    void drawLine(float r, float g, float b, float x1, float y1, float x2, float y2);
    void drawBitmap(unsigned w, unsigned h, BYTE * data);

	void clearWindow();
	void swapBuffers();

private:
	bool initGl();
	void getArgs();
	static LRESULT CALLBACK msgWindowFunc_static(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK msgWindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};


#endif
