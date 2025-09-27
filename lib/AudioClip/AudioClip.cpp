#include "AudioClip.h"
#include "AudioClipController.h"

AudioClip::AudioClip(AudioClipController *controller,
                     const String &fileName,
                     float volume,
                     bool repeat,
                     float callEndWhenPercent)
    : _controller(controller),
      _id(fileName),
      _volume(volume),
      _isCallOnEnd(true),
      _fileName(fileName),
      _callEndWhenPercent(callEndWhenPercent)

{
    Repeat = repeat;
    _isPlaying = false;
    serialPrint(" AudioClip::AudioClip constructor ");
    
    if (repeat)
        _load();
}

void AudioClip::play()
{
    if (_isPlaying)
        return;

    if (!_load())
    {
        serialPrint(" can't load file");
        return;
    }

    if (_controller)
        _controller->addClip(this);
    _wavFile.seek(44);
    _totalBytesRead = 0;
    _isPlaying = true;
    _progressPercent = 0;
    _isCallOnEnd = false;
    serialPrint(" play()");
}

void AudioClip::stop()
{
    if (!_isPlaying)
        return;

    serialPrint(" stop()");
    _isPlaying = false;
    _isCallOnEnd = false;
    _wavFile.close();
    _controller->removeClip(this);
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

    _progressPercent = (float)_totalBytesRead / (float)_wavDataSize * 100.0f;

    if (_progressPercent >= _callEndWhenPercent)
        _callOnEnd_event();

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
            _callOnEnd_event();
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
 

void AudioClip::increaseVolume()
{
    _volume += 0.1f;
    if (_volume > 2.0f)
        _volume = 2.0f;
}

void AudioClip::decreaseVolume()
{
    _volume -= 0.1f;
    if (_volume < 0.0f)
        _volume = 0.0f;
}

void AudioClip::setVolume(float newVolume)
{
    if (newVolume < 0.0f)
        _volume = 0.0f;
    else if (newVolume > 1.0f)
        _volume = 1.0f;
    else
        _volume = newVolume;
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
    _sampleIdx = 0.0f;
}

uint16_t AudioClip::getIdx() const
{
    return (uint16_t)_sampleIdx;
}

bool AudioClip::isBufferNotEmpty() const
{
    return getIdx() < _lastNumBytesRead;
}

bool AudioClip::isReadyToMix() const
{
    return _isPlaying && _samplesArr != nullptr && isBufferNotEmpty();
}

int16_t AudioClip::getNextSample()
{
    int16_t sample = *((int16_t *)(_samplesArr + _sampleIdx));
    _sampleIdx += 2;
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
        return true;

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

void AudioClip::_callOnEnd_event()
{
    if (!_isCallOnEnd && !Repeat)
    {

        _isCallOnEnd = true;
        if (onEnd)
        {
            serialPrint("onEnd");
            onEnd(this, _controller);
        }
        else
            serialPrint("no event");
    }
}

bool AudioClip::_validWavData(WavHeader_Struct *Wav)
{
    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        serialPrint("Invalid data - Not RIFF format");
        return false;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        serialPrint("Invalid data - Not Wave file");
        return false;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        serialPrint("Invalid data - No format section found");
        return false;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        serialPrint("Invalid data - data section not found");
        return false;
    }
    if (Wav->FormatID != 1)
    {
        serialPrint("Invalid data - format Id must be 1");
        return false;
    }
    if (Wav->FormatSize != 16)
    {
        serialPrint("Invalid data - format section size must be 16.");
        return false;
    }
    if ((Wav->NumChannels != 1) & (Wav->NumChannels != 2))
    {
        serialPrint("Invalid data - only mono or stereo permitted.");
        return false;
    }
    if (Wav->SampleRate > 48000)
    {
        serialPrint("Invalid data - Sample rate cannot be greater than 48000");
        return false;
    }
    if ((Wav->BitsPerSample != 8) & (Wav->BitsPerSample != 16))
    {
        serialPrint("Invalid data - Only 8 or 16 bits per sample permitted.");
        return false;
    }
    return true;
}

void AudioClip::_dumpWAVHeader(WavHeader_Struct *Wav)
{
    if (memcmp(Wav->RIFFSectionID, "RIFF", 4) != 0)
    {
        serialPrint("Not a RIFF format file - ");
        serialPrint_fileHeader(Wav->RIFFSectionID, 4);
        return;
    }
    if (memcmp(Wav->RiffFormat, "WAVE", 4) != 0)
    {
        serialPrint("Not a WAVE file - ");
        serialPrint_fileHeader(Wav->RiffFormat, 4);
        return;
    }
    if (memcmp(Wav->FormatSectionID, "fmt", 3) != 0)
    {
        serialPrint("fmt ID not present - ");
        serialPrint_fileHeader(Wav->FormatSectionID, 3);
        return;
    }
    if (memcmp(Wav->DataSectionID, "data", 4) != 0)
    {
        serialPrint("data ID not present - ");
        serialPrint_fileHeader(Wav->DataSectionID, 4);
        return;
    }
    // All looks good, dump the data
    serialPrint("Load success: Size:");
    Serial.print(Wav->Size);
    Serial.print(" Format section size:");
    Serial.print(Wav->FormatSize);
    Serial.print(" Wave format:");
    Serial.print(Wav->FormatID);
    Serial.print(" Channels:");
    Serial.print(Wav->NumChannels);
    Serial.print(" Sample Rate:");
    Serial.print(Wav->SampleRate);
    Serial.print(" Byte Rate:");
    Serial.print(Wav->ByteRate);
    Serial.print(" Block Align:");
    Serial.print(Wav->BlockAlign);
    Serial.print(" Bits Per Sample:");
    Serial.print(Wav->BitsPerSample);
    Serial.print(" data Size:");
    Serial.print(Wav->DataSize);
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