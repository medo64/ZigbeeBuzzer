## Zigbee Buzzer Software

### File Format

Each tone in file consists of note and octave char that control the frequency
and duration char that controls length of the note in hundreds of a second.
Those three fields are then repeated. All whitespace characters are ignored.

E.g. `A46` will play A4 note for 60 ms.

#### Notes

| Char | Note |
|------|------|
|  C   |  C   |
|  c   |  C#  |
|  D   |  D   |
|  d   |  D#  |
|  E   |  E   |
|  F   |  F   |
|  f   |  F#  |
|  G   |  G   |
|  g   |  G#  |
|  A   |  A   |
|  a   |  A#  |
|  B   |  B   |

#### Octave

Octave can be any digit from `0` to `9`.

#### Duration

| Char | Duration |
|------|----------|
|  0   | 1000 ms  |
|  1   |  500 ms  |
|  2   |  250 ms  |
|  3   |  120 ms  |
|  4   |   60 ms  |
|  5   |   30 ms  |
|  6   |   20 ms  |
|  7   |   10 ms  |

| Char | Duration |
|------|----------|
|  A   |   10 ms  |
|  B   |   20 ms  |
|  C   |   30 ms  |
|  D   |   40 ms  |
|  E   |   50 ms  |
|  F   |   60 ms  |
|  G   |   70 ms  |
|  H   |   80 ms  |
|  I   |   90 ms  |
|  J   |  100 ms  |
|  K   |  110 ms  |
|  L   |  120 ms  |
|  M   |  130 ms  |
|  N   |  140 ms  |
|  O   |  150 ms  |
|  P   |  160 ms  |
|  Q   |  170 ms  |
|  R   |  180 ms  |
|  S   |  190 ms  |
|  T   |  200 ms  |
|  U   |  210 ms  |
|  V   |  220 ms  |
|  W   |  230 ms  |
|  X   |  240 ms  |
|  Y   |  250 ms  |
|  a   |  260 ms  |
|  b   |  270 ms  |
|  c   |  280 ms  |
|  d   |  290 ms  |
|  e   |  300 ms  |
|  f   |  310 ms  |
|  g   |  320 ms  |
|  h   |  330 ms  |
|  i   |  340 ms  |
|  j   |  350 ms  |
|  k   |  360 ms  |
|  l   |  370 ms  |
|  m   |  380 ms  |
|  n   |  390 ms  |
|  o   |  400 ms  |
|  p   |  410 ms  |
|  q   |  420 ms  |
|  r   |  430 ms  |
|  s   |  440 ms  |
|  t   |  450 ms  |
|  u   |  460 ms  |
|  v   |  470 ms  |
|  w   |  480 ms  |
|  x   |  490 ms  |
|  y   |  500 ms  |
