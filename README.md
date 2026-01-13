### Supports monophonic melodies only; chords and polyphonic structures are not supported.


## build
```shell
make
```

## write text file
```text
[Example 1]

// tempo in BPM
tempo 120

// simple melody
C4 8       // a eighth note with default velocity 100
E4 8       
G4 4       // a quarter note with default velocity 100
rest 8     // a eighth rest
D4 16 60   // a eighth note with velocity 60
C4 16 60
B3 16 60
B4 16 60

A4 8      // a eighth note with default velocity 100
C5 8 
E4 8
A4 .4     // a dotted quarter note with default velocity 100
rest 4


[Example 2]

// higher timing resolution
ppq 960
tempo 90
channel 1
program 11 // Vibraphone (GM)

E3 8
F#3 8
A3 4
rest 8
G#3 8
B3 8

C#4 16
B3 8
E3 16
E4 2
rest 4


[Example 3]

// tempo and resolution
tempo 69
ppq 480

channel 0
program 30   // Distortion Guitar (GM)

G#3 ,8   // triplet
C#4 ,8
E4 ,8
G#3 ,8
C#4 ,8
E4 ,8
G#3 ,8
C#4 ,8
E4 ,8
G#3 ,8
C#4 ,8
E4 ,8

G#3 ,8
C#4 ,8
E4 ,8
G#3 ,8
C#4 ,8
E4 ,8
G#3 ,8
C#4 ,8
E4 ,8
G#3 ,8
C#4 ,8
E4 ,8

A3 ,8
C#4 ,8
E4 ,8
A3 ,8
C#4 ,8
E4 ,8
A3 ,8
D4 ,8
F#4 ,8
A3 ,8
D4 ,8
F#4 ,8

G#3 ,8
C4 ,8
F#4 ,8
G#3 ,8
C#4 ,8
E4 ,8
G#3 ,8
C#4 ,8
D#4 ,8
F#3 ,8
C4 ,8
D#4 ,8

E3 ,8
G#4 ,8
C#4 ,8
G#3 ,8
C#4 ,8
E4 ,8
G#3 ,8
C#4 ,8
E4 ,8
G#4 .8
G#4 16



[Example 4]

// slow tempo for pitch clarity
tempo 100
ppq 480
channel 0
program 40    // Violin (GM)

G3  16 80
A3  16 90

B3  .4 100
F#4  8 
B4   8 
A4   4
D5   8

B4   8
F#4  2
E4   8
A4   8
G#4  8

F#4  8
D#4  4
A3   8
C#4 .4 
E4   8

D4  16
C#4 16
B3  .4
B3   4
C#4 ,16
D4  ,16
E4  ,16
E#4 ,16

F#4 .4
B4   8
F#5  8
C#5  4
A5   8

G#5  8
D#5  2
E5   8
C5   8
A4   8

G#4  8
C#5  4
B#4  8
C#5  32
D5   32
C#5  32
D5   32
C#5  32
D5   32
C#5  32
D5   32
C#5  32
D5   32
C#5  32
D5   32 
D#5   8

A4   16
G#4  16
F#   .4
F#    4
rest  4



## run
- name your midi file as your sample text file 
    ```shell
    text2midi.exe [sample text file]
    ```
- to change your midi file's name
    ```shell
    text2midi.exe [sample text file] [midi file name]

