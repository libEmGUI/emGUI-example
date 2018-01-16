#ifndef EMGUI_PORT_OPTS_H
#define EMGUI_PORT_OPTS_H

#include <stdint.h>
#include "Adafruit-GFX/gfxfont.h"

#define EMGUI_XPICTURE_TYPE char*
#define EMGUI_XFONT_TYPE    const GFXfont *

//#define EMGUI_DEBUG 1

//#define EMGUI_STATUS_BAR_HEIGHT	40

// TODO: MAKE RETURN BUTTON !!
#ifndef EM_GUI_PIC_RETURN 
#define EM_GUI_PIC_RETURN				"DemoImages/undo.png"
#endif

#endif // !EMGUI_PORT_OPTS_H