typedef struct hl_t *hl_link;

struct hl_item {
	unsigned int id; /* unique id */
	char name[64]; /* for naming highlights */
	unsigned long hex_start;
	unsigned long hex_end;
	unsigned long dat_start;
	unsigned long dat_end;
	int flg;
	unsigned int palette;   /* pallete, which we are using for highlight */
	int toggle; /* is this highlighting enabled? */
};

struct hl_t { 
	struct hl_item item;
	hl_link next;
};


struct repl_t {
	int current_y;
	int current_x;
};

void ui__Init(void);
void ui__ErrorMsg(char *);
void ui__SystemErrorMsg(char *);
void ui__StatusMsg(char *);
void ui__MsgWin_Show(char *, int width, int height);

int ui__ToolWin_Show(int), ui__ToolWin_Hide();
int ui__ToolWin_Print(char *, int);
short ui__ToolWin_Exist();

int ui__REPL_Main();
int ui__REPLWin_Show();
int ui__REPLWin_Hide();
int ui__REPLWin_print(const char*);
short ui__REPLWin_Exist();

void ui__MainWin_Resize(int);
void printcolorline(int, int, int, char *);
void ui__Line_Print();

int ui__Color_Set(char *);
void ui__Colors_Set();
void ui__Colors_Load();
void ui__Colors_Save();

void ui__Screen_Repaint(void), ui__Screen_New(void);

int ui__BlockHighlightAdd(struct block_item *blk);

