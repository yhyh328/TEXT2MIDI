### Supports monophonic melodies only; chords and polyphonic structures are not supported.
### This project currently targets MIDI 1.0

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

// part / voice
channel 0
program 30   # Distortion Guitar (GM)



G#3 ,3,8   // triplet
C#4 ,3,8
E4 ,3,8
G#3 ,3,8
C#4 ,3,8
E4 ,3,8
G#3 ,3,8
C#4 ,3,8
E4 ,3,8
G#3 ,3,8
C#4 ,3,8
E4 ,3,8

G#3 ,3,8
C#4 ,3,8
E4 ,3,8
G#3 ,3,8
C#4 ,3,8
E4 ,3,8
G#3 ,3,8
C#4 ,3,8
E4 ,3,8
G#3 ,3,8
C#4 ,3,8
E4 ,3,8

A3 ,3,8
C#4 ,3,8
E4 ,3,8
A3 ,3,8
C#4 ,3,8
E4 ,3,8
A3 ,3,8
D4 ,3,8
F#4 ,3,8
A3 ,3,8
D4 ,3,8
F#4 ,3,8

G#3 ,3,8


[Example 4]

// This sample is stylistically inspired by Violin Concerto No. 2 by Béla Bartók.
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
C#4 ,3,16
D4  ,3,16
E4  ,3,16
E#4 ,3,16

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
F#4   .4  // need update code to tie here

// This design choice ensures that the entire tuplet spans exactly two quarter notes (PPQ × 2),
// allowing the passage to align cleanly with the internal timing grid used by the MIDI generator.
F#4    ,13,4     // need update code to tie here
G#4    ,13,4
A4     ,13,4
B4     ,13,4
C#5    ,13,4
D#5    ,13,4
E#5    ,13,4
F#5    ,13,4
G#5    ,13,4
A5     ,13,4
B5     ,13,4
C#6    ,13,4
D#6    ,13,4

E6   4
E6   16   // need update code to tie here
D#6  16
C#6  16
B#6  16

A6   16
C#6  16
E6   8  // need update code to tie here

E6   16
D6   16
C#6  16
A#5  16

B5   4 // need update code to tie here
B5   16 // need update code to tie here
G5   16
E5   16
C#5  16
E5   16
B4   .8 // need update code to tie here


// This design choice ensures that the entire tuplet spans exactly a quarter note (PPQ),
// allowing the passage to align cleanly with the internal timing grid used by the MIDI generator.
B4  ,9,4 // need update code to tie here
C#5 ,9,4
D#5 ,9,4
E5  ,9,4
F#5 ,9,4
G#5 ,9,4
A#5 ,9,4
B5  ,9,4
C#6 ,9,4

D6   4 // need update code to tie here
D6  16 // need update code to tie here
C#6 16
B5  16
A#5 16
B5  16
D6  16
F6  8 // need update code to tie here
F6  16 // need update code to tie here
D6  16
C6  16
Bb5 16

A5   4  // need update code to tie here
A5  16 // need update code to tie here
F5  16
C#5 16
B4  16
D5  16
A4  .8 // need update code to tie here


// This design choice ensures that the entire tuplet spans exactly a quarter note (PPQ),
// allowing the passage to align cleanly with the internal timing grid used by the MIDI generator.
A4  ,10,4 // need update code to tie here
B4  ,10,4
C#5  ,10,4
D5  ,10,4
E5  ,10,4
G5  ,10,4
A5  ,10,4
B5  ,10,4
D6  ,10,4
E6  ,10,4

G#6  .4
D#6   8
D6    8
A5    4
B5    8
C6    .2
rest  4

## run
- name your midi file as your sample text file 
    ```shell
    text2midi.exe [sample text file]
    ```
- to change your midi file's name
    ```shell
    text2midi.exe [sample text file] [midi file name]

