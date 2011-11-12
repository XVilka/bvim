void keys__Init();
void keys__Destroy();
int keys__Key_Pressed(int);
struct key *keys__KeyString_Parse(char *);
int keys__Key_Map(struct key *);
int keys__Key_Unmap(struct key *);
void keys__KeyMaps_Show();

