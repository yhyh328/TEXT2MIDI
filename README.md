### build
```shell
make
```

### write text file
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
```

### run
- name your midi file as your sample text file 
    ```shell
    text2midi.exe [sample text file]
    ```
- to change your midi file's name
    ```shell
    text2midi.exe [sample text file] [midi file name]

