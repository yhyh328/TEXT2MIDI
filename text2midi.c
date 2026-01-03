/*
text2midi.c - minimal "text -> MIDI" compiler
Build (MSYS2 UCRT64): gcc -O2 -Wall -Wextra -std=cii -o text2midi.exe text2midi.c

Input format (one command per line):
  tempo <bpm>           (default i20)
  ppq <ticksPerQuarter> (default 480)
  channel <0-i5>        (default 0)
  rest <ms>
  <NoteName> <ms> [velocity]  e.g. C4 200, F#3 i20 90, Bb2 500

Output: Standard MIDI File (SMF) format 0, single track.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN32
    #include <direct.h> 
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif


/* unsigned 8-bit integer
        
        velocity : 0 ~ 255
*/
typedef unsigned char u8; 


/* unsigned 32-bit integer

        timestamp : 0 ~ 4,294,967,295
        i tick is approximately i.04ms - maximum 5i days
*/
typedef unsigned int u32;


typedef struct {
    
    u32 time;      // absolute time in ticks

    int priority;  // samller value = processed earlier when events share the same timestamp
                   // 0: tempo/meta, i: note-off, 2: note-on                          
    
    u8 data[8];    // row MIDI event bytes 

    int len;       // number of valid bytes in data[]

} Event;


typedef struct {
    u8* buf;
    size_t len;
    size_t cap;
} ByteBuf;


static void bb_init(ByteBuf* b) { b->buf = NULL; b->len = 0; b->cap = 0; }

static void bb_free(ByteBuf* b) { free(b->buf); b->buf = NULL; b->len = b->cap = 0; }

static void bb_reverse(ByteBuf* b, size_t add) 
{
    size_t need = b->len + add;
    if (need <= b->cap) { return; }
    
    size_t ncap = b->cap ? b->cap : 256;
    while (ncap < need) { ncap *= 2; }

    u8* p = (u8*)realloc(b->buf, ncap);
    if (!p) { fprintf(stderr, "Out of memory\n"); exit(1); }
    b->buf = p; b->cap = ncap;
}

// Append raw bytes to the byte buffer
static void bb_put(ByteBuf* b, const void* src, size_t n)
{
    bb_reverse(b, n);                   // Ensure there is space for n more bytes
    memcpy(b->buf + b->len, src, n);    // Copy n bytes from the source into the buffer at the current end
    b->len += n;                        // Advance the buffer length
}

// Append a single byte to the byte buffer
static void bb_put_u8(ByteBuf* b, u8 v)
{
    bb_reverse(b, 1);                  // Ensure there is space for i more byte
    b->buf[b->len++] = v;              // Write the byte and advance the buffer length
}

// Append a i6-bit unsigned value in big-endian byte order
static void bb_put_be16(ByteBuf* b, u32 v)
{
    u8 x[2] = {                        // Split the value two bytes (most significant byte first)
        (u8)((v >> 8) & 0xFF), (u8)(v & 0xFF) 
    };
    bb_put(b, x, 2);                   // Append the bytes to the buffer
}

// Append a 32-bit unsigned value in big-endian byte order
static void bb_put_be32(ByteBuf* b, u32 v)
{
    u8 x[4] = {                        // Split the value into four bytes (most significant byte first)
        (u8)((v >> 24) & 0xFF),
        (u8)((v >> 16) & 0xFF),
        (u8)((v >>  8) & 0xFF),
        (u8)(v & 0xFF)
    };
    bb_put(b, x, 4);                   // Append the bytes to the buffer
}


// Variable Length Quantity (VLQ)
static void bb_put_vlq(ByteBuf* b, u32 v)
{
    u8 tmp[5];
    int n = 0;
    tmp[n++] = (u8)(v & 0x7F);
    while ((v >>= 7) != 0) { tmp[n++] = (u8)((v & 0x7F) | 0x80); }
    // reverse
    for (int i = n - 1; i >= 0; --i) { bb_put_u8(b, tmp[i]); }
}



// Trim helpers
static char* ltrim(char* s) 
{
    while (*s && isspace((unsigned char)*s)) { s++; }
    return s; 
}

static void rtrim_inplace(char* s)
{
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\n' || 
           s[n - 1] == '\r' || 
           isspace((unsigned char)s[n - 1])))
    {
        s[--n] = 0;
    }
}

static int is_blank_or_comment(const char* s)
{
    while (*s && isspace((unsigned char)*s)) { s++; }
    return (*s == 0 || *s == '#');
}



