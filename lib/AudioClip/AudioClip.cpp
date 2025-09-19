#include "AudioClip.h"

AudioClip::AudioClip(AudioClipController *controller,
                     const String &fileName,
                     float volume,
                     bool repeat)
    : _controller(controller),
      _id(fileName),
      _volume(volume),
      _isChange(false)
{
    _wav.Repeat = repeat;
    serialPrint(" AudioClip::AudioClip constructor ");
    if (!_loadWavFileHeader(fileName))
        while (true)
            ;
}

void AudioClip::play()
{
    if (!_wav.Playing)
    {
        _wav.Playing = true;
        _progressPercent = 0;
        _isChange = true;
        serialPrint(" play()");
    }
}

void AudioClip::stop()
{
    if (_wav.Playing)
    {
        _wav.Playing = false;
        _isChange = true;
        serialPrint(" stop()");
    }
}

// public static
void AudioClip::loop(std::list<AudioClip *> &items)
{
    bool _noPlaying = true;

    for (AudioClip *item : items)
    {
        if (item->isPlaying())
        {
            _noPlaying = false;
            break;
        }
    }
    if (_noPlaying)
    {
        vTaskDelay(1000);
        return;
    }

    static bool ReadingFile = true;
    static bool IsRead = true;

    static byte Samples[NUM_BYTES_TO_READ_FROM_FILE];
    static uint16_t BytesReadFromFile;
    if (ReadingFile) // Read next chunk of data in from files
    {
        // Read data into the wavs own buffers
        IsRead = false;
        for (AudioClip *item : items)
        {
            if (item->_read())
                IsRead = true;
        }
        if (!IsRead)
        {
            Serial.print(" AudioClip::loop -- no Playing!");
            vTaskDelay(1000);
            return;
        }
        BytesReadFromFile = Mix(Samples, items); // Mix the samples together and store in the samples buffer
        ReadingFile = false;                     // Switch to sending the buffer to the I2S
    }
    else
        ReadingFile = FillI2SBuffer(Samples, BytesReadFromFile); // We keep calling this routine until it returns true, at which point
}

bool AudioClip::isPlaying() const
{
    return _wav.Playing;
}

float AudioClip::getPlayingProgress() const
{
    return _progressPercent;
}

