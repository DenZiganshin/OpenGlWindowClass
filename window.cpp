#include "window.h"


CWindow::CWindow(){
    _WndWidth = 640;
    _WndHeight = 480;
    _initX = 20;
    _initY = 20;
    _exit = false;

}

/*
*	make win32 window
*/
bool CWindow::initWnd(int x,int y,int w,int h,HINSTANCE hInstance){
    DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    char className[] = "ResultWindow";

	_initX = x;
	_initY = y;
	_WndWidth = w;
	_WndHeight = h;

    //init window class
    WNDCLASS wc;
    ZeroMemory(&wc,sizeof(wc));
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.lpfnWndProc = CWindow::msgWindowFunc_static;
    wc.style=CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.lpszMenuName=NULL;
    wc.cbClsExtra=0;
    wc.cbWndExtra=0;
    if(!RegisterClass (&wc))
        return false;

    //calc needle size
    RECT rect = {0,0,_WndWidth,_WndHeight};
    AdjustWindowRect(&rect,windowStyle,false);
    //create window
    _hwnd = CreateWindow(
        className,
        "Title",
        windowStyle,
        _initX,_initY,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,NULL,hInstance,NULL);

    //send link to 'me'
    SetWindowLongPtr(_hwnd,GWLP_USERDATA, (LONG_PTR) this );

    //init ogl
    if(!initGl()){
        return false;
    }

    //show window
    ShowWindow(_hwnd, SW_SHOW);
    UpdateWindow(_hwnd);
    return true;
}


/*
*	starts window main loop
*/
void CWindow::startMainLoop(mainLoop f){
    _loopFunc = f;
    //last time
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while(!_exit){
        if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
            if( (msg.message == WM_CLOSE) ||
                (msg.message == WM_DESTROY) ||
                (msg.message == WM_QUIT) ){
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        //fps count
        _loopFunc();
    }
}

/*
*	makes texture from file, return ogl texture id;
*/
int CWindow::makeTexture(std::string name, BYTE **outData){
    //check: file existance
    if(_access(name.c_str(), 4) == -1){
        printf("error:%s - not found\n",name.c_str());
        return -1;
    }

    //init GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    //load Bitmap
	std::wstring wname(name.begin(), name.end());
	//check readable
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(wname.c_str());

    //get image demensions
    UINT dimensionCount;
    GUID *dimensionIds = NULL;
    dimensionCount = bitmap->GetFrameDimensionsCount();
    dimensionIds = (GUID*)malloc(dimensionCount * sizeof(GUID));
    bitmap->GetFrameDimensionsList(dimensionIds, dimensionCount);


    //prepare bitmapData structs
    Gdiplus::BitmapData* bitmapData = new Gdiplus::BitmapData;
    int imgWidth = bitmap->GetWidth();
    int imgHeight = bitmap->GetHeight();
    Gdiplus::Rect rect(0, 0, imgWidth, imgHeight);

    BYTE* ImageBuffer = (BYTE*)malloc(sizeof(BYTE*) *imgWidth*imgHeight*4);
    BYTE* pixels;

    //get data
    bitmap->SelectActiveFrame(&dimensionIds[0],0);
    bitmap->LockBits(
            &rect,
            Gdiplus::ImageLockModeRead,
            PixelFormat32bppARGB,
            bitmapData);

    pixels = (BYTE*)bitmapData->Scan0;
    //--//copy bitmap data
    memcpy(ImageBuffer, pixels, sizeof(BYTE)*4*imgHeight*imgWidth);
    //second copy to output
    if(outData != NULL){
        *outData = (BYTE*)malloc(sizeof(BYTE*) *imgWidth*imgHeight*4);
        memcpy(*outData, pixels, sizeof(BYTE)*4*imgHeight*imgWidth);
    }

    //close orig data
    bitmap->UnlockBits(bitmapData);


    //generate texture id
    GLuint id;
    glGenTextures(1,&id);
    //make texture
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, imgWidth, imgHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, ImageBuffer);

    //cleanup
    free(dimensionIds);
    free(ImageBuffer);

    delete bitmapData;
    delete bitmap;
    Gdiplus::GdiplusShutdown(gdiplusToken);

    //return
    return id;
}

/*
*	draw texture in certain cell
*/
void CWindow::drawImg(GLuint texId, float x, float y, float w, float h){
    drawImg(texId, x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f);
}

