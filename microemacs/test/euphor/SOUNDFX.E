 
-- sound effect for game using sound blaster card.
-- by Jacques Deschenes, Baie-Comeau, PQ, Canada, e-mail: desja@quebectel.com
-- creation date: Nov 2th, 1996
-- revision history
-- Jan. 5th, 1997,  added global function SoundDone() 
 
include file.e
include sbcard.e
include doswrap.e
 
-----------------------------------------------------------------------------
 
 
global function LoadSoundEffects(sequence Files)
-- input: list of files to load in buffers
-- output: return a sequence containning all the sound buffers ready to play.
 
integer FHandle, DataOfs, junk
sequence WaveInfo, AllBuffers, SoundInfo
 
   AllBuffers = repeat({},length(Files))
   SoundInfo = repeat(0,4)
   --clear_screen()
   --puts(1,"Loading sound files:\n")
   for  i = 1 to length(Files)  do  -- load files loop
      FHandle = open(Files[i],"rb")
      if FHandle = -1 then
	 puts(1,"File "&Files[i]&" not found.\n")
      else
	--puts(1,Files[i]&"\n")
	WaveInfo = GetWavInfo(FHandle)
	DataOfs = where(FHandle)
	close(FHandle)
	SoundInfo[2] = WaveInfo[siDATA_LEN]
	if SoundInfo[2] > 65536 then
	  SoundInfo[2] = 65536 -- max buffer size
	end if
	SoundInfo[1] = AllocateDMABuffer(SoundInfo[2])-- allocate buffer needed size
	if SoundInfo[1] > 0 then  -- load only if buffer allocated.
	  FHandle = DosOpen(Files[i],READ)
	  if DosSeek(FHandle,DataOfs,0) then
	    SoundInfo[2] = BlockRead(FHandle,SoundInfo[1],SoundInfo[2])
	  end if
	end if
	junk = DosClose(FHandle)
	SoundInfo[3] = ComputeTC(WaveInfo[siSPS],WaveInfo[siCHANNELS])
	SoundInfo[4] = WaveInfo[siCHANNELS]
	close(FHandle)
	AllBuffers[i] = SoundInfo
      end if
   end for 
    return AllBuffers
end function -- LoadSoundEffects()
 
 
integer SoundPending
 
SoundPending = 0
 
-------------------------------------------------------------------------------
 
global function SoundDone()
-- if a sound is pending, check if done. If so return 1 
-- if not done return 0
    if not SoundPending then
	return 1
    end if
    if BufferDone() then
	SoundPending = 0
	return 1
    end if
    return 0
end function -- SoundDone()
-------------------------------------------------------------------------------
 
global procedure PlaySoundEffect(sequence SoundEffect)
-- play only 8 bits wave data
-- input: SoundEffect is a sequence containting sound effect data.
  
  if length(SoundEffect) != 4 then
      return -- not valid sequence
  end if
  if not SoundDone() then  -- if a sound  already playing
     if not ResetDSP() then  -- stop it by resetting sound blaster
       return
     end if
  end if
  SetVolume(MASTER_VOLUME,VOL_MAX)
  SetVolume(VOICE_VOLUME,4)
  SpeakerOn()
  SetChannels(SoundEffect[4]) -- set number of channels mono or stereo
  PlayBuffer(SoundEffect[1],  -- dma buffer address
	   SoundEffect[2],    -- data size
	   SoundEffect[3],    -- time constant
	   SoundEffect[4])    -- number of channels
  SoundPending = 1
end procedure -- PlaySoundEffect()
 
