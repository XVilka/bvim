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

/* Internal abstractions */

int InitBlocksList();
block_link BlockNew();
void BlockFree();
void BlockInsertNext();
block_link BlockDeleteNext();
block_link BlockNext();
struct block_item BlockGet();

/* External interface */

int blocks__Iterator(int (*(func))(), int result);
int blocks__Add(struct block_item b);
int blocks__Init();
int blocks__Destroy();
int blocks__DelByID(int id);
int blocks__DelByName(char* name);
struct block_item* blocks__GetByID(unsigned int id);
struct block_item* blocks__GetByName(char* name);

