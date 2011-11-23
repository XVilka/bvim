/* e.g.
 * input = "print(4+9)"
 * output = "13"
 */
struct iorecord {
	char* input;
	char* output;
};

typedef struct iorecord iorecord_t;

unsigned long io__BufferAdd();
int io__BufferAdd2(unsigned long id);
int io__BufferDestroy(unsigned long buffer_id);
unsigned long io__RecordInsert(unsigned long buffer_id, iorecord_t record);
int io__RecordDelete(unsigned long buffer_id, unsigned long record_id);
iorecord_t* io__RecordGet(unsigned long buffer_id, unsigned long record_id);
