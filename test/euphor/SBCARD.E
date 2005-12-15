-- sbcard.e
-- sound blaster card interface
-- by Jacques Deschenes, Baie-Comeau, P.Q., Canada, e-mail:desja@quebectel.com
-- creation date: October 11th, 1996
-- version 1.0
-- This first version doesn't use interrupt service routine, so can't play in
-- background, except for file small enough to be loaded completely in DMA
-- buffer all at once (max size 64KB)
-- It only play 8 bits files, mono or stereo.
-- It has been written for sound blaster pro with dsp version 3.
-- But it can be used to play mono file on prior dsp version card.
--
-- list of globals procedures and functions.
--   procedure PlayBuffer(atom buffer, integer size, 
--                        integer TimeConstant, integer channels) 
--  function ComputeTC(integer SampleRate, NbChannels) 
--  procedure SetChannels(integer channels)
--  function BufferDone()
--  function GetWavInfo(integer FHandle)
--  procedure SetVolume(byte volume)
--  procedure SetOutputFilter(integer State)
--  procedure DmaStop()
--  procedure DmaContinue()
--  procedure SpeakerOn()
--  procedure SpeakerOff()
--  procedure WriteDSP(byte data)
--  function ReadDSP()
--  procedure WriteDAC(byte data)
--  procedure ReadADC()
--  procedure ResetDSP()
--  GetDSPVersion() 
-- 
-- **** How to play a sound. ***
-- 1) First of all it need environment variable "BLASTER" to be correctly set.
--    InitDSP() read this variable to set card parameters.
-- 2) use AllocateDMABuffer() (from dma.e file) to allocate a valid buffer for
--    dma transfert. (maximum buffer size is 64KB)
-- 3) Open the wave file and use GetWavInfo to read the header information.
--    this function return a sequence of parameters needed for card setup.
-- 4) Call SetChannels(WavInfo[siCHANNELS]) to set for stereo or mono 
--    file sound. where WavInfo is the sequence returned by GetWavInfo.
-- 5) Read in file sound data in the dma buffer.
-- 6) call SpeakerOn()
-- 7) call SetVolume(volume)
-- 8) call ComputeTC(WavInfo[siSPS],WavInfo[siCHANNELS])
-- 9  call PlayBuffer()
-- 10) call BufferDone() to know when a sound is done playing.
-- 11) When sound done call SpeakerOff() if needed.

-- If sound data is to large to hold in buffer the program need to loop and
-- and play it by segments.
-- Study WaveDemo.ex for an example.
--
-- See comment at the beginnig of each proc and func. to know how to all it
-- with valid param.
-- 
--            *************  REVISIONS  History *****************
-- revision: 1.01
-- revision date: Oct. 15th, 1996
-- Corrected global constant VOL_MAX previously set to 255 now set to 7
-- modification to global procedure SetVolume(byte volume)
--  Now it is SetVolume(integer WhichOne,byte volume)
--  it can set any one 6 volume controls of the CT1345 mixer.
--  Added procedure SetOutputFilter(integer State) to set output filter ON/OFF
--
-- revision date: Oct. 29th, 1996
-- Modidification to InitDSP() doesn't abort if fail to reset DSP
-- Added global integer DSP_RESET_OK , which is modified by ResetDSP() to 
-- TRUE if success and FALSE if fail to reset.
--
-- revision date: Oct 30th, 1996
-- added global variable iSbCardError, which is set by ResetDSP(),WriteDSP()
-- iSbCardError replace DSP_RESET_OK which is suppressed from code.
--  and ReadDSP() if there is an error with de sound card, otherwise should
--  be 0 .  This var is and index in ERROR_MESSAGES sequence.
-- ERROR_MESSAGES is now a global variable.
-- no one procedure or function abort now.  the decision to abort is now
-- the responsability of the main program.

