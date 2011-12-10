#define FUZZY_BITAP_HAMMINGTON_DISTANCE 1
#define FUZZY_BITAP_LEVENSHTEIN_DISTANCE 2

struct match {
	char* ptr;
	char* str;
	long start;
	long length;
};

typedef struct match match_t;

typedef struct match_list *mlist;

struct match_list {
	match_t m;
	mlist next;
};

struct found {
	mlist ml;
	long cnt;
};

/* Exported functions */

struct found fuzzy_search(void* input, long input_size, void *pattern, long pattern_size, int algo, int distance);

