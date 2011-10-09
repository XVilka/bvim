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

void keys__Init();
void keys__Destroy();
int keys__Key_Pressed(int);
struct key *keys__KeyString_Parse(char *);
int keys__Key_Map(struct key *);
int keys__Key_Unmap(struct key *);
void keys__KeyMaps_Show();