-- revision date: Oct 31th, 1996
-- added global procedure AskBlasterInfo() 
--   If there is no BLASTER environment this can be called to supplied needed
--   information.
-- revision date: Nov 2th, 1996
-- corrected bug in GetBlasterInfo()



include machine.e
include ports.e
include delay.e
include dma.e
include bitwise.e
-- include dsp_int.e

constant TRUE = 1, FALSE = 0

-- for debug purpose only --------------------------------------------------
constant debug = 0   -- comment it out or set it to 0 if not debugging
integer fn -- file handle
constant  ErrFileName = "SBCARD.ERR"

-----------------------------------------------------------------------------

global word BASE_IO  -- sound card base io address
global byte IRQ      -- Hardware interrupt number used by the sound card
byte SB_INT -- sound card software interrupt
global byte DMA8      -- 8 bit DMA channel number
global byte DMA16     -- 16 bit DMA channel number
global integer DmaChannel  -- DmaChannel currently used by sound card

DMA16 = 0
DMA8 = 0

word DSP_RESET,        -- dsp reset i/o port
     DSP_READ_DATA,    -- dsp read data i/o port
     DSP_WRITE_DATA ,  -- dsp write data i/o port
     DSP_WRITE_STATUS -- dsp write status i/o port
word  DSP_DATA_AVAIL    -- dsp read status i/o port
     
constant DSP_DATA_READY = #AA  -- byte returned by dsp signal ready condition
    
global integer iSbCardError --ResetDSP(), WriteDSP and ReadDSP()
	-- set this variable in case of error due to sound card.
	-- this error code is an index for the following ERROR_MESSAGES

iSbCardError = 0  -- no error
	 
global constant ERROR_MESSAGES={
    "Failed to reset DSP.\n",
    "Failed to write to DSP.\n",
    "Failed to read from DSP.\n",
    "\"BLASTER\" environment variable not found.\n",
    "Bad syntax in \"BLASTER\" environment variable.\n",
    "Not enough memory.\n",
    "Program enter InitDsp() now.\n"  -- this one for debug only
    } -- ERROR_MESSAGES

global constant 
    ERROR_RESET_DSP = 1, 
    ERROR_WRITE_DSP = 2, 
    ERROR_READ_DSP = 3,
    ERROR_NO_BLASTER = 4,
    ERROR_ENV_BAD =5,   
    ERROR_MEM_ALLOC = 6,
    ENTER_INIT = 7  -- debug message
    
global atom DSPVer  -- DSP version number 


-- some DSP commands
constant SET_BLOCK_SIZE = #48,
	 PAUSE_DMA = #D0,
	 SET_TIME_CONSTANT = #40,
	 SPEAKER_ON = #D1,
	 SPEAKER_OFF = #D3,
	 DMA_STOP = #D0,
	 DMA_CONT = #D4,
	 EXIT_AUTO_INIT = #DA,
	 GET_DSP_VERSION = #E1,
	 SET_SINGLE_MODE = #14,
	 SET_AUTO_INIT = #1C,
	 WRITE_DAC =#10,
	 READ_ADC = #20,
	 INVOKE_INT = #F2,
	 HSPEED_OUT = #91 

-- DSP mode related constant
constant SINGLE_CYCLE = 0
constant AUTO_INIT = 1


constant TIME_OUT = .25 --time out delay for DSP ACCESS (abort after .5 seconds)

------ debugging purpose only  -------------------------------------

procedure ErrorLog(integer ErrCode)
-- log error in a file with date and time info. 
integer fn
sequence dt
   fn = open(ErrFileName,"a")
   if fn = -1 then
      return
   end if
   dt = date()
   printf(fn,"date: %2d/%2d/%2d hour: %2d:%2d:%2d\n%s\n",
	  {dt[1],dt[2],dt[3],dt[4],dt[5],dt[6],ERROR_MESSAGES[ErrCode]})
   close(fn)
end procedure -- ErrorLog()

-----------------------------------------------------------------------

