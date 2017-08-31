#ifndef EMGUI_OPTS_H
#define EMGUI_OPTS_H

#include "DemoImages/pics.h"

#define LCD_STATUS_BAR_HEIGHT 24




#define EM_GUI_OVERRIDE_DEFAULT_PICS 1

#ifdef EM_GUI_OVERRIDE_DEFAULT_PICS
#define XPICTURE_TYPE char*
#endif // EM_GUI_OVERRIDE_DEFAULT_PICS



#define EM_GUI_PIC_CROSS	"DemoImages/close.png"
 
#define EM_GUI_PIC_LABEL_BG			"DemoImages/wrench.png"

#define EM_GUI_PIC_YES					"DemoImages/ok.png"

#define EM_GUI_PIC_NO					"DemoImages/no.png"

#define EM_GUI_PIC_REFRESH				"DemoImages/undo.png"
#define EM_GUI_PIC_OPENFOLDER	"DemoImages/folder.png"
		
#define EM_GUI_PIC_PROCESS		"DemoImages/process.png"

#define EM_GUI_PIC_MAGIC		"DemoImages/magic.png"

#define EM_GUI_PIC_HELP			"DemoImages/help.png"


// TODO: MAKE RETURN BUTTON !!
#ifndef EM_GUI_PIC_RETURN 
#define EM_GUI_PIC_RETURN				"DemoImages/undo.png"
#endif

//#define EM_GUI_PIC_CROSS rgb_test


//#define PIC_CROSS rgb_test
#endif // !EMGUI_OPTS_H