#ifndef EMGUI_PORT_OPTS_H
#define EMGUI_PORT_OPTS_H

#include <stdint.h>
#include "Adafruit-GFX/gfxfont.h"

#define LCD_STATUS_BAR_HEIGHT 24

#define EMGUI_XPICTURE_TYPE char*
#define EMGUI_XFONT_TYPE    const GFXfont *

#define EM_GUI_PIC_CROSS		"DemoImages/close.png"
#define EM_GUI_PIC_WRENCH		"DemoImages/wrench.png"
#define EM_GUI_PIC_YES			"DemoImages/ok.png"
#define EM_GUI_PIC_NO			"DemoImages/no.png"
#define EM_GUI_PIC_REFRESH		"DemoImages/undo.png"
#define EM_GUI_PIC_OPENFOLDER	"DemoImages/folder.png"
#define EM_GUI_PIC_PROCESS		"DemoImages/process.png"
#define EM_GUI_PIC_MAGIC		"DemoImages/magic.png"
#define EM_GUI_PIC_HELP			"DemoImages/help.png"


// TODO: MAKE RETURN BUTTON !!
#ifndef EM_GUI_PIC_RETURN 
#define EM_GUI_PIC_RETURN				"DemoImages/undo.png"
#endif

#endif // !EMGUI_PORT_OPTS_H