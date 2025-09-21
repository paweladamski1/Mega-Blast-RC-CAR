#include "AudioClip.h"
#include "AudioClipController.h"

AudioClip::AudioClip(AudioClipController *controller,
                     const String &fileName,
                     float volume,
                     bool repeat)
    : _controller(controller),
      _id(fileName),
      _volume(volume),
      _isChange(false),
      _fileName(fileName)

{
    Repeat = repeat;
    _isPlaying = false;
    serialPrint(" AudioClip::AudioClip constructor ");

    if (repeat)
        _load();
}

void AudioClip::play()
{
    if (!_isPlaying)
    {
        if (!_load())
        {
            serialPrint(" can't load file");
            return;
        }

        if (_controller)
            _controller->addClip(this);

        _isPlaying = true;
        _progressPercent = 0;
        _isChange = true;
        serialPrint(" play()");
    }
}

bool AudioClip::read()
{
    if (!_wavFile)
        return false;

    if (!_isPlaying)
        return false;

    if (_totalBytesRead + NUM_BYTES_TO_READ_FROM_FILE > _wavDataSize) // If next read will go past the end then adjust the
        _lastNumBytesRead = _wavDataSize - _totalBytesRead;           // amount to read to whatever is remaining to read
    else
        _lastNumBytesRead = NUM_BYTES_TO_READ_FROM_FILE; // Default to max to read

    _wavFile.read(_samplesArr, _lastNumBytesRead); // Read in the bytes from the file
    _totalBytesRead += _lastNumBytesRead;          // Update the total bytes red in so far

    _onEndCall();

    if (_totalBytesRead >= _wavDataSize) // Have we read in all the data?
    {
        if (Repeat)
        {
            serialPrint("Repeat!");
            _wavFile.seek(44);   // Reset to start of wav data
            _totalBytesRead = 0; // Clear to no bytes read in so far
        }
        else
        {
            stop();
        }
    }
    if (_lastNumBytesRead == 0)
        return false;

    if (!_samplesArr)
    {
        Serial.println("ERROR: Samples is null in _read()");
        return false;
    }
    return _isPlaying;
}

void AudioClip::stop()
{
    if (_isPlaying)
    {
        serialPrint(" stop()");
        _isPlaying = false;
        _isChange = true;
        if (!Repeat)
        {
            serialPrint("close file ");
            _wavFile.close();
            _controller->removeClip(this);
        }
    }
}

bool AudioClip::isPlaying() const
{
    if (_wavFile)
        return _isPlaying;
    else
        return false;
}

float AudioClip::getPlayingProgress() const
{
    return _progressPercent;
}

void AudioClip::resetIdx()
{
    _Idx = 0;
}

uint16_t AudioClip::getIdx() const
{
    return _Idx;
}

bool AudioClip::isBufferNotEmpty() const
{
    return _Idx < _lastNumBytesRead;
}

bool AudioClip::isReadyToMix() const
{
    return _isPlaying && _samplesArr != nullptr && isBufferNotEmpty();
}

int16_t AudioClip::getNextSample()
{    
    int16_t sample = *((int16_t *)(_samplesArr + _Idx));
    _Idx += 2;
    return static_cast<int16_t>(sample * _volume);
}

uint16_t AudioClip::getBytesInBuffer() const
{
    return _lastNumBytesRead;
}

//--------------------------------------------------------------------------------------------------------------
// PRIVATE
bool AudioClip::_load()
{
    serialPrint("_load()");

    if (_wavFile)
    {
        serialPrint("Could not open");
        Serial.print(" - file already open");
        return true;
    }
    // Load wav file, if all goes ok returns true else false
    WavHeader_Struct WavHeader;

    _wavFile = SD.open(_fileName); // Open the wav file

    if (!_wavFile)
    {
        Serial.print("Could not open :");
        Serial.println(_fileName);
        return false;
    }

    _wavFile.read((byte *)&WavHeader, 44); // Read in the WAV header, which is first 44 bytes of the file.
                                           // We have to typecast to bytes for the "read" function
    if (_validWavData(&WavHeader))
    {
        _dumpWAVHeader(&WavHeader); // Dump the header data to serial, optional!
        Serial.println();
        _wavDataSize = WavHeader.DataSize; // Copy the data size into our wav structure
        return true;
    }
    serialPrint(".. fail");
    return false;
}

