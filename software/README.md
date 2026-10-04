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
|  1   |   10 ms  |
|  2   |   20 ms  |
|  3   |   30 ms  |
|  4   |   40 ms  |
|  5   |   50 ms  |
|  6   |   60 ms  |
|  7   |   70 ms  |
|  8   |   80 ms  |
|  9   |   90 ms  |
|  A   |  100 ms  |
|  B   |  110 ms  |
|  C   |  120 ms  |
|  D   |  130 ms  |
|  E   |  140 ms  |
|  F   |  150 ms  |
|  G   |  160 ms  |
|  H   |  170 ms  |
|  I   |  180 ms  |
|  J   |  190 ms  |
|  K   |  200 ms  |
|  L   |  210 ms  |
|  M   |  220 ms  |
|  N   |  230 ms  |
|  O   |  240 ms  |
|  P   |  250 ms  |
|  Q   |  260 ms  |
|  R   |  270 ms  |
|  S   |  280 ms  |
|  T   |  290 ms  |
|  U   |  300 ms  |
|  V   |  310 ms  |
|  W   |  320 ms  |
|  X   |  330 ms  |
|  Y   |  340 ms  |
|  Z   |  350 ms  |
|  a   |  360 ms  |
|  b   |  370 ms  |
|  c   |  380 ms  |
|  d   |  390 ms  |
|  e   |  400 ms  |
|  f   |  410 ms  |
|  g   |  420 ms  |
|  h   |  430 ms  |
|  i   |  440 ms  |
|  j   |  450 ms  |
|  k   |  460 ms  |
|  l   |  470 ms  |
|  m   |  480 ms  |
|  n   |  490 ms  |
|  o   |  500 ms  |
|  p   |  510 ms  |
|  q   |  520 ms  |
|  r   |  530 ms  |
|  s   |  540 ms  |
|  t   |  550 ms  |
|  u   |  560 ms  |
|  v   |  570 ms  |
|  w   |  580 ms  |
|  x   |  590 ms  |
|  y   |  600 ms  |
|  z   |  610 ms  |
