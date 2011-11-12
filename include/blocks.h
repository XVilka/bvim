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