void AudioClip::_onEndCall()
{
    _progressPercent = (float)_totalBytesRead / (float)_wavDataSize * 100.0f;

    if (_progressPercent > 97.0f)
    {
        if (_isChange)
        {
            Serial.print(_progressPercent);
            Serial.print("% ");
            _isChange = false;
            if (onEnd)
                onEnd(this, _controller);
        }
    }
}

bool AudioClip::_validWavData(WavHeader_Struct *Wav)
{
    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        Serial.print("Invalid data - Not RIFF format");
        return false;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        Serial.print("Invalid data - Not Wave file");
        return false;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        Serial.print("Invalid data - No format section found");
        return false;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        Serial.print("Invalid data - data section not found");
        return false;
    }
    if (Wav->FormatID != 1)
    {
        Serial.print("Invalid data - format Id must be 1");
        return false;
    }
    if (Wav->FormatSize != 16)
    {
        Serial.print("Invalid data - format section size must be 16.");
        return false;
    }
    if ((Wav->NumChannels != 1) & (Wav->NumChannels != 2))
    {
        Serial.print("Invalid data - only mono or stereo permitted.");
        return false;
    }
    if (Wav->SampleRate > 48000)
    {
        Serial.print("Invalid data - Sample rate cannot be greater than 48000");
        return false;
    }
    if ((Wav->BitsPerSample != 8) & (Wav->BitsPerSample != 16))
    {
        Serial.print("Invalid data - Only 8 or 16 bits per sample permitted.");
        return false;
    }
    return true;
}

void AudioClip::_dumpWAVHeader(WavHeader_Struct *Wav)
{
    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        Serial.print("Not a RIFF format file - ");
        serialPrint_fileHeader(Wav->RIFFSectionID, 4);
        return;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        Serial.print("Not a WAVE file - ");
        serialPrint_fileHeader(Wav->RiffFormat, 4);
        return;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        Serial.print("fmt ID not present - ");
        serialPrint_fileHeader(Wav->FormatSectionID, 3);
        return;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        Serial.print("data ID not present - ");
        serialPrint_fileHeader(Wav->DataSectionID, 4);
        return;
    }
    // All looks good, dump the data
    Serial.print("Total size :");
    Serial.println(Wav->Size);
    Serial.print("Format section size :");
    Serial.println(Wav->FormatSize);
    Serial.print("Wave format :");
    Serial.println(Wav->FormatID);
    Serial.print("Channels :");
    Serial.println(Wav->NumChannels);
    Serial.print("Sample Rate :");
    Serial.println(Wav->SampleRate);
    Serial.print("Byte Rate :");
    Serial.println(Wav->ByteRate);
    Serial.print("Block Align :");
    Serial.println(Wav->BlockAlign);
    Serial.print("Bits Per Sample :");
    Serial.println(Wav->BitsPerSample);
    Serial.print("data Size :");
    Serial.println(Wav->DataSize);
}

void AudioClip::serialPrint(const char *data)
{
    Serial.println();
    Serial.print(_id);
    Serial.print(" ");
    Serial.print(data);
    Serial.print(" ");
}

void AudioClip::serialPrint(const char *data, const uint32_t n)
{
    Serial.println();
    Serial.print(data);
    Serial.print(" ");
    Serial.print(_id);
    Serial.print(" ");
    Serial.print(n);
    Serial.print(" ");
}

void AudioClip::serialPrint_fileHeader(const char *data, uint8_t NumBytes)
{
    for (uint8_t i = 0; i < NumBytes; i++)
        Serial.print(data[i]);
    Serial.println();
}

AudioClip::~AudioClip()
{
    stop();
}