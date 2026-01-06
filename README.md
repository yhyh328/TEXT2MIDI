### Supports monophonic melodies only; chords and polyphonic structures are not supported.


## build
```shell
make
```

## write text file
```text
[Example 1: minimal melody]

# tempo in BPM
tempo 120

# simple melody
C4 200
E4 200
G4 400
rest 200
D4 400 80


[Example 2: advanced timing control]

# higher timing resolution
ppq 960
tempo 90
channel 1

C4 150
E4 150
G4 300
rest 75
G4 150

[Example 3: Distortion Guitar Riff]

# tempo and resolution
tempo 120
ppq 480

# part / voice
channel 0
program 25   # Distortion Guitar (GM)

# melody (F key)
F4 200
A4 200
Bb4 200
C5 400
rest 200
Bb4 200 90
F4 400


[Example 4: Accidentals (sharp and flat)]

# slow tempo for pitch clarity
tempo 72
ppq 480
channel 0
program 40    # Violin (GM)

# phrase 1 (natural + sharp)
C4   600
D4   600
F#4  600
G4   600

# phrase 2 (flat color)
Bb4  600
A4   600
Ab4  600
G4   600

# phrase 3 (mixed accidentals)
F#4  400
G4   400
Bb4  400
A4   400

# short ornamental turn
G4   200
Ab4  200
G4   200
F#4  200

# cadence
E4   800
C4   800


```

## run
- name your midi file as your sample text file 
    ```shell
    text2midi.exe [sample text file]
    ```
- to change your midi file's name
    ```shell
    text2midi.exe [sample text file] [midi file name]

