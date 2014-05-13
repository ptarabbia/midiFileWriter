#pragma once
//
// A C++ class for writing midi files
// initial C code from https://github.com/mru00/dsp
//
// Sample usage:
// MidiFileWriter writer("test.mid");
// writer.addTrackName("track 1");
// writer.addTimeSignature(4, 2, 0x24, 8);
// writer.addTempo(120);
// writer.writeNoteEvent(0x90, 36, 127); // note on
// writer.increaseDiffTime(10);
// writer.writeNoteEvent(0x80, 36, 0); // note off


class MidiFileWriter
{
public:
	MidiFileWriter(const char* filename, short timeDivision=96);
	~MidiFileWriter();
	bool canWrite() const { return (_handle!=NULL); }
	void increaseDiffTime(unsigned long delta);
	void writeNoteEvent(unsigned char eventType, unsigned char note, unsigned char velocity);
	void addCopyright(const char* copyright);
	void addTrackName(const char* name);
	void addTimeSignature(unsigned char numerator, unsigned char denominator, unsigned char numclocks, unsigned char num32);
	void addTempo(unsigned long bpm);
protected:
	void writeVarLength(unsigned long number);
	void writeTimestamp();
	void writeHeader(short time_division, short number_of_tracks); 
	void writeTrackHeader();
	void writeTrackEnd();
private:
  FILE* _handle;
  unsigned long _difftime;
  unsigned char _runningStatus;
  long _trackChunksizeFixup;
};
