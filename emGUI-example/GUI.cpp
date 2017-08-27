/*
	This is main GUI source

*/
#include "GUI.h"

using namespace Gdiplus;



extern LCD_Glue LCD;
extern xInterface * interface1;
extern xWidget* wdg;
extern xLabel * mouseMonitor;


// Convert color from emGUI to GDI format
ARGB convertColor(uint16_t color) {
	uint8_t usRed = ((color >> 11) & 0b00011111) * 8;
	uint8_t usGreen = ((color >> 5) & 0b00111111) * 4;
	uint8_t usBlue = ((color & 0b00011111)) * 8;

	ARGB answer = (ARGB)((ARGB)0xFF000000 | (usRed << 16) | (usGreen << 8) | usBlue);
	return answer;

}



// Action on interface creatings
bool myHandler(xWidget *) {
	auto window = pxWindowCreate(WINDOW_MENU);
	vWindowSetHeader(window, "Wnd1");
	vWidgetSetBgColor(window, 0xFF, false);
	auto l1 = pxLabelCreate(1, 1, 238, 60, "hypothetical rosters of players \
  considered the best in the nation at their respective positions\
  The National Collegiate Athletic Association, a college sports \
  governing body, uses officially recognized All-America selectors \
  to determine the consensus selections. These are based on a point \
  system in which a player is awarded three points for every selector \
  that names him to the first team, two points for the second team, \
    and one point for the third team. The individual who receives the \
    most points at his position is called a consensus All-American.[4] \
    Over time, the sources used to determine the consensus selections \
    have changed, and since 2002, the NCAA has used five selectors, \
    the Associated Press (AP), American Football Coaches Association \
    (AFCA), Football Writers Association of America (FWAA), The Sporting \
    News (TSN), and the Walter Camp Football Foundation (WCFF),   \
    to determine consensus All-Americans.[5]", FONT_ASCII_8_X, 1010, window);

	mouseMonitor = pxLabelCreate(1, 200, 238, 0, "x:   y:   ", FONT_ASCII_8_X, 500, window);
	auto b1 = pxButtonCreate(60, 100, rgb_test, window);

	auto window2 = pxWindowCreate(WINDOW_ABOUT);
	auto labelAbout = pxLabelCreate(1, 1, 238, 60, "This is Demo for emGUI. 2017", FONT_ASCII_8_X, 50, window2);

	vWindowSetOnCloseRequestHandler(window2, &onCloseWAHandler);
	vInterfaceOpenWindow(WINDOW_ABOUT);
	vInterfaceOpenWindow(WINDOW_MENU);
	return true;
}


bool onCloseWAHandler(xWidget *) {
	vInterfaceOpenWindow(WINDOW_MENU);
	return true;
}

bool initGUI() {

	return true;
}