import serial
import time
import os
from pydub import AudioSegment
import audioread
import struct

def readMusicFile(file_path):
    pass

def sendMetaData(ser, channel, sampleRate):
    # 패킷 초기화 (1024 바이트)
    packet = bytearray(1024)
    # "ST"를 첫 2바이트에 저장
    packet[:2] = b"ST" # 0x53 0x54
    # channel (2 Byte) 추가
    packet[2:4] = struct.pack("<H", channel)
    # sampleRate (2 Byte) 추가
    packet[4:6] = struct.pack("<H", sampleRate)
    # 패킷 전송
    ser.write(packet)

def sendRawData(file_path, port="COM3", baudrate=115200):
    try:
        # serial comm
        ser = serial.Serial(
            port, 
            baudrate, 
            parity='N',
            stopbits=1,
            bytesize=8,
            timeout=3
            )
        
        # read file to get Channel Count(Mono, Stereo, etc.), Sample Rate (16000, 44100..)
        with audioread.audio_open(file_path) as f:
            channelCnt = f.channels
            sampleRate = f.samplerate
            print(f.channels, f.samplerate)
        

        # extract raw data(bytes)
        song = AudioSegment.from_mp3(file_path)
        rawData = song.raw_data

        sendMetaData(ser=ser, channel=channelCnt, sampleRate=sampleRate)

        file_data = rawData
        file_length = len(rawData)
        # print(len(file_data))
        for idx in range(0, file_length, 1024):
            # print(f"1024bytes: {file_data[idx:idx+1024]}\n\n")
            ser.write(file_data[idx:idx+1024])


        ######################## TEST CODE ##########################    
        # sound = AudioSegment(
        #     # raw audio data (bytes data)
        #     data=rawData,
        #     # 2 byte (16 bit) samples
        #     sample_width=2,
        #     frame_rate=sampleRate,
        #     channels=channelCnt
        # )
        # # # Check \x00\x1cG\x1e\xd1\x1e\xac\x1d\xac\x1c
        # # print(type(rawData), rawData[8192:8300])
        # # 아래부터는 그냥 send 하면 된다
        # f = open("player/music/test33.raw", 'wb')
        # f.write(rawData)
        # f.close()
        # file_handle = sound.export("player/music/test.raw", format="raw")
        # raw_audio = AudioSegment.from_file("player/music/test33.raw", format="raw",
        #                            frame_rate=16000, channels=1, sample_width=2)
        # file_handle = sound.export("player/music/test223.wav", format="wav")
        ######################## TEST CODE ##########################    

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
    
if __name__ == '__main__':
    sendRawData("player/music/E_01_SPEAKER_POWER_ON.mp3")
