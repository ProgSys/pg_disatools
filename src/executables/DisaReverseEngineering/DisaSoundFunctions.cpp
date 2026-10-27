#include "DisaSoundFunctions.h"

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <span>
#include <utility>
#include <sys/types.h>
#include <sys/stat.h>

#include <FunctionPatcher.h>

static_assert(offsetof(DisaAudioStream, refCount) == 0x58);
static_assert(offsetof(DisaAudioStream, fadeGain) == 0x7c);
static_assert(offsetof(DisaAudioStream, isFadingIn) == 0x95);
static_assert(offsetof(DisaStreamSlot, fileName) == 0x30);
static_assert(offsetof(DisaStreamSlot, loopMode) == 0x58);
static_assert(offsetof(DisaStreamSlot, state) == 0x5007d);

namespace {
    /// DAT_00677278, the music volume of the settings.
    int32_t& DAT_00677278_musicVolume() { return FP::getData<int32_t>(0x00677278); }
    /// DAT_006b75a4, the music that plays, -1 after it was started with a volume of 0.
    int32_t& DAT_006b75a4_currentMusic() { return FP::getData<int32_t>(0x006b75a4); }
    /// DAT_0065db28, the music kept in the save data.
    int16_t& DAT_0065db28_savedMusic() { return FP::getData<int16_t>(0x0065db28); }
    /// DAT_0065db2a, the music volume kept in the save data.
    int16_t& DAT_0065db2a_savedMusicVolume() { return FP::getData<int16_t>(0x0065db2a); }
    /// DAT_008b7db8, the last track started by FUN_00504f30.
    int32_t& DAT_008b7db8_currentTrack() { return FP::getData<int32_t>(0x008b7db8); }
    /// DAT_008b7dbc, the last track started by FUN_005050f0.
    int32_t& DAT_008b7dbc_currentEventTrack() { return FP::getData<int32_t>(0x008b7dbc); }
    /// DAT_00545a50, the music tracks with ids below 100.
    DisaMusicEntry* DAT_00545a50_musicTracks() { return &FP::getData<DisaMusicEntry>(0x00545a50); }
    /// DAT_0053ee98, the music tracks from id 101 on, indexed with id - 101.
    DisaMusicEntry* DAT_0053ee98_eventMusicTracks() { return &FP::getData<DisaMusicEntry>(0x0053ee98); }
    /// DAT_0058c198, the file names of the music, "dummy.dat" first.
    const char** DAT_0058c198_audioFileNames() { return &FP::getData<const char*>(0x0058c198); }
    /// DAT_00778df0, the two stream slots.
    std::array<DisaStreamSlot, 2>& DAT_00778df0_streamSlots() { return FP::getData<std::array<DisaStreamSlot, 2>>(0x00778df0); }
    /// DAT_0088abc4, points at the entry count of DAT_0088abc8.
    uint16_t* DAT_0088abc4_musicShopCount() { return FP::getData<uint16_t*>(0x0088abc4); }
    /// DAT_0088abc8, the music shop entries.
    DisaMusicShopEntry* DAT_0088abc8_musicShopEntries() { return FP::getData<DisaMusicShopEntry*>(0x0088abc8); }
    /// DAT_0065e360, one byte per music shop entry, bit 1 is set once the music was played.
    uint8_t* DAT_0065e360_musicHeardFlags() { return &FP::getData<uint8_t>(0x0065e360); }
    /// DAT_006e62bc, counts up every frame.
    int32_t DAT_006e62bc_tick() { return FP::getData<int32_t>(0x006e62bc); }
    /// DAT_005bae4c, the file system that owns the loaded archives.
    void* DAT_005bae4c_fileSystem() { return FP::getData<void*>(0x005bae4c); }

    /**
    * @brief Replaces the CRT _ftol2 at 0x00523f90, truncates towards zero.
    * @details The original multiplies on the x87 stack, where float times float is exact. Taking double keeps that.
    */
    int32_t FUN_00523f90_truncate(double value) { return static_cast<int32_t>(static_cast<int64_t>(value)); }

    /// Original function: FUN_00435720, stops the OpenAL source and waits for the refill thread.
    void FUN_00435720_stopSource(DisaAudioStream* stream) { FP::callStdFastFarA<void>(0x00435720, stream); }
    /// Original function: FUN_00435c00, frees the decoder and hands the stream back to the pool.
    void FUN_00435c00_freeStream(DisaAudioStream* stream) { FP::callStdFastFarA<void>(0x00435c00, stream); }
    /// Original function: FUN_00439260, closes the file.
    void FUN_00439260_freeFile(DisaAudioFile* file) { FP::callStdFastFarA<void>(0x00439260, file); }

    /**
    * @brief Opens a file from the archives or the disk, the result has a count of 1.
    * Original function: FUN_00434720
    * @details The third argument is never read, the original passes whatever is in ECX.
    */
    void FUN_00434720_openFile(DisaAudioFile** fileOut, const char* fileName) {
        FP::callStdFarA<DisaAudioFile**>(0x00434720, fileOut, fileName, 0);
    }

    /**
    * @brief Takes a stream from the pool, sets it up for the file and starts it. The result has a count of 1.
    * Original function: FUN_00434910
    * @details Calls into OpenAL and Vorbis. Takes over the reference of file. The last three arguments are never
    * read, the original leaves them uninitialised.
    */
    void FUN_00434910_createStream(DisaAudioStream** streamOut, DisaAudioFile* file, uint32_t gain, int32_t loopMode, int32_t unknown50,
        int32_t startMs) {
        FP::callStdFarA<DisaAudioStream**>(0x00434910, streamOut, file, gain, loopMode, unknown50, startMs, 0, 0, 0);
    }

    /// Inlined in the original wherever a stream is released.
    void releaseStream(DisaAudioStream* stream) {
        if (stream && --stream->refCount == 0) FUN_00435c00_freeStream(stream);
    }

    /**
    * @brief Releases a file.
    * Original function: FUN_00434cf0, it takes a pointer to the handle and has FUN_00439260 inlined.
    */
    void FUN_00434cf0_releaseFile(DisaAudioFile* file) {
        if (file && --file->refCount == 0) FUN_00439260_freeFile(file);
    }

    /// The shared body of FUN_00504f30 and FUN_005050f0.
    void startMusicTrack(const DisaMusicEntry& track, int32_t volume, int32_t loopMode) {
        //imul in the original, it wraps
        const int32_t scaled = static_cast<int32_t>(static_cast<uint32_t>(track.baseVolume) * static_cast<uint32_t>(volume) * 0xffu) >> 15;
        const int32_t gain = FUN_00523f90_truncate(static_cast<float>(scaled));

        DisaStreamSlot& slot = DAT_00778df0_streamSlots()[0];
        slot.loopMode = loopMode == -2 ? track.loopMode : loopMode;

        const char* fileName = DAT_0058c198_audioFileNames()[track.fileIndex];
        FUN_00510840_openStream(0, fileName, FUN_00419870_getFileSize(fileName), 1);
        FUN_00510b30_setStreamVolume(0, gain, gain);
        slot.loopStart = track.loopStart;
        slot.startPosition = 0;
        slot.loopEnd = track.loopEnd;
    }
}

void DisaAudioStream::FUN_004359e0_setGain(int16_t newGain) {
    if (!isFadingOut && !isFadingIn) {
        gain = static_cast<uint8_t>(newGain);
        return;
    }
    fadeGain = newGain;
    fadeFrame = 0;
}

void __fastcall FUN_0046a830_playMusic(int32_t music) {
    const int32_t volume = DAT_00677278_musicVolume();
    if (DAT_006b75a4_currentMusic() == music && volume != 0) return;

    DAT_006b75a4_currentMusic() = volume == 0 ? -1 : (music & 0xffff);
    DAT_0065db28_savedMusic() = static_cast<int16_t>(music);
    DAT_0065db2a_savedMusicVolume() = static_cast<int16_t>(volume);

    if (music == 0 || music == 100) {
        FUN_00510ad0_stopStream(0);
        return;
    }
    if (music < 100) FUN_00504f30_playMusicTrack(music, volume, -2);
    else FUN_005050f0_playEventMusicTrack(music, volume, -2);

    //the original tests the first entry before it looks at the count
    const int32_t count = std::max<int32_t>(*DAT_0088abc4_musicShopCount(), 1);
    const std::span entries(DAT_0088abc8_musicShopEntries(), count);
    const auto findIt = std::ranges::find(entries, music, [](const DisaMusicShopEntry& entry) { return int32_t(entry.musicId); });
    if (findIt != entries.end()) DAT_0065e360_musicHeardFlags()[std::distance(entries.begin(), findIt)] |= 1;
}

void FUN_00504f30_playMusicTrack(int32_t track, int32_t volume, int32_t loopMode) {
    DAT_008b7db8_currentTrack() = track;
    startMusicTrack(DAT_00545a50_musicTracks()[track], volume, loopMode);
}

void FUN_005050f0_playEventMusicTrack(int32_t track, int32_t volume, int32_t loopMode) {
    DAT_008b7dbc_currentEventTrack() = track;
    startMusicTrack(DAT_0053ee98_eventMusicTracks()[track - 101], volume, loopMode);
}

int32_t FUN_00510840_openStream(int32_t slotIndex, const char* fileName, int32_t fileSize, int32_t flags) {
    DisaStreamSlot& slot = DAT_00778df0_streamSlots()[slotIndex];
    if (slot.stream) FUN_00435720_stopSource(slot.stream);
    releaseStream(std::exchange(slot.stream, nullptr));
    FUN_00434cf0_releaseFile(std::exchange(slot.file, nullptr));

    slot.fileSize = fileSize;
    slot.flags = flags;
    slot.fileName = fileName;
    slot.unknown38 = 0;
    slot.unknown34 = 0;
    slot.openTick = DAT_006e62bc_tick();
    slot.openVolumeLeft = slot.volumeLeft;
    slot.openVolumeRight = slot.volumeRight;
    slot.unknown1c = 0;
    slot.unknown2c = 0;
    slot.unknown14 = 0;
    slot.unknown18 = 0;
    slot.startPosition = 0;
    slot.unknown6c = 0;
    slot.loopStart = 0;
    slot.loopEnd = 0;

    DisaAudioFile* opened = nullptr;
    FUN_00434720_openFile(&opened, fileName);
    if (opened) ++opened->refCount;
    FUN_00434cf0_releaseFile(std::exchange(slot.file, opened));
    FUN_00434cf0_releaseFile(opened);

    if (flags == 0) {
        slot.state = 1;
        return 0;
    }

    const int32_t startMs = FUN_00523f90_truncate(static_cast<double>(static_cast<float>(slot.startPosition)) / 44.1);
    if (slot.file) ++slot.file->refCount;
    DisaAudioStream* created = nullptr;
    FUN_00434910_createStream(&created, slot.file, static_cast<uint8_t>(slot.gainLeft), slot.loopMode, slot.unknown50, startMs);
    if (created) ++created->refCount;
    releaseStream(std::exchange(slot.stream, created));
    releaseStream(created);
    slot.state = 2;
    return 0;
}

void FUN_00510ad0_stopStream(int32_t slotIndex) {
    DisaStreamSlot& slot = DAT_00778df0_streamSlots()[slotIndex];
    slot.flags |= 2;
    if (!slot.stream) return;

    FUN_00435720_stopSource(slot.stream);
    releaseStream(std::exchange(slot.stream, nullptr));
    FUN_00434cf0_releaseFile(std::exchange(slot.file, nullptr));
}

void FUN_00510b30_setStreamVolume(int32_t slotIndex, int32_t volumeLeft, int32_t volumeRight) {
    DisaStreamSlot& slot = DAT_00778df0_streamSlots()[slotIndex];
    slot.volumeLeft = volumeLeft;
    slot.volumeRight = volumeRight;
    const double master = slot.masterVolume;
    slot.gainLeft = FUN_00523f90_truncate(master * static_cast<float>(volumeLeft));
    slot.gainRight = FUN_00523f90_truncate(master * static_cast<float>(volumeRight));

    DisaAudioStream* stream = slot.stream;
    if (!stream) return;

    const int16_t gain = static_cast<int16_t>(slot.gainLeft);
    stream->targetGain = gain;
    if (stream->isFadingOut || stream->isFadingIn) {
        stream->fadeGain = gain;
        stream->fadeFrame = 0;
        return;
    }
    if (gain >= 0) stream->FUN_004359e0_setGain(gain);
    else FUN_00435720_stopSource(stream);
}

int32_t FUN_00419870_getFileSize(const char* fileName) {
    void* fileSystem = DAT_005bae4c_fileSystem();
    const int32_t index = FP::callThisFarA<int32_t>(0x00418d00, fileSystem, fileName, 2);
    if (index < 0) {
        struct _stat64 info{};
        const char* path = FP::callThisFarA<const char*>(0x0041c180, fileSystem, 0, fileName);
        //the CRT of the game, through its import table
        FP::getData<decltype(&_stat64)>(0x00528144)(path, &info);
        return static_cast<int32_t>(info.st_size);
    }

    //an archive takes 0x48 bytes, the high byte of the index picks it, the low three bytes the entry
    const uint8_t* archive = *reinterpret_cast<const uint8_t* const*>(static_cast<uint8_t*>(fileSystem) + 0xbeda8) + (index >> 24) * 0x48;
    const int32_t archiveType = *reinterpret_cast<const int32_t*>(archive + 0x30);
    const uint8_t* entries = *reinterpret_cast<const uint8_t* const*>(archive + 0x40);
    const int32_t entry = index & 0xffffff;
    switch (archiveType) {
        case 0: return *reinterpret_cast<const int32_t*>(entries + entry * 0x40 + 0x30);
        case 10: return *reinterpret_cast<const int32_t*>(entries + entry * 0x34 + 0x2c);
        default: return *reinterpret_cast<const int32_t*>(entries + entry * 0x30 + 0x18);
    }
}
