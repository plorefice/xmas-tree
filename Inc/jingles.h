#ifndef __JINGLES_H__
#define __JINGLES_H__

enum JingleId {
    JINGLE_BELLS = 0,
    NUM_JINGLES
};

void jingle_start(enum JingleId jingle);
void jingle_stop(void);
void jingle_update(void);
bool jingle_is_playing(void);

#endif /* __JINGLES_H__ */