#ifdef ANSI

void ui__Init(void);
void ui__ErrorMsg(char *);
void ui__MsgWin_Show(char *, int width, int height);
int ui__ToolWin_Show(int), ui__ToolWin_Hide();
int ui__ToolWin_Print(char *, int);
short ui__ToolWin_Exist();
void ui__MainWin_Resize(int);
void printcolorline(int, int, int, char *);
void ui__Line_Print();

int ui__Color_Set(char *);
void ui__Colors_Set();
void ui__Colors_Load();
void ui__Colors_Save();

void ui__Screen_Repaint(void), ui__Screen_New(void);

#else

void ui__Init();
void ui__ErrorMsg();
void ui__MsgWin_Show();
int ui__ToolWin_Show(), ui__ToolWin_Hide();
int ui__ToolWin_Print();
short ui__ToolWin_Exist();
void ui__MainWin_Resize();
void printcolorline();
void ui__Line_Print();
int ui__lineout();

int ui__Color_Set();
void ui__Colors_Set();
void ui__Colors_Load();
void ui__Colors_Save();

void ui__Screen_New(), ui__Screen_Repaint();

#endif
