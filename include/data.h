/* defines for filemode */
#define	ERROR				-1
#define REGULAR				0
#define NEW					1
#define DIRECTORY			2
#define CHARACTER_SPECIAL	3
#define BLOCK_SPECIAL		4
#define PARTIAL             5

/* regular expressions */
#define END     0
#define ONE     1
#define STAR    2

/* undo modes */
#define U_EDIT		1	/* undo o r R */
#define U_TRUNC		2	/* undo D */
#define U_INSERT	4	/* undo i */
#define U_DELETE	8	/* undo x */
#define U_BACK		16	/* undo X */
#define U_APPEND	32	/* undo P A */
#define U_TILDE		64	/* ~ */

#define S_GLOBAL	0x100

/* logic modes */
#define LSHIFT  1
#define RSHIFT  2
#define LROTATE 3
#define RROTATE 4
#define AND 5
#define OR  6
#define XOR 7
#define NEG 8
#define NOT 9

#define HEX			0
#define ASCII		1
#define FORWARD		0
#define BACKWARD	1
#define CR			'\r'
#define NL			'\n'
#define BS			8
#define	ESC			27
#define SEARCH		0

#define CMDLNG(a,b)     (len <= a && len >= b)

#ifndef NULL
#	define NULL		((void *)0)
#endif

#ifndef TRUE
#	define TRUE		1
#	define FALSE	0
#endif

#define PTR		char *
#define DELIM	'/'

/* Define escape key */
#define KEY_ESC		27

#define MAXCMD	255
#define BUFFER	1024

#define BLK_COUNT 32		/* number of data blocks */
#define MARK_COUNT 64		/* number of markers */



/* ---------------------------------------------------------
 *      Main data types for bvi and its plugins
 * ---------------------------------------------------------
 */

struct BVI {
	char* version;
};

/* ---------------------------------
 * CORE structure - each for
 * different buffers
 * ---------------------------------
 */

struct CORE {
	struct {
		int COLUMNS_DATA;
		int COLUMNS_HEX;
		int COLUMNS_ADDRESS;
		/*
		   struct colors {} */
	} params;
	struct {
		PTR mem;
		PTR maxpos;
	} editor;
	struct {
		int maxy;
		int maxx;
	} screen;
	/*
	   struct MARKERS;
	 */
};

typedef struct CORE core_t;

/* -----------------------------------
 * Current state of the current buffer
 * ----------------------------------- 
 */

#define BVI_MODE_CMD	1
#define BVI_MODE_EDIT	2
#define BVI_MODE_VISUAL 3
#define BVI_MODE_REPL	4

struct STATE {
	/* Command, Edit, Visual or REPL modes */
	int mode;
	/* Current positions */
	PTR pagepos;
	PTR curpos;
	PTR current;
	PTR mempos;
	int x;
	int y;
	int loc;
	int screen;
	int scrolly;
	int toggle_selection;
	struct {
		long start;
		long end;
	} selection;
};

typedef struct STATE state_t;

/* ----------------------------------------------------------
 *                   keys data types
 * ----------------------------------------------------------
 */

#define BVI_CTRL(n)		(n&0x1f)

#define BVI_HANDLER_INTERNAL	0
#define BVI_HANDLER_SCRIPT		1
#define BVI_HANDLER_LUA			2

struct key {
	int id;
	char *name;
	char *description;
	short enabled;
	short handler_type;
	union {
		char *lua_cmd;
		char *int_cmd;
		int (*func) ();
	} handler;
};

struct keys_array {
	struct key *arr;
	int items;
	int allocated;
};

/* ---------------------------------------------------------
 *                 commands data types
 * ---------------------------------------------------------
 */

#define FLAG_FORCE 1

struct command {
	int id;
	char *name;
	char *description;
	short enabled;
	short handler_type;
	union {
		char *lua_cmd;
		char *int_cmd;
		int (*func) (char, int, char **);
	} handler;
	int size1;
	int size2;
};

struct command_array {
	struct command *arr;
	int items;
	int allocated;
};

/* ----------------------------------------------------------
 *                blocks data types
 * ----------------------------------------------------------
 */

typedef struct block *block_link;

/* TODO:
 *
 * Add blocks annotations and block events
 *
 */
struct block_item {
	unsigned int id; /* unique id */
	char name[64]; /* for naming blocks */
	unsigned long pos_start;
	unsigned long pos_end;
	// TODO: need to be implemented as BITMASKS/FLAGS
	unsigned int hl_toggle; /* do we need highlight this block? */
	unsigned short folding; /* do we need fold this block? */
	unsigned int palette;   /* pallete, which we are using for highlight this block */
	char *annotation;
	struct {
		int event_type; // BVI_HANDLER_INTERNAL or BVI_HANDLER_LUA
		union {
			char *lua_cmd;
			char *int_cmd;
			int (*func)();
		} handler;
	} event;
};

struct block { 
	struct block_item item;
	block_link next;
};