//--------------------------------------------------------------------------------------------------------------
// PRIVATE
bool AudioClip::_loadWavFileHeader(String FileName)
{
    // Load wav file, if all goes ok returns true else false
    WavHeader_Struct WavHeader;

    _wav.WavFile = SD.open(FileName); // Open the wav file
    if (_wav.WavFile == false)
    {
        Serial.print("Could not open :");
        Serial.println(FileName);
        return false;
    }
    else
    {
        _wav.WavFile.read((byte *)&WavHeader, 44); // Read in the WAV header, which is first 44 bytes of the file.
                                                   // We have to typecast to bytes for the "read" function
        if (_validWavData(&WavHeader))
        {
            _dumpWAVHeader(&WavHeader); // Dump the header data to serial, optional!
            Serial.println();
            _wav.DataSize = WavHeader.DataSize; // Copy the data size into our wav structure
            return true;
        }
        else
            return false;
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

bool AudioClip::_read()
{
    if (!_wav.Playing)
        return false;

    if (_wav.TotalBytesRead + NUM_BYTES_TO_READ_FROM_FILE > _wav.DataSize) // If next read will go past the end then adjust the
        _wav.LastNumBytesRead = _wav.DataSize - _wav.TotalBytesRead;       // amount to read to whatever is remaining to read
    else
        _wav.LastNumBytesRead = NUM_BYTES_TO_READ_FROM_FILE; // Default to max to read

    _wav.WavFile.read(_wav.Samples, _wav.LastNumBytesRead); // Read in the bytes from the file
    _wav.TotalBytesRead += _wav.LastNumBytesRead;           // Update the total bytes red in so far

    _onEndCall();

    if (_wav.TotalBytesRead >= _wav.DataSize) // Have we read in all the data?
    {
        if (_wav.Repeat)
        {
            serialPrint("Repeat!");
            _wav.WavFile.seek(44);   // Reset to start of wav data
            _wav.TotalBytesRead = 0; // Clear to no bytes read in so far
        }
        else
        {
            _wav.WavFile.seek(44);   // Reset to start of wav data
            _wav.TotalBytesRead = 0; // Clear to no bytes read in so far
            _wav.Playing = false;    // Flag that wav has completed

            serialPrint("sample end!");
            serialPrint("... ");
        }
    }
    if (_wav.LastNumBytesRead == 0)
        return false;
    if (!_wav.Samples)
    {
        Serial.println("ERROR: _wav.Samples is null in _read()");
        return false;
    }
    return _wav.Playing;
}

void AudioClip::_onEndCall()
{
    _progressPercent = (float)_wav.TotalBytesRead / (float)_wav.DataSize * 100.0f;

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

uint16_t AudioClip::Mix(byte *samples, std::list<AudioClip *> &items)
{
    // Mix all playing wavs together, returns the max bytes that are in the buffer, usually this would be the full buffer but
    // in rare cases wavs may be close to the end of the file and thus not fill the entire buffer
    int32_t mixedSample;       // The mixed sample
    uint16_t i;                // index into main samples buffer
    uint16_t MaxBytesInBuffer; // Max bytes of data in buffer, most of time buffer will be full
    const float Volume = 0.3;
    i = 0;

    for (AudioClip *item : items)
        item->_wavIdx = 0;

    bool runing = true;

    while (runing)
    {
        runing = false;

        for (AudioClip *item : items)
            if (item->_wavIdx < item->_wav.LastNumBytesRead)
            {
                runing = true;
                break;
            }
        if (!runing)
            break;

        mixedSample = 0;
        int activeCount = 0;
        for (AudioClip *item : items)
        {

            if (item->_wav.Playing && item->_wav.Samples != nullptr && item->_wavIdx < item->_wav.LastNumBytesRead)
            {
                mixedSample += *((int16_t *)(item->_wav.Samples + item->_wavIdx)) * item->_volume;
                item->_wavIdx += 2;
                activeCount++;
            }
        }

        if (mixedSample > 32767 || mixedSample < -32768)
        {
            mixedSample /= activeCount;
        }

        if (mixedSample > 32767)
            mixedSample = 32767;
        if (mixedSample < -32768)
            mixedSample = -32768;

        if (i + 1 < NUM_BYTES_TO_READ_FROM_FILE)
        {
            *((int16_t *)(samples + i)) = (int16_t)mixedSample;
        }
        else
        {
            break;
        }

        i += 2;
    }

    MaxBytesInBuffer = 0;
    for (AudioClip *item : items)
        if (item->_wav.LastNumBytesRead > MaxBytesInBuffer)
            MaxBytesInBuffer = item->_wav.LastNumBytesRead;

    if (MaxBytesInBuffer + 2 > NUM_BYTES_TO_READ_FROM_FILE)
        MaxBytesInBuffer = NUM_BYTES_TO_READ_FROM_FILE - 2;

    //  We now alter the data according to the volume control
    for (i = 0; i < MaxBytesInBuffer; i += 2)
        *((int16_t *)(samples + i)) = (*((int16_t *)(samples + i))) * Volume;

    return MaxBytesInBuffer;
}

bool AudioClip::FillI2SBuffer(byte *Samples, uint16_t BytesInBuffer)
{
    // Writes bytes to buffer, returns true if all bytes sent else false, keeps track itself of how many left
    // to write, so just keep calling this routine until returns true to know they've all been written, then
    // you can re-fill the buffer

    size_t BytesWritten;           // Returned by the I2S write routine,
    static uint16_t BufferIdx = 0; // Current pos of buffer to output next
    uint8_t *DataPtr;              // Point to next data to send to I2S
    uint16_t BytesToSend;          // Number of bytes to send to I2S

    // To make the code eaier to understand I'm using to variables to some calculations, normally I'd write this calcs
    // directly into the line of code where they belong, but this make it easier to understand what's happening

    DataPtr = Samples + BufferIdx;                                            // Set address to next byte in buffer to send out
    BytesToSend = BytesInBuffer - BufferIdx;                                  // This is amount to send (total less what we've already sent)
    i2s_write(I2S_NUM_0, DataPtr, BytesToSend, &BytesWritten, portMAX_DELAY); // Send the bytes, wait 1 RTOS tick to complete
    BufferIdx += BytesWritten;                                                // increasue by number of bytes actually written

    if (BufferIdx >= BytesInBuffer)
    {
        // sent out all bytes in buffer, reset and return true to indicate this
        BufferIdx = 0;
        return true;
    }
    else
        return false; // Still more data to send to I2S so return false to indicate this
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