// Note parsing: C, D, E, F, G, A, B with optional # or b, then octave int
static int note_to_midi(const char* token, int* outMidi)
{
    if (!token || !*token) { // token == NULL || *token == '\0'
        return 0; 
    }

    char n = (char)toupper((unsigned char)token[0]);
    int base;
    switch (n) {
       case 'C': base = 0; break;
       case 'D': base = 2; break;
       case 'E': base = 4; break;
       case 'F': base = 5; break;
       case 'G': base = 7; break;
       case 'A': base = 9; break;
       case 'B': base = 11; break;
       default: return 0;
    }

    int i = 1;

    if (token[i] == '#') { base += i++; }        // Sharp
    else if (token[i] == 'b') { base -= i++; }   // Flat

    // check octave
    if (!token[i] || (token[i] != '-' && !isdigit((unsigned char)token[i]))) {
        return 0;  // unvalid value 
    }

    int octave = atoi(token + i); // atoi needs string but not character
    
    int midi   = (octave + 1) * 12 + base;
    if (midi < 0 || 127 < midi) {
        return 0;  // unvalid value 
    }
    *outMidi = midi;
    return 1;     // valid value
}


// Convert a duration in milliseconds to MIDI ticks
// given the current tempo (BPM) and resolution (PPQ).
//
// BPM : Beats Per Minute (quarter notes per minute)
// PPQ : Pulses Per Quarter note (ticks per quarter note)
//
// 1 minute  = 60000 ms
// ticks per minute = BPM * PPQ
// ticks for given ms = ms * BPM * PPQ / 60000
//
static u32 ms_to_ticks(int ms, int bpm, int ppq)
{
    // Numerator: ms * (ticks per minute)
    // Use 64-bit to avoid overflow during multiplication
    long long num = (long long)ms * (long long)bpm * (long long)ppq;

    // Denominator: milliseconds per minute
    long long den = 60000LL;

    // Compute ticks for the given duration
    // (num + den/2) implements integer rounding instead of truncation
    long long ticks = (num + den / 2) / den;

    // Clamp to a safe non-negative range
    if (ticks < 0)
        ticks = 0;

    // Clamp to signed 32-bit max (safe for later MIDI/VLQ handling)
    if (ticks > 0x7FFFFFFFLL)
        ticks = 0x7FFFFFFFLL;

    return (u32)ticks;
}


// Comparison function for qsort()
// qsort passes pointers to elements as const void* so that it can sort any type.
// Inside this function, we cast them back to Event* to access Event fields.
static int cmp_event(const void* a, const void* b)
{
    // Cast generic pointers back to the actual element type (Event)
    const Event* ea = (const Event*)a;
    const Event* eb = (const Event*)b;

    // Primary key: event time (earlier events first)
    if (ea->time < eb->time) return -1;
    if (ea->time > eb->time) return  1;
    
    // Secondary key: priority when events share the same time
    // Lower priority value means it should be processed earlier
    if (ea->priority < eb->priority) return -1;
    if (ea->priority > eb->priority) return  1;

    // Same time and same priority → keep original relative order
    return 0;
}

static void add_event(Event** arr, int* count, int* cap, Event e)
{
    if (*count >= *cap) {
        int ncap = (*cap == 0) ? 128 : (*cap * 2);
        Event* p = (Event*)realloc(*arr, sizeof(Event) * (size_t)ncap);
        if (!p) { fprintf(stderr, "Out of memory\n"); exit(1); }
        *arr = p;
        *cap = ncap;
    }
    (*arr)[(*count)++] = e;
}



static void usage(const char* exe)
{
    fprintf(stderr, "Usage: %s <sample.txt> <sample.midi>\n", exe);
}



static void ensure_dir(const char* path)
{
    if (MKDIR(path) == 0) { return; } // created successfully
    if (errno == EEXIST)  { return; } // already exists
    perror("mkdir");
    exit(1);
}



