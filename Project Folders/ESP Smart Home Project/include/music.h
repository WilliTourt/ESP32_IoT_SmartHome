
/*
  Doodle Frequencies
        Eb      F       G       Ab      Bb      C       D
  H1    622     698     785     832     932     1047    1176
  H2    1245    1398    1569    1661    1865    2093    2349

  160bpm -> 93.75ms per beat
*/

#define Eb 622
#undef F
#define F 698
#define G 785
#define Ab 832
#define Bb 932
#define C 1047
#define D 1176

#define BEAT_QUARTER 94

const int doodle_intro[] = {
    Bb, 0, Eb, 0,    Bb, 0, Eb, 0,    Bb, 0, Eb, 0,    Bb, 0, Eb, 0
};

const int doodle_main[] = {
    Bb, 0, Eb, 0,   Eb, 0, F, 0,    G, 0, 0, 0,     F, 0, 0, 0,     Eb, 0, Eb, Eb,     Eb, 0, F, 0,     G, 0, 0, 0,     F, 0, 0, 0, 
    Bb, 0, Eb, 0,   Eb, 0, F, 0,    G, 0, 0, 0,     F, 0, 0, 0,     Eb, 0, Eb, Eb,     Eb, 0, Ab, 0,    G, 0, 0, 0,     F, 0, 0, 0,   
};
