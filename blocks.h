typedef struct block *block_link;

struct block_item {
	int id; /* unique id */
	char name[64]; /* for naming blocks */
	unsigned long pos_start;
	unsigned long pos_end;
	unsigned int hl_toggle; /* do we need highlight this block? */
	unsigned short folding; /* do we need fold this block? */
	unsigned int palette;   /* pallete, which we are using for highlight this block */
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
struct block_item* blocks__GetByID(int id);
struct block_item* blocks__GetByName(char* name);