global function ResetDSP()
-- ResetDSP returns TRUE if reset was successful
atom start, TimeOut  
  Output(1,DSP_RESET) 
  delay(.01)  -- problably not needed.  DSP chip ask for a 3 micro second delay
  Output(0,DSP_RESET) 
  start = time()
  TimeOut = 0
  while not TimeOut and floor(Input(DSP_DATA_AVAIL)/#80)=0 do 
    TimeOut = floor(time()-start) >= TIME_OUT
  end while 
  if TimeOut then
    iSbCardError = ERROR_RESET_DSP
    return FALSE
  end if
  if Input(DSP_READ_DATA) = DSP_DATA_READY then
	 iSbCardError = 0
	 return  TRUE
     else
	 iSbCardError = ERROR_RESET_DSP
	 if debug then
	    ErrorLog(ERROR_RESET_DSP)
	 end if
	 return FALSE 
  end if
end function --ResetDSP()

global procedure WriteDSP(byte value) 
-- write a byte to dsp write register
atom start integer TimeOut
  if iSbCardError then
     return
  end if
  start = time()
  TimeOut = 0
  while not TimeOut and floor(Input(DSP_WRITE_STATUS)/#80) = 1 do 
    TimeOut = time() - start > TIME_OUT
  end while
  if TimeOut then
    iSbCardError = ERROR_WRITE_DSP
    if debug then
      ErrorLog(ERROR_WRITE_DSP)
    end if
    return
  end if
  Output(value,DSP_WRITE_DATA)
end  procedure --WriteDSP()

global function ReadDSP() 
-- read a byte from dsp read register
atom start 
integer TimeOut
  if iSbCardError then
     return -1
  end if
  TimeOut = 0
  start = time()
  while not TimeOut and floor(Input(DSP_DATA_AVAIL)/#80) = 0 do 
    TimeOut = time() - start > TIME_OUT
  end while
  if TimeOut then
    iSbCardError = ERROR_READ_DSP
    if debug then
	ErrorLog(ERROR_READ_DSP)
    end if
    return -1
  end if
  return Input(DSP_READ_DATA)
end function -- ReadDSP()

global procedure WriteDAC(byte level) 
--  Write a byte to digital to analog converter.
--  It can be used in a loop to produce sound by sending a sequence of data.
--  of limited use in euphoria due to slow speed tranfert.  
  WriteDSP(WRITE_DAC) 
  WriteDSP(level) 
end procedure -- WriteDAC()

global function ReadADC()  
--  ReadADC reads a byte from analog to digital converter.
-- The ADC is connected to microphone input.
-- Used in a loop could be used to record sound. 
-- Unusable in euphoria due to lack of speed.
  WriteDSP(READ_ADC) 
  return ReadDSP()
end function -- ReadDAC()

global function GetDSPVersion()
-- return the Digital sound processor version number.
integer major, minor
  WriteDSP(GET_DSP_VERSION)
  major = ReadDSP()
  minor = ReadDSP()
  return major+ minor/100
end function -- GetDSPVersion

global procedure SpeakerOn()
--  connects the DAC to the output
  WriteDSP(SPEAKER_ON) 
end procedure -- SpeakerOn()

global procedure SpeakerOff()
--  SpeakerOff disconnects the DAC from the output,
--  but does not affect the DAC operation  
  WriteDSP(SPEAKER_OFF) 
end procedure -- SpeakerOff()

global procedure DMAContinue()
-- continue playing sound.
  WriteDSP(DMA_CONT) 
end  procedure  -- DMAContinue()

global procedure DMAStop()
-- suspend playing sound.
  WriteDSP(DMA_STOP) 
end procedure -- DMAStop()

-- MIXER REGISTERS
global constant MIC_VOLUME = #0A  -- volume levels(0..3)
global constant MASTER_VOLUME = #22  --(0..7)
global constant MIDI_VOLUME = #26 --(0..7)
global constant  CD_VOLUME = #28 --(0..7)
global constant  LINE_VOLUME =#2E --(0..7)
global constant VOICE_VOLUME = #4 --(0..7)
global constant MIXER_ADDR = #4  -- write to that port to select internal reg.
constant MIXER_DATA = #5   -- read or  write here data to (from) registers.
constant FILTER_STEREO = #E  -- outpout filter and mono/stereo register
constant MIXER_RESET = 0  -- write any byte to that register to reset mixer.
constant FILTER_SOURCE = #C -- program low pass input filter and input source.

-- volume min and max value
global constant VOL_OFF = 0,
		VOL_MAX = 7

global procedure SetVolume(integer WhichOne, byte volume)
-- Set the 6 different volume controls of CT1345 mixer, part of sbpro card.
-- volume is limited to 8 levels 0..7  ( 4 levels for MIC_VOLUME 0..3)
-- for WhichOne use one of the _VOLUME constant above 
-- Except for MIC_VOLUME, there is 2 volume left and right
-- for right volume number is multiplied by 2 and for left by 32
-- so byte to write to mixer = volume * 2 + volume * 32
byte  r,l
  if WhichOne = MIC_VOLUME then 
    volume = remainder(volume,4)*2
    Output(WhichOne,BASE_IO + MIXER_DATA) 
  else
    volume = remainder(volume,8) -- insure it's inside bound
    r = volume*2  -- right volume
    l = volume * 32 -- left volume
    Output(WhichOne,BASE_IO + MIXER_ADDR) -- select specific volume register
    Output(r+l,BASE_IO + MIXER_DATA)    -- write volume level to register.
  end if
end procedure -- SetVolume()

global constant FILTER_ON = 1,  FILTER_OFF = 0

global procedure SetOutputFilter(integer State)
-- activate or deactivate the output filter of CT1345 mixer on sbpro card.
integer Current
    Output(FILTER_STEREO,BASE_IO+MIXER_ADDR)
    Current = Input(BASE_IO+MIXER_DATA)
    if State = FILTER_ON then
      Output(OR(Current,#20),BASE_IO+MIXER_DATA)
    else    
      Output(AND(Current,#DF),BASE_IO+MIXER_DATA)
    end if
end procedure -- SetOutputFilter()

function HexToI(sequence HStr)
-- convert an hexadecimal string to an integer
-- always return a number if HStr is not valid return 0
-- Only upper case letters are recognise
integer i
    i = 0
    for  n = 1 to length(HStr) do
	if HStr[n] >= '0' and HStr[n] <= '9' then
	    i = i * 16 + HStr[n] - '0'
	elsif HStr[n] >= 'A' and HStr[n] <= 'Z' then
	    i = i * 16 + HStr[n] - 'A' + 10
	else exit
	end if
    end for
    return i
end function -- HexToI()

function AToI(sequence NbStr)
-- convert a decimal string to it's integer value
integer i
    i = 0
    for n = 1 to length(NbStr) do
	if NbStr[n] >= '0' and NbStr[n] <= '9' then
	    i = i * 10 + NbStr[n] - '0'
	else
	    exit
	end if
    end for
    return i
end function -- AToI()

function upper(sequence Str)
-- convert to upper case
sequence UpStr
    UpStr = ""
    for i = 1 to length(Str) do
	if Str[i] >= 'a' and Str[i] <='z' then
	    UpStr = UpStr & 'A' + (Str[i] - 'a')
	else
	    UpStr = UpStr & Str[i]
	end if
    end for
    return UpStr
end function -- upper()

global procedure AskBlasterInfo()
-- Ask user for sound blaster card paramters.
-- needed parameters are: BASE_IO, IRQ, DMA8, DMA16
sequence InStr
puts(1,"Required information to use sound blaster card.\n")
puts(1,"\nWhat is your sound card base I/O address? (544,576)")
InStr = gets(0)
BASE_IO = AToI(InStr)
puts(1,"\nWhat is your sound card IRQ? (2,5,7,10)")
InStr = gets(0)
IRQ = AToI(InStr)
puts(1,"\nWhat is your sound card 8 bits dma channel? (0,1,3)" )
InStr=gets(0)
DMA8 = AToI(InStr)
puts(1,"\nWhat is your sound card 16 bits dma channel? (4,5,6,7)")
InStr = gets(0)
DMA16 = AToI(InStr)
end procedure  -- AskBlasterInfo()

function GetBlasterInfo()
-- Get sound blaster card info from environment variable BLASTER
object blaster  integer i
blaster =  getenv("BLASTER")
if atom(blaster) then
    if debug then
	ErrorLog(ERROR_NO_BLASTER)
    end if
    return ERROR_NO_BLASTER
end if
    blaster = upper(blaster)
    i = match("A",blaster)
    if not i then
	if debug then
	    ErrorLog(ERROR_ENV_BAD)
	end if
	return ERROR_ENV_BAD
    end if
    BASE_IO = HexToI(blaster[i+1..i+4])
    if BASE_IO < #210 then
	if debug then
	    ErrorLog(ERROR_ENV_BAD)
	end if
	return ERROR_ENV_BAD
    end if
    i = match("I",blaster)
    if not i then
	if debug then
	    ErrorLog(ERROR_ENV_BAD)
	end if
	return ERROR_ENV_BAD
    end if
    IRQ = AToI(blaster[i+1..i+2])
    if not find(IRQ,{2,5,7,10}) then
	if debug then
	    ErrorLog(ERROR_ENV_BAD)
	end if
	return ERROR_ENV_BAD
    end if
    if IRQ < 8 then
       SB_INT = IRQ + 8
    else
       SB_INT = IRQ + #68
    end if
    i = match("D",blaster)
    if not i then
	if debug then
	    ErrorLog(ERROR_ENV_BAD)
	end if
	return ERROR_ENV_BAD
    end if
    DMA8 = blaster[i+1] - '0'
    if not find(DMA8,{0,1,3}) then  -- validate 8 bit dma channel
	if debug then
	    ErrorLog(ERROR_ENV_BAD)
	end if
	return ERROR_ENV_BAD
    end if
    i = match("H",blaster)
    if i  then
	DMA16 = AToI(blaster[i+1..i+2]) 
      if not find(DMA16,{5,6,7}) then  -- validate dma channel
	if debug then
	    ErrorLog(ERROR_ENV_BAD)
	end if
	return ERROR_ENV_BAD
      end if
    end if
   return 0
end function -- GetBlasterInfo()

procedure InitDSP()
-- initialise DSP and some sound card parameters
  iSbCardError = GetBlasterInfo()
  if iSbCardError then
    return
  end if
  --  Calculate the port addresses  
  DSP_RESET = BASE_IO + #6 
  DSP_READ_DATA = BASE_IO + #A 
  DSP_WRITE_DATA = BASE_IO + #C 
  DSP_WRITE_STATUS = BASE_IO + #C 
  DSP_DATA_AVAIL = BASE_IO + #E 
  if not ResetDSP() then
    return
  end if
  DSPVer = GetDSPVersion()
  DmaChannel = DMA8
  ResetDma(DMA8)
  ResetDma(DMA16)
end procedure InitDSP()



   

-- *******   following function read informations in wave files ****

global constant MONO = 1, STEREO = 2

constant sNOT_VALID_WAV = "It's not a valid wav file.\n"
global constant -- index to access Wave info sequence
  siFILE_LEN = 1,  -- file length
  siFMT_ID = 2,    -- format identifier
  siCHANNELS = 3,  -- number of channels
  siSPS = 4,       -- samples per seconds 
  siAVG_BPS = 5,   -- average bits per seconds
  siBLK_ALN = 6,   -- block align
  siBITS_PS = 7,   --  bits per sample 8 or 16
  siDATA_LEN = 8   -- data length
  
function ReadTag(integer FHandle)
-- each block begin by a tag of 4 characters
-- file pointer should be on first char of tag
sequence sTag
    sTag = {}
    for i = 1 to 4 do
	sTag = sTag & getc(FHandle)
    end for
    return sTag
end function -- ReadTag()

function ReadInteger(integer FHandle)
-- read an integer from file
-- file pointer should be on first byte of integer
sequence Bytes
    Bytes = {}
    for  i = 1 to 4 do
	Bytes = Bytes & getc(FHandle) 
    end for
    return bytes_to_int(Bytes)
end function --ReadInteger()

function ReadWord(integer FHandle)
    return getc(FHandle) + getc(FHandle)*256
end function -- ReadWord()

global function GetWavInfo(integer FHandle)
-- read wave file header record
-- return the followin sequence of information.
-- {file length, format id, number of channels, samples per second,
--  average bytes per second, block align, bits per sample, data length}
-- see above for globals constants defined to access information in this
-- sequence.
--
-- structure of wave file header record
-- tagged record type file. tags are 4 characters long.
--     data              offset       data nature
--     ------------------------------------------
--     "RIFF"            [1..4]        file Id tag
--     integer           [5..8]       file length excluding this record
--    "WAVE"             [9..12]      wave file tag, confirm it's a wave file
--    "fmt "             [13..16]     format tag announce format record
--     integer           [17..20]     length of format record
--    word               [21..22]     format tag
--    word               [23..24]     number of channels  (mono=1,stereo=2)
--    integer            [25..28]     samples per seconds
--    integer            [29..32]     average bytes per second  
--    word               [33..34]     block align
--    bits per sample    [35..36]     word
--    "data"|"fact"      [37..40]     tag announce a data record or fact record
--    integer            [41..44]     length of that record, if data it means
--                                    length of sound sample, if fact jump
--                                     over it and go to data record.   

sequence WavInfo ,sTag
integer number
  sTag = ReadTag(FHandle)  -- read "RIFF" tag
  if not match("RIFF", sTag) then
    puts(1,sNOT_VALID_WAV)
    return {}
  end if
  WavInfo = {ReadInteger(FHandle)}  -- read file length
  sTag = ReadTag(FHandle)  -- read "WAVE" tag
  if not match("WAVE", sTag) then
    puts(1,sNOT_VALID_WAV)
    return {}
  end if
  sTag = ReadTag(FHandle)  -- read "fmt " tag
  number = ReadInteger(FHandle) -- read format record length, discart it.
  WavInfo = WavInfo & ReadWord(FHandle) -- read format numeric id
  WavInfo = WavInfo & ReadWord(FHandle) -- read number of channels
  WavInfo = WavInfo & ReadInteger(FHandle) -- read samples per second
  WavInfo = WavInfo & ReadInteger(FHandle)  -- read average bytes per second, discard it.
  WavInfo = WavInfo & ReadWord(FHandle)  -- read block align
  WavInfo = WavInfo & ReadWord(FHandle) -- read number of bits per sample.
  sTag = ReadTag(FHandle)  -- should be  "data" or "fact" tag.
  if match("data",sTag) then
    WavInfo = WavInfo & ReadInteger(FHandle)
  else -- fact record, jump over it
    number = ReadInteger(FHandle) -- read fact record length
    for i = 1 to number do
	number = getc(FHandle)   -- skip fact record
    end for
    sTag = ReadTag(FHandle)
    if match("data", sTag) then
	WavInfo = WavInfo & ReadInteger(FHandle) -- read data length
    else -- something wrong with that file.
      puts(1,sNOT_VALID_WAV)
      return {}
    end if
  end if
  return WavInfo
end function -- GetWaveInfo()
-- ************************************************************************

global function BufferDone()
-- read DMA status register to check  for terminal count flag
-- see dma.e file
integer r
   if ReadStatus(DmaChannel) then
     r = Input(DSP_DATA_AVAIL) -- aknowledge DSP interrupt
     return 1
   else
      return 0
   end if
end function -- BufferDone()


global procedure SetChannels(integer channels)
-- set dsp for MONO or STEREO
integer temp
atom buffer
if channels = MONO then
  Output(FILTER_STEREO,BASE_IO+MIXER_ADDR)
  temp = Input(BASE_IO+MIXER_DATA)
  Output(AND(temp,#FD),BASE_IO+MIXER_DATA)
else
  Output(FILTER_STEREO,BASE_IO+MIXER_ADDR)
  temp = Input(BASE_IO+MIXER_DATA)
  Output(OR(temp,2),BASE_IO+MIXER_DATA)     
  -- reset mixer output toggle
  buffer = allocate_low(16)
  poke(buffer,#80)
  SetDma(buffer,1,DmaChannel,DMA_OUT+DMA_SINGLE)
  WriteDSP(SET_SINGLE_MODE) 
  WriteDSP(0)
  WriteDSP(0)
  while not BufferDone() do
  end while
  free_low(buffer)
end if
end procedure -- SetChannels()

global function ComputeTC(integer SampleRate, integer NbChannels)
-- compute time constant from sample rate and number of channels.
  return 256 - floor(1000000/(NbChannels*SampleRate))
end function -- ComputeTC()

global procedure PlayBuffer(atom buffer, integer size, 
integer TimeConstant, integer channels) 
--  PlayBuffer plays a sound buffer of a given size
--  The buffer must not cross a 64KB page boundary  
--  Only plays 8 bit mono single cycle mode
-- parameters: 
--   buffer -> low memory data buffer allocate with AllocateDmaBuffer()
--   size -> number of bytes in data buffer
--   TimeConstant -> a constant used by DSP = 256 - floor(1000000/SampleRate)

    
    if iSbCardError then
	return -- can't play if the sound card is in error
    end if

--  Set the playback time constant
    WriteDSP(SET_TIME_CONSTANT) 
    WriteDSP(TimeConstant) 
    SetDma(buffer,size,DmaChannel,DMA_OUT+DMA_SINGLE)
    
      if channels = MONO then
	WriteDSP(SET_SINGLE_MODE)  -- single cycle mode
	WriteDSP(remainder(size-1,256))
	WriteDSP(floor((size-1)/256))
      else    -- stereo play
	WriteDSP(SET_BLOCK_SIZE)
	WriteDSP(remainder(size-1,256))
	WriteDSP(floor((size-1)/256))
	WriteDSP(HSPEED_OUT)
      end if
end  procedure -- PlayBuffer()

---      following code not completed.

constant cOUTPUT = #41,
	 c16BITS_OUT = #B0,
	 cPCM16_MONO = #10,
	 cPCM16_STEREO = #30
	 

-- NOTE: 961109, This code is not working, to be debugged.
global procedure Play16bitsBuffer(atom buffer, integer size, integer SR,
				  integer channels)  
-- Play 16 bits sound buffer in single mode

    if DSPVer < 4 or iSbCardError or size < 2 then
	return -- can't play if the sound card is in error or no data or
	       -- 8 bits sound card.    
    end if
    SetDma(buffer,size,DmaChannel,DMA_OUT+DMA_SINGLE)
    size = floor(size/2)
    WriteDSP(cOUTPUT)  -- write sampling rate
    WriteDSP(floor(SR/256))
    WriteDSP(remainder(SR,256))
    WriteDSP(c16BITS_OUT)  
    if channels = MONO then
	 WriteDSP(cPCM16_MONO)
    else -- plays stereo    
	 WriteDSP(cPCM16_STEREO) 
    end if
    WriteDSP(remainder(size,256)) -- write data size.
    WriteDSP(floor(size/256))
end procedure -- Play16bitsBuffer()

if debug then
    ErrorLog(ENTER_INIT)
end if

InitDSP()
