#ifndef EMGUI_PORT_OPTS_H
#define EMGUI_PORT_OPTS_H

#include <stdint.h>
#include "Adafruit-GFX/gfxfont.h"

#define EMGUI_XPICTURE_TYPE char*
#define EMGUI_XFONT_TYPE    const GFXfont *

#define EMGUI_DEBUG 1

#define EM_GUI_PIC_WRENCH		"DemoImages/wrench.png"
#define EM_GUI_PIC_OPENFOLDER	"DemoImages/folder.png"
#define EM_GUI_PIC_PROCESS		"DemoImages/process.png"
#define EM_GUI_PIC_MAGIC		"DemoImages/magic.png"
#define EM_GUI_PIC_HELP			"DemoImages/help.png"


// TODO: MAKE RETURN BUTTON !!
#ifndef EM_GUI_PIC_RETURN 
#define EM_GUI_PIC_RETURN				"DemoImages/undo.png"
#endif

#endif // !EMGUI_PORT_OPTS_H