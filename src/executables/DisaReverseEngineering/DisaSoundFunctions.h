/*
 * GNU Lesser General Public License (LGPL-3.0)
 *
 * Copyright (c) 2026 ProgSys
 *
 * This code is part of a free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * This code is distributed in the hope that it will be useful, without any
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 * or https://www.gnu.org/licenses/lgpl-3.0.md
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#include <array>
#include <cstdint>

/**
* @brief Reference counted handle of an opened audio file.
* @details FUN_00434720 opens it, FUN_00439260 frees it once the count drops to 0. Only the count is known.
*/
struct DisaAudioFile {
    int32_t refCount;
};

/**
* @brief A pooled OpenAL source with its Ogg Vorbis decoder and refill thread.
* @details FUN_00434910 takes one from the pool, FUN_00435c00 hands it back once the count drops to 0.
* Only the fields used by the music code are named, the object is never created on this side.
*/
struct DisaAudioStream {
    std::array<uint8_t, 0x58> unknown0;
    int32_t refCount;                       //0x58
    int32_t unknown5c;                      //0x5c FUN_00435720 reads the play state into it
    std::array<uint8_t, 0x2> unknown60;
    uint8_t gain;                           //0x62
    uint8_t unknown63;
    int16_t targetGain;                     //0x64
    std::array<uint8_t, 0x4> unknown66;
    int16_t fadeFrame;                      //0x6a
    std::array<uint8_t, 0x10> unknown6c;
    int32_t fadeGain;                       //0x7c
    std::array<uint8_t, 0x14> unknown80;
    uint8_t isFadingOut;                    //0x94 guessed, FUN_00435720 stops the source when set
    uint8_t isFadingIn;                     //0x95 guessed

    /**
    * @brief Sets the gain, or the gain a running fade ends at.
    * Original function: FUN_004359e0
    */
    void FUN_004359e0_setGain(int16_t newGain);
};

/**
* @brief One of the two streamed channels, slot 0 is the music, slot 1 the voices.
* @details The array starts at DAT_00778df0. The large size comes from the decode buffer inside the slot.
* Volumes are on the game scale (0 to 0xff), the gains are the volumes times masterVolume.
*/
struct DisaStreamSlot {
    float masterVolume;                     //0x00 the volume setting / 5
    int32_t gainLeft;                       //0x04
    int32_t gainRight;                      //0x08
    int32_t volumeLeft;                     //0x0c
    int32_t volumeRight;                    //0x10
    int32_t unknown14;
    int32_t unknown18;
    int32_t unknown1c;
    int32_t openTick;                       //0x20 DAT_006e62bc when the file was opened
    int32_t openVolumeLeft;                 //0x24
    int32_t openVolumeRight;                //0x28
    int32_t unknown2c;
    const char* fileName;                   //0x30
    int32_t unknown34;
    int32_t unknown38;
    int32_t fileSize;                       //0x3c
    std::array<uint8_t, 0xc> unknown40;
    int32_t flags;                          //0x4c 1 starts playing on open, FUN_00510ad0 adds 2
    int32_t unknown50;                      //0x50 handed to FUN_00434910
    int32_t unknown54;
    int32_t loopMode;                       //0x58
    int32_t unknown5c;
    uint32_t startPosition;                 //0x60 in samples at 44.1 kHz
    int32_t loopStart;                      //0x64
    int32_t loopEnd;                        //0x68
    int32_t unknown6c;
    std::array<uint8_t, 0x5007d - 0x70> buffer;
    uint8_t state;                          //0x5007d 1 opened, 2 playing
    std::array<uint8_t, 0x2> unknown5007e;
    DisaAudioStream* stream;                //0x50080
    DisaAudioFile* file;                    //0x50084
};
static_assert(sizeof(DisaStreamSlot) == 0x50088);

/**
* @brief A music track, the tables DAT_00545a50 (ids below 100) and DAT_0053ee98 (ids from 101 on).
*/
struct DisaMusicEntry {
    int16_t fileIndex;                      ///index into the file names at DAT_0058c198
    int16_t baseVolume;                     ///0 to 0x7fff
    int32_t loopStart;
    int32_t loopEnd;
    int8_t loopMode;
    std::array<uint8_t, 0x3> unknownD;
};
static_assert(sizeof(DisaMusicEntry) == 0x10);

/**
* @brief An entry of the table at DAT_0088abc8, likely MUSICSHOP.DAT. Only the music id is known.
*/
struct DisaMusicShopEntry {
    std::array<uint8_t, 0xa> unknown0;
    int16_t musicId;                        //0x0a
    std::array<uint8_t, 0x88> unknownC;
};
static_assert(sizeof(DisaMusicShopEntry) == 0x94);

/**
* @brief Plays a music, 0 and 100 stop it.
* Original function: FUN_0046a830
* @details Does nothing when the music already plays, unless the music volume is 0. Ids below 100 come from
* DAT_00545a50, ids above 100 from DAT_0053ee98. The music is marked as heard in DAT_0065e360.
*/
void __fastcall FUN_0046a830_playMusic(int32_t music);

/**
* @brief Plays a music track with an id below 100 on stream slot 0.
* Original function: FUN_00504f30
* @param loopMode -2 takes the loop mode of the track
*/
void FUN_00504f30_playMusicTrack(int32_t track, int32_t volume, int32_t loopMode);

/**
* @brief Plays a music track with an id from 101 on, on stream slot 0.
* Original function: FUN_005050f0
* @param loopMode -2 takes the loop mode of the track
*/
void FUN_005050f0_playEventMusicTrack(int32_t track, int32_t volume, int32_t loopMode);

/**
* @brief Opens a file on a stream slot, and starts it when flags is not 0.
* Original function: FUN_00510840, always returns 0.
* @details Stops and releases what the slot played before. The stream starts with the current gain of the slot,
* the caller sets the volume right after.
*/
int32_t FUN_00510840_openStream(int32_t slot, const char* fileName, int32_t fileSize, int32_t flags);

/**
* @brief Stops a stream slot and releases its file. Nothing happens to the file while no stream plays.
* Original function: FUN_00510ad0
*/
void FUN_00510ad0_stopStream(int32_t slot);

/**
* @brief Sets the volume of a stream slot and hands the gain to its stream, a negative gain stops it.
* Original function: FUN_00510b30
*/
void FUN_00510b30_setStreamVolume(int32_t slot, int32_t volumeLeft, int32_t volumeRight);

/**
* @brief The size of a file, from the loaded archives or from the disk, 0 when it does not exist.
* Original function: FUN_00419870
*/
int32_t FUN_00419870_getFileSize(const char* fileName);