void CWindow::drawImg(GLuint texId, float x, float y, float w, float h, float imgX, float imgY, float imgW, float imgH){
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glColor3f(1.0f,1.0f,1.0f);
    glBindTexture(GL_TEXTURE_2D, texId);

    glBegin(GL_QUADS);
      glVertex2f(x, y);
      glTexCoord2f(imgX, imgY);

      glVertex2f(x, y+h);
      glTexCoord2f(imgX+imgW, imgY);

      glVertex2f(x+w, y+h);
      glTexCoord2f(imgX+imgW, imgY+imgH);

      glVertex2f(x+w, y);
      glTexCoord2f(imgX, imgY+imgH);

    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

}

void CWindow::drawBitmap(unsigned w, unsigned h, BYTE * data){
    glRasterPos2d(100,100);
    glPixelZoom(1,1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(w,h,GL_BGRA_EXT, GL_UNSIGNED_BYTE,data);
}

/*
*   draw rectangle
*/
void CWindow::drawRect(float r, float g, float b, float x, float y, float w, float h){
    glColor3f(r, g, b);

    glBegin(GL_QUADS);
      glVertex2f(x, y + h); //1
      glVertex2f(x + w ,y + h); //2
      glVertex2f(x + w,	y); //3
      glVertex2f(x, y); //4
    glEnd();
}

void CWindow::drawLine(float r, float g, float b, float x1, float y1, float x2, float y2){
    glColor3f(r, g, b);

    glBegin(GL_LINES);
      glVertex2f(x1, y1); //1
      glVertex2f(x2, y2); //2
    glEnd();
}

/*
*	clear ogl draw buffer
*/
void CWindow::clearWindow(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/*
*	swap opengl display buffers
*/
void CWindow::swapBuffers(){
    SwapBuffers(_hdc);
}




/*
*	initialize some ogl params
*/
bool CWindow::initGl(){
    HGLRC hRC;
    GLuint PixelFormat;
    PIXELFORMATDESCRIPTOR pfd=
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        16,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    if (!(_hdc=GetDC(_hwnd)))
    {
        printf("Can't Create A GL Device Context.\n");
        return false;
    }

    if (!(PixelFormat=ChoosePixelFormat(_hdc,&pfd)))
    {
        printf("Can't Find A Suitable PixelFormat.\n");
        return false;
    }

    if(!SetPixelFormat(_hdc,PixelFormat,&pfd))
    {
        printf("Can't Set The PixelFormat.\n");
        return false;
    }

    if (!(hRC=wglCreateContext(_hdc)))
    {
        printf("Can't Create A GL Rendering Context.\n");
        return false;
    }

    if(!wglMakeCurrent(_hdc,hRC))
    {
        printf("Can't Activate The GL Rendering Context.\n");
        return false;
    }


    glShadeModel( GL_SMOOTH );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //calc client coords
    int dt = GetSystemMetrics(SM_CXBORDER);
    // top left 'zero' bug
    glViewport(dt, dt, _WndWidth, _WndHeight);
    glOrtho(0 ,_WndWidth, 0, _WndHeight, -1, 1);

    glMatrixMode(GL_MODELVIEW);

    return true;
}


/*
*	commands to take command line arguments
*/
void CWindow::getArgs(){
    //получение аргументов командной строки
    LPTSTR line = GetCommandLine();
    int argc;
}

void CWindow::shutdown(){
    _exit = true;
}


/*
*	window messager func
*/
LRESULT CALLBACK CWindow::msgWindowFunc_static(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    CWindow *me = (CWindow*) GetWindowLong(hwnd, GWLP_USERDATA);
    return me->msgWindowFunc(hwnd,message,wParam,lParam);
}

LRESULT CALLBACK CWindow::msgWindowFunc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    switch(message){
        //управление
    case WM_PAINT:
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        //клавиатура
    case WM_KEYDOWN:
        _keys[wParam] = true;
        break;
    case WM_KEYUP:
        _keys[wParam] = false;
        break;
        //изменение параметров окна
    case WM_SIZING: //sizing in progress
        break;
    case WM_SIZE: //sized
        break;
    case WM_MOVING:
        break;
    case WM_MOVE:
        break;
        //сообщения мыши
    case WM_LBUTTONDBLCLK:
        break;
    case WM_MOUSEMOVE:
        break;
    case WM_WINDOWPOSCHANGED:
        break;
    case WM_MOUSELEAVE:
        break;
    case WM_RBUTTONDOWN:
        //Functions::wndMouseFunc(WM_RBUTTONDOWN,LOWORD(lParam), HIWORD(lParam), 0);
        break;
    case WM_RBUTTONUP:
        break;
    case WM_LBUTTONDOWN:
        break;
    case WM_LBUTTONUP:
        break;
    case WM_MOUSEWHEEL:
        break;
    default:
        return DefWindowProc (hwnd,message,wParam,lParam);
    }
}