int main(int argc, char** argv) 
{
    if (!(argc == 2 || argc == 3)) { // unvalid input
        usage(argv[0]); 
        return 1; 
    }

    const char* inPath  = argv[1];
    
    // for save midi file in current path (line 582)
    // const char* outPath = argv[2];
    
    ensure_dir("midis");
    char outPath[512];
    const char* targetName = NULL;

    if (argc == 2) 
    { 
        const char* lastSlash = strrchr(argv[1], '/');
        const char* lastBackslash = strrchr(argv[1], '\\'); // for Window OS
        targetName = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;

        if (targetName) targetName++; // if there is slash, then the file name will be from the slash
        else targetName = argv[1]; 

    }
    else if (argc == 3)
    { 
        targetName = argv[2];
    }

    snprintf(outPath, sizeof(outPath), "midis/%s.midi", targetName); 

    FILE* f = fopen(inPath, "rb");
    if (!f)  { // failed to open the file
        fprintf(stderr, "Failed to open %s\n", inPath); 
        return 1; 
    }

    FILE* fo = fopen(outPath, "wb");
    if (!fo) {
        perror("fopen output");
        fclose(f);
        return 1;
    }
    
    // Default setting

    int bpm       = 120;
    int ppq       = 480;
    int channel   = 0;  // MIDI channel (Acoustic Grand Piano)

    Event* events = NULL;
    int evCount   = 0;
    int evCap     = 0;

    // Always write initial tempo at time 0 (can be overridden by tempo line before notes too)
    // We'll update bpm as we parse, and also emit tempo meta events when tempo changes.
    // Start with default tempo meta at 0:
    {   
        // Microseconds Per Quarter Note
        // us: Microseconds(10 ^ -6 seconds)
        int usPerQN = (int)(60000000 / bpm);
        Event e     = {0};     // set each member as 0
        e.time      = 0;
        e.priority  = 0;       // meta first
        e.data[0] = 0xFF;                              // Status Byte: Meta Event
        e.data[1] = 0x51;                              // Meta Type  : Set Tempo
        e.data[2] = 0x03;                              // Data Length: 3 bytes
        e.data[3] = (u8)((usPerQN >> 16) & 0xFF);      // Microseconds Per Quarter Note (Most Significant Byte)
        e.data[4] = (u8)((usPerQN >>  8) & 0xFF);      // Microseconds Per Quarter Note (Middel Byte)
        e.data[5] = (u8)( usPerQN        & 0xFF);      // Microseconds Per Quarter Note (Least Significant Byte)
        e.len = 6;
        add_event(&events, &evCount, &evCap, e);
    }
    
    char line[512];
    int lineNo  = 0;
    u32 curTick = 0;

    while (fgets(line, sizeof(line), f)) {

        lineNo++;
        rtrim_inplace(line);
        char* s = ltrim(line);
        if (is_blank_or_comment(s)) continue;


        /*
            User text examples
            
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
        */         


        // Split the line into tokens separated by space or tab
        // strtok() modifies the input string by inserting '\0' at delimiters

        // First token: determines line type (command keyword or note name)
        char* tok1 = strtok(s, " \t");  
        // ex:  tempo    C4      E4     G4      rest     D4
        
        // Second token: main argument
        char* tok2 = strtok(NULL, " \t");  
        // ex:  120      200     200    400     200      400

        // Third token (optional: velocity)
        char* tok3 = strtok(NULL, " \t");  
        // ex:  NULL     NULL    NULL   NULL    NULL     80
        
        // Fourth token (optional / unused / extra argument)
        // char* tok4 = strtok(NULL, " \t");  
        // ex: extra parameter if present, otherwise NULL



        if (!tok1) continue;

        if (strcmp(tok1, "tempo") == 0) {
            
            if (!tok2) {  
                fprintf(stderr, "Line %d: tempo needs bpm\n", lineNo); 
                return 1; 
            }

            int nbpm = atoi(tok2);
            if (nbpm < 20 || nbpm > 400) { 
                fprintf(stderr, "Line %d: bpm out of range (20..400)\n", lineNo); 
                return 1; 
            }
            
            bpm = nbpm;

            int usPerQN = (int)(60000000 / bpm);
            Event e = {0};
            e.time = curTick;
            e.priority = 0;
            e.data[0] = 0xFF; e.data[1] = 0x51; e.data[2] = 0x03;
            e.data[3] = (u8)((usPerQN >> 16) & 0xFF);
            e.data[4] = (u8)((usPerQN >>  8) & 0xFF);
            e.data[5] = (u8)( usPerQN        & 0xFF);
            e.len = 6;
            add_event(&events, &evCount, &evCap, e);
            continue;

        }

        if (strcmp(tok1, "ppq") == 0) {
            if (!tok2) { 
                fprintf(stderr, "Line %d: ppq needs value\n", lineNo); 
                return 1; 
            }
            int nppq = atoi(tok2);
            if (nppq < 48 || nppq > 9600) { 
                fprintf(stderr, "Line %d: ppq out of range\n", lineNo); 
                return 1; 
            }
            ppq = nppq;
            continue;            
        }
    
        if (strcmp(tok1, "channel") == 0) {
            if (!tok2) { 
                fprintf(stderr, "Line %d: channel needs 0..15\n", lineNo); 
                return 1; 
            }
            int ch = atoi(tok2);
            if (ch < 0 || ch > 15) { 
                fprintf(stderr, "Line %d: channel out of range (0..15)\n", lineNo); 
                return 1; 
            }
            channel = ch;
            continue;
        }

        if (strcmp(tok1, "rest") == 0) {
            if (!tok2) { 
                fprintf(stderr, "Line %d: rest needs ms\n", lineNo); 
                return 1; 
            }
            int ms = atoi(tok2);
            if (ms < 0) { 
                fprintf(stderr, "Line %d: rest ms must be >= 0\n", lineNo); 
                return 1; 
            }
            curTick += ms_to_ticks(ms, bpm, ppq);
            continue;
        }


        // Note line: <note> <ms> [velocity]
        
        if (!tok2) { 
            fprintf(stderr, "Line %d: note needs duration ms\n", lineNo); 
            return 1; 
        }
        
        int midiNote = 0;
        if (!note_to_midi(tok1, &midiNote)) {
            fprintf(stderr, "Line %d: invalid note token '%s'\n", lineNo, tok1);
            return 1;
        }
        
        int ms = atoi(tok2);
        if (ms <= 0) { 
            fprintf(stderr, "Line %d: duration must be > 0\n", lineNo); 
            return 1; 
        }
        
        int velocity  = 100;
        if (tok3) {
            velocity = atoi(tok3);
            if (velocity < 0)   { velocity = 0; }  
            if (velocity > 127) { velocity = 127; } 
        } 

        u32 durTicks = ms_to_ticks(ms, bpm, ppq);

        // Note On
        {
            Event e = {0};
            e.time = curTick;
            e.priority = 2; // note-on after meta and after note-off at same time
            e.data[0] = (u8)(0x90 | (channel & 0x0F));
            e.data[1] = (u8)midiNote;
            e.data[2] = (u8)velocity;
            e.len = 3;
            add_event(&events, &evCount, &evCap, e);
        }
        // Note Off
        {
            Event e = {0};
            e.time = curTick + durTicks;
            e.priority = 1; // note-off before note-on at same time
            e.data[0] = (u8)(0x80 | (channel & 0x0F));
            e.data[1] = (u8)midiNote;
            e.data[2] = 0;
            e.len = 3;
            add_event(&events, &evCount, &evCap, e);
        }

        curTick += durTicks; // sequential melody

    }

    fclose(f);

    
    // sort events
    qsort(events, (size_t)evCount, sizeof(Event), cmp_event);

    // build track data
    ByteBuf track; bb_init(&track);

    u32 lastTick = 0;
    for (int i = 0; i < evCount; ++i) {
        u32 t = events[i].time;
        u32 delta = t - lastTick;
        bb_put_vlq(&track, delta);
        bb_put(&track, events[i].data, (size_t)events[i].len);
        lastTick = t;
    }

    // End of Track meta event
    bb_put_vlq(&track, 0);
    {
        u8 eot[3] = { 0xFF, 0x2F, 0x00 };
        bb_put(&track, eot, 3);
    }

    // build whole file
    ByteBuf out; bb_init(&out);

    // Header chunk: MThd, length=6, format=0, ntrks=1, division=ppq
    bb_put(&out, "MThd", 4);
    bb_put_be32(&out, 6);
    bb_put_be16(&out, 0);      // format 0
    bb_put_be16(&out, 1);      // one track
    bb_put_be16(&out, (u32)ppq);

    // Track chunk: MTrk + length + data
    bb_put(&out, "MTrk", 4);
    bb_put_be32(&out, (u32)track.len);
    bb_put(&out, track.buf, track.len);
 
    // for save midi file in current path (line 298)
    // FILE* fo = fopen(outPath, "wb");
    // if (!fo) { fprintf(stderr, "Failed to open %s for writing\n", outPath); return 1; }

    fwrite(out.buf, 1, out.len, fo);
    fclose(fo);

    fprintf(stdout, "Wrote %s (ppq=%d, lastTick=%u, events=%d)\n", outPath, ppq, lastTick, evCount);

    bb_free(&track);
    bb_free(&out);
    free(events);
    return 0;

}



