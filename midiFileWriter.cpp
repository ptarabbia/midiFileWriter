//
// A C++ class for writing midi files
// initial C code from https://github.com/mru00/dsp
//
#include "midiFileWriter.h"

static inline unsigned short reorder_word(unsigned short in) {
  return ((in>>8)&0xff) | ((in&0xff) << 8);
}

static inline unsigned int reorder_dword(unsigned int in) {
  return (reorder_word((in >> 16)&0xffff)) | (reorder_word(in&0xffff)<<16);
}

MidiFileWriter::MidiFileWriter(const char* filename, short timeDivision): _difftime(0), _runningStatus(0xff) {
	_handle = fopen (filename, "wb+");
	if (canWrite())
	{
		writeHeader(timeDivision, 1); // division = 120
		writeTrackHeader();
	}
}


MidiFileWriter::~MidiFileWriter()
{
  if (_handle) 
  {
	  writeTrackEnd();
	  fclose(_handle);
  }
}


void MidiFileWriter::increaseDiffTime(unsigned long delta)
{
	_difftime += delta;
}

void MidiFileWriter::writeTimestamp()
{
	writeVarLength(_difftime);
	_difftime = 0;
}

//http://www.omega-art.com/midi/mfiles.html
void MidiFileWriter::writeVarLength(unsigned long number) 
{
  unsigned char b;
  unsigned long buffer;
  buffer = number & 0x7f;
  while ((number >>= 7) > 0) {
	buffer <<= 8;
	buffer |= 0x80 + (number & 0x7f);
  }
  while (1) {
	b = buffer & 0xff;
	fwrite(&b, 1, 1, _handle);
	if (buffer & 0x80) buffer >>= 8;
	else break;
  }
}


void MidiFileWriter::addCopyright(const char* copyright)
{
	writeTimestamp();

	unsigned char b;
	b = 0xFF; fwrite(&b, 1, 1, _handle);
	b = 0x2; fwrite(&b, 1, 1, _handle);
	b = strlen(copyright); fwrite(&b, 1, 1, _handle);
	fwrite(copyright, b, 1, _handle);	
}

void MidiFileWriter::addTempo(unsigned long bpm)
{
	writeTimestamp();

	unsigned char b;
	b = 0xFF; fwrite(&b, 1, 1, _handle);
	b = 0x51; fwrite(&b, 1, 1, _handle);
	b = 0x03; fwrite(&b, 1, 1, _handle);
	bpm = 60000000 / bpm;
	b = bpm >> 16; fwrite(&b, 1, 1, _handle);
	b = (bpm & 0xFFFFF) >> 8; fwrite(&b, 1, 1, _handle);
	b = (bpm & 0xFF); fwrite(&b, 1, 1, _handle);
}

void MidiFileWriter::addTimeSignature(unsigned char numerator, unsigned char denominator, unsigned char numclocks, unsigned char num32)
{
	writeTimestamp();

	unsigned char b;
	b = 0xFF; fwrite(&b, 1, 1, _handle);
	b = 0x58; fwrite(&b, 1, 1, _handle);
	b = 0x04; fwrite(&b, 1, 1, _handle);
	fwrite(&numerator, 1, 1, _handle);
	fwrite(&denominator, 1, 1, _handle);
	fwrite(&numclocks, 1, 1, _handle);
	fwrite(&num32, 1, 1, _handle);
}

void MidiFileWriter::addTrackName(const char* name)
{
	writeTimestamp();

	unsigned char b;
	b = 0xFF; fwrite(&b, 1, 1, _handle);
	b = 0x3; fwrite(&b, 1, 1, _handle);
	b = (unsigned char)strlen(name); fwrite(&b, 1, 1, _handle);
	fwrite(name, b, 1, _handle);
}


void MidiFileWriter::writeNoteEvent(unsigned char eventType, unsigned char note, unsigned char velocity) 
{
	writeTimestamp();

	// running status
	if ( eventType != _runningStatus ) {
		fwrite(&eventType, 1, 1, _handle);
		_runningStatus = eventType;
	}

	fwrite(&note, 1, 1, _handle);
	fwrite(&velocity, 1, 1, _handle);  
}


void MidiFileWriter::writeHeader(short timedivision, short numberoftracks) 
{
	  int chunksize = reorder_dword(6);
	  short formattype = reorder_word(0);

	  numberoftracks  = reorder_word(numberoftracks);
	  timedivision= reorder_word(timedivision);

	  fwrite("MThd", 4, 1, _handle);
	  fwrite(&chunksize, 4, 1, _handle);
	  fwrite(&formattype, 2, 1, _handle);
	  fwrite(&numberoftracks, 2, 1, _handle);
	  fwrite(&timedivision, 2, 1, _handle);
}



void MidiFileWriter::writeTrackHeader() 
{
  int chunksize = reorder_dword(60000);
  fwrite("MTrk", 4, 1, _handle);
  _trackChunksizeFixup = ftell(_handle);
  fwrite(&chunksize, 4, 1, _handle);
}


void MidiFileWriter::writeTrackEnd() 
{
	unsigned char b;

	writeTimestamp();

	b = 0xFF; fwrite(&b, 1, 1, _handle);
	b = 0x2F; fwrite(&b, 1, 1, _handle);

	writeVarLength(0);

	long pos = ftell(_handle);
	int chunksize = reorder_dword(pos - _trackChunksizeFixup - sizeof(unsigned int));

	fseek(_handle, _trackChunksizeFixup, SEEK_SET);
	fwrite(&chunksize, 4, 1,  _handle);
	fseek(_handle, pos, SEEK_SET);
}
