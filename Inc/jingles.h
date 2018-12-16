#ifndef __JINGLES_H__
#define __JINGLES_H__

enum jingle_id {
    NO_JINGLE = 0,
    JINGLE_BELLS,
    IMPERIAL_MARCH,
    SUPER_MARIO,
    NUM_JINGLES
};

void jingle_start(enum jingle_id jingle);
void jingle_stop(void);
void jingle_update(void);
bool jingle_is_playing(void);

#endif /* __JINGLES_H__ */