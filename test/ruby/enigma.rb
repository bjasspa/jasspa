#!/usr/bin/ruby
# --------------------------------------------------------------------------------
#
# Ruby Enigma                                          v.:0.0.5 by <mcc>
# ================================================================================
#                                                      2004-08-22
#
#
# ___ REQUIREMENTS _______________________________________________________________
require 'pp'
require 'getoptlong'



# ___CLASSES______________________________________________________________________
# ................................................................................
# ................................................................................
# ................................................................................
# ................................................................................
# ................................................................................


# ...WHEEL........................................................................
#
# creates an Object representing a wheel of
# the enigma. All kinds of wheels -- even
# static (not rotating) ones are covered
# by the functionality of this class --
# UKWs and ETWs are included
#
class Wheel
  
  attr_reader :carry, :letter

  def initialize( wiring, grundstellung , ringstellung, can_rotate, turnoverpoints  )
    
    @letter = ""

    #
    # encoding schema
    #
    @iwiring=wiring

    #
    # inverted encoding schema 
    #
    @iwiring.each_index{ |idx| @owiring[@iwiring[idx]]=idx }

    #
    # size of the code
    #
    @codesize=@iwiring.size

    @grundstellung=grundstellung
    @ringstellung=ringstellung
    @can_rotate=can_rotate

    #
    # turn wheel into the initial position
    # and check for the turnoverpoint in
    # initial position
    #
    @position=@grundstellung
    @carry = @turnoverpoint[0] == nil ? false : @turnoverpoint.include?(@position)      

  end


  #
  # encode a given letter
  # direction: 0 -> direction from the EKW to the UKW
  #            1 -> direction from the UKW back to the EKW
  #
  def encode( letter,direction )
    @letter = (@position - @ringstellung + code) % @codesize
    if( direction == 0 )
      @letter = ( @iwiring[@letter] - @position + @ringstellung ) % @codesize
    else
      @letter = ( @owiring[@letter] - @position + @ringstellung ) % @codesize
    end
  end

  # 
  # rotate wheel to next position
  # ignored if wheel is not rotateable (EKW,some UKWs)
  #
  def rotate
    if @can_rotate 
      @position=(@position + 1) % @codesize  
    end
    @carry = @turnoverpoint[0] == nil ? false : @turnoverpoint.include?(@position)      
  end

end

# ...HARDWARE.....................................................................
#
# creates an Object representing the complete 
# hardware of a certain model of the Enigma
# it does include ALL hardware...even unused
# parts! One can choose what parts should go
# into its personal Enigma configuration via
# commandline options. 
#
class Hardware

  attr_reader :parts

  def initialize( model )

    #
    # setup of an hash containing all possible parts
    # of one model of the Enigma
    #
    @parts=Hash.new

    #
    # name of the model of the Enigma, the parts of hardware belongs to
    #
    @parts["model"]            = ""
    
    #
    # wiring of all wheels available for a certain Enigma model
    #
    @parts["rotdata"]          = Array.new # wiring of the ordinary Rotors 
    rotdataidx                 = 0
    
    ## @parts["greekdata"]        = Array.new # wiring of the so called Greek  Rotors 
    ## greekdataidx               = 0
    

    #
    # wiring of the UKWs available for a certain Enigma model
    #
    @parts["ukwdata"]          = Array.new # wiring of the Umkehrwalze 
    ukwdataidx                 = 0

    #
    # turnoverpoints of all wheels if they have some
    #
    @parts["turnoverpoint"]    = Array.new # turnover point settings
    turnoverpointidx  = 0
    
    #
    # wiring of the EKWs available for a certain Enigma model
    #
    @parts["etwdata"]          = Array.new # wiring of the Eintrittswalze
    etwdataidx                 = 0
    

    #
    # some specific properties
    # ........................................
    #
    # ETW differs from default ETW
    #
    @parts["etw_diff"]         = 0

    #
    # steckerboard is available
    #
    @parts["hassteckers"]      = 0

    #
    # Greek wheel is available
    #
    @parts["greek_wal"]        = 0

    #
    # UKW has ringsettings
    #
    @parts["ukw_ring"]         = 0

    #
    # UKW is settable by the user
    #
    @parts["ukw_set"]          = 0

    #
    # specific Enigma model "suffers" 
    # from double stepping "feature"
    #
    @parts["double_step"]      = 0

    #
    # UKW steps
    #
    @parts["ukw_step"]         = 0

    #
    # for future expansions
    #
    @parts["machine_mess"]     = 4  # currently unsused
    @parts["machine_ring"]     = 4  # currently unsused
    
    
    tmparray=Array.new
    
    readdef  = false
    curmodel = ""
    
    fi = File.open( "enigmas.cnf","r" )
    fi.each{ |line|
      # skip pure comment lines
      next if line =~ /^#/
      
      # if wanted model reached, read definition of it
      if( readdef )
        
        # if read next model definition: stop
        # example: MODEL=M4
        if line =~ /^MODEL=/
          break;
        end
        
        # found Walzendefinition
        # from file: letters
        # example:
        # WAL_VII=RCSPBLKQAUMHWYTIFZVGOJNEXD
        if( line =~ /WAL_([BGIV]+)=([A-Z]+)/ )
          case $1
            when "I"    idx=1
            when "II"   idx=2
            when "III"  idx=3
            when "IV"   idx=4
            when "V"    idx=5
            when "VI"   idx=6
            when "VII"  idx=7
            when "VIII" idx=8
            when "B"    idx=9
            when "G"    idx=10
          end
          @parts["rotdata"][idx]=$2.split(//).collect{ |entry| entry[0] - ?A }
          next
        end

# old version       
#        if( line =~ /WAL_[BGIV]+=([A-Z]+)/ )
#          @parts["rotdata"].push($1.split(//).collect{ |entry| entry[0] - ?A })
#          next
#        end
        
        #
        # found Umkehrwalzendefinition
        # from file: letters
        # example:
        # UKW_B=ENKQAUYWJICOPBLMDXZVFTHRGS
        #
        if( line =~ /UKW(_A|_B|_C)*=([A-Z]+)/ )
          # if default UKW
          if $1 == nil
            idx=0
          else
            idx=$1[1] -?A
          end
          @parts["ukwdata"][idx]=*$2.split(//).collect{ |entry| entry[0] - ?A }
          next
        end
        
        #
        # found Eintrittswalzendefinition
        # from file: letters
        # exampls
        # ETW=QWERTZUIOASDFGHJKPYXCVBNML
        #
        if( line =~ /ETW=([A-Z]+)/ )
          @parts["etwdata"].push($1.split(//).collect{ |entry| entry[0] - ?A })
        end
        
        #
        # found turnover point definitions
        # from file: numbers 0-25
        # example
        # WAL_VI_TURN=( 12 25 )
        #
        if( line =~ /WAL_([IV]+)_TURN=\( ([0-9 ]+)\)/ )
#         @parts["turnoverpoint"].push($2.split(/ /).collect{ |entry|  entry.to_i + 1 })
          case $1
            when "I"    idx=1
            when "II"   idx=2
            when "III"  idx=3
            when "IV"   idx=4
            when "V"    idx=5
            when "VI"   idx=6
            when "VII"  idx=7
            when "VIII" idx=8
            when "B"    idx=9
            when "G"    idx=10
          end
          @parts["turnoverpoint"][idx] = $2.split(/ /).collect{ |entry|  entry.to_i }
        end
        
        #
        # has different configuration of the Eintrittswalze
        # from file: number
        # example: ETW_DIFF=0
        #
        if( line =~ /ETW_DIFF=([0-9])/ )
          @parts["etw_diff"]    = $1[0]-?0
          next 
        end
        
        #
        # supports steckers
        # from file: number
        # example: STECKERS=1
        #
        if( line =~ /STECKERS=([0-9])/ )
          @parts["hassteckers"] = $1[0]-?0
          next  
        end
        
        #
        # supports fourth wheel (Griechenwalze)
        # from file:  number
        # example: GREEK_WAL=0
        #
        if( line =~ /GREEK_WAL=([0-9])/ )
          @parts["greek_wal"]   = $1[0]-?0
          next  
        end
        
        #
        # supports settable ring of the Umkehrwalze
        # from file:  number
        # example: UKW_RING=0
        #
        if( line =~ /UKW_RING=([0-9])/ )
          @parts["ukw_ring"]    = $1[0]-?0
          next  
        end
        
        #
        # supports setting of the Umkehrwalze
        # from file:  number
        # example: UKW_SET=0
        #
        if( line =~ /UKW_SET=([0-9])/ )
          @parts["ukw_set"]     = $1[0]-?0
          next  
        end
        
        #
        # supports double stepping
        # from file:  number
        # example: DOUBLE_STEP=1
        #
        if( line =~ /DOUBLE_STEP=([0-9])/ )
          @parts["double_step"] = $1[0]-?0
          next  
        end
        
        #
        # supports stepping of the Umkehrwalze
        # from file:  number
        # example: UKW_STEP=0
        #
        if( line =~ /UKW_STEP=([0-9])/ )
          @parts["ukw_step"]    = $1[0]-?0
          next  
        end

      else # wanted model not found already
        
        #
        # check for the model definition
        # from file: string
        # example: MODEL=M4
        #
        line =~ /^MODEL=(.*)[ ]*$/
        if $1 =~ /#{model}/
          @parts["modelname"]=$1
          readdef = true 
          next
        end
        
      end
    }

    #
    # close hardware definition file
    #
    fi.close
    
    # For Enigmas without a different ETW wiring, create a default wiring
    if @parts["etw_diff"]=0
      @parts["etwdata"].push("ABCDEFGHJIJKLMNOPQRSTUVWXY".split(//).collect{ |entry| entry[0] - ?A })
    end
  end


  #
  # show setup of choosen Enigma model
  #
  def show_setup
    print "Setup for Enigma model: ",@modelname, "\n"
    print "=============================================================\n\n"
    print "This model...\n"
    print "* Has a Steckerboard\n" if( @hassteckers == 1)
    print "* Has a different Eintrittswalzen wiring\n" if( @etw_diff == 1)
    print "* Allows setting of the Umkehrwalze\n" if( @ukw_set == 1 )
    print "* Supports stepping of the Umkehrwalze\n" if( @ukw_step == 1 )
    print "* Allows setting of the Umkehrwalzenring\n" if( @ukw_ring == 1 )
    print "* Supports double stepping\n" if( @double_step == 1 )
    print "\n"
    print "This Enigma model has ", @rotdata.size + @greekdata.size , " wheels to choose from.\n"
    print "It helds 3 ordinary wheels"
    if( @greek_val == 1)
      print " and ", @greekdata.size, " greek wheels."
    else
      print "."
    end
    print "\n"
    if( @etw_diff == 1 )
      print "\nThe different wiring of the Eintrittwalze is:\n"
      @etwdata.each{ |rot| rot.each{ |val| print((val + ?A).chr," ") }; print "\n" }
    end
    print "\nThe setup of the ordinary wheels:\n"
    @rotdata.each{ |rot| rot.each{ |val| print((val + ?A).chr," ") } ; print "\n" }
    print "\n"
    if( @greek_val == 1)
      print "\nThe setup of the greek wheels:\n"
      @greekdata.each{ |rot| rot.each{ |val| print((val + ?A).chr," ") } ; print "\n" }
    end
    print "\nThe turnpover points are:\n"
    @turnoverpoint.each{ |rot| rot.each{ |val| print((val + ?A).chr," ") } ; print "\n" }
  end
  
end # Hardware

# ...STECKERBOARD.................................................................
#
# Creates an object representing the steckerboard.   
# All not steckkered letters are selfsteckered by 
# a spring load mechanism. What letteres get steckered
# can be choosen by the user. There are zero up to
# thirteen steckered i.e. swapped letters
#
class Steckerboard

  def initialize( steckers )
    @istecker=Array.new
    @ostecker=Array.new
    if( stecker != nil )      # user has used stecks/patch cables
      if( stecker.size > 13 ) # max. 13 pairs of letter can be interchanged 
        print "Too many steckers used!\n"
        exit 1
      end
    else                      # selfsteckered: no patch cable in use 
      steckers={              # this is the default selfsteckered
        "A" => "A" ,          # configuration (two-way!)
                "B" => "B" ,
                "C" => "C" ,
                "E" => "E" ,
                "F" => "F" ,
                "G" => "G" ,
                "H" => "H" ,
                "I" => "I" ,
                "J" => "J" ,
                "K" => "K" ,
                "L" => "L" ,
                "M" => "M" ,
                "O" => "O" ,
                "P" => "P" ,
                "Q" => "Q" ,
                "R" => "R" ,
                "S" => "S" ,
                "T" => "T" ,
                "U" => "U" ,
                "V" => "V" ,
                "W" => "W" ,
                "X" => "X" ,
                "Y" => "Y" ,
                "Z" => "Z"
      }
    end

    #
    # remap steckers into two arrays used
    # for both ways of encoding each
    #
    steckers.each{ |key,value|
      key   = key[0]   - ?A
      value = value[0] - ?A
      @istecker[key]=value
      @ostecker[value]=key
    }
  end

  #
  # do the encode !
  #
  def encode( code )
    if( code.to_i != code )
      # way in
      @istecker[code]
    else
      # way out
      @ostecker[code]
    end
  end

end

# ...LAMPBOARD....................................................................
#                                                                                 
# Creates an object representing the lampboard.
# Of course, there are no real lamps here in this 
# simulator, but we finally need letters to be shown
# so this is good opportunity for mapping and formatting
# (file or stdout)
#
class Lampboard

  def initialize( format, output )
    @format = format
    @output = output
    @alpha="ABCDEFGHIJKLMNOPQRSTUVWXYZ".split(//)
  end

  def show( code )
    code.each{ |letter|
      printf( output, "", @alpha[letter] )
    }
  end

end

# ...KEYBOARD....................................................................
#                                                                                 
# Creates an object representing the keyboard.
# There is no keyboard in sense of the real Enigma, but
# there is one to simulate: It represents the input
# (file or stdin)
#
class Keyboard

  def initialize( transmode, input )
    @transmode = transmode
    @input = input
  end

  def show( input )
    input.tr!( 'abcdefghijklmnopqrstuvwxyz ','ABCDEFGHIJKLMNOPQRSTUVWXYZX' ) if @transmode
    

    code.each{ |letter|
      printf( output, "", @alpha[letter] )
    }
  end

end



# ...MACHINE......................................................................
#
# Creates an object representing the actual model of 
# the Enigma according to hardware layout and user
# preferences and her/his choose of parts to be 
# put into her/his personal Enigma
#
class Machine

  attr_reader :myenigma

  @myenigma=Hash.new
  @myenigma["wheels"]               = Array.new
  @myenigma["steckerboard"]         = Array.new

  def initialize( useroptions )

    @myenigma             = Hash.new
    @myenigma["wheels"]   = Array.new

    @oldcarry  = [false,false,false,true]
    @curcarry  = [false,false,false,true]
    @nextcarry = [false,false,false,true]

    #
    # get all possible hardware from the
    # the hardware definition file for
    # the wanted mode of the Enigma
    #
    @hardware=Hardware.new(useroptions["model"])

    # 
    # look whether this a Enigma model "suffering"
    # from the "double steopping feature" (see below)
    #
    @doublestepper = @hardware.parts["double_step"] == 1

    
    # ......................................................................
    # put all wheels onto the spindle
    # max. configuration (Enigma M4) would be:
    # UKW,GRIECHENWALZE,WHEEL_A,WHEEL_B,WHEEL_C,ETW
    #

    # Handling of the UKW('s)
    # if there are more than one Umkehrwalze
    # available -- get user choice of the specific
    # Umkehrwalze. Take the default one
    # else.
    #
    # Get the basics of the UKW
    grundstellung=useroptions["grund"].shift
    ringstellung=useroptions["rings"].shift
    theukw=useroptions["ukwheel"].shift
    theukw = theukw == nil ? 0 : theukw ;
    ukwdata = @hardware.parts["ukwdata"][theukw] 
    rotates = @hardware.parts["ukw_step"] == 1
    turnoverpoints = nil # ukws doesnt have them

    #
    # put left most wheel onto spindel: the Umkehrwalze
    #
    @myenigma["wheels"].push(Wheel.new(ukwdata,grundstellung,ringstellung,rotates,turnoverpoints))

    #
    # next are the "ordinary" wheels...
    # first the Griechenwalze, if any
    #
    if @hardware.parts["greek_wal"] == 1
      grundstellung=useroptions["grund"].shift
      ringstellung=useroptinos["rings"].shift
      thegreekw=useroptions["wheels"].shift
      greekdata = @hardware.parts["greekdata"][thegreekw] 
      rotates = false # greek wheel doesnt step
      turnoverpoints = nil # greek wheel does not step so no turnoverpoints
      # put Griechenwalze onto spindel
      @myenigma["wheels"].push(Wheel.new(greekdata,grundstellung,ringstellung,rotates,turnoverpoints))
    end

    #
    # now the remaining three wheels
    #
    3.times{ |idx|
      grundstellung=useroptions["grund"].shift
      ringstellung=useroptinos["rings"].shift
      thewheel=useroptions["wheels"].shift
      rotdata = @hardware.parts["greekdata"][thewheel] 
      rotates = true 
      turnoverpoints = @hardware.parts["turnoverpoints"][idx]
      # put wheel onto spindel
      @myenigma["wheels"].push(Wheel.new(rotdata,grundstellung,ringstellung,rotates,turnoverpoints))
    }

    #
    # at last the Eintrittswalze
    #
    grundstellung=0
    ringstellung=0
    etwdata=@hardware.parts["etwdata"][0]
    rotates = false
    turnoverpoints = nil
    # put wheel onto spindel
    @myenigma["wheels"].push(Wheel.new(rotdata,grundstellung,ringstellung,rotates,turnoverpoints))

    #
    # ......................................................................

    # 
    #
    # Setup the Steckerboard
    #
    steckerboard=Steckerboard.new( useroptions["steckers"] )

    #
    # Setup Keyboard
    #
    keyboard = Keyboard.new( useroptions["transmode"],useroptions["ichannel"] )

  end

  def encode

    #
    # get letter to encode
    #
    letter=keyboard.get


    #
    # First the wheels are rotated,
    # _then_ the letter is encoded
    #
    @myenigma["wheels"].size.reverse_each_with_index{ |wheel,idx|
      #
      # if previous wheel had reached its turnoverpoint
      # (or in case of the right most wheel: in any
      # case) rotate wheel
      #
      if @curcarry[idx]
        @nextcarry[idx+1]=wheel.rotate
        #
        # middle wheel is special
        #
        if @doublestepper
          if idx == 1 and @curcarry[idx] == true 
            @nextcarry[idx] == true
          end
        end
      else
        break # no further rotation of the wheels
      end
    }
    

    @encoding=Array.new
    
    #
    # next is the encoding process 
    #
    @encoding.push(letter)
    letter=steckerboard.encode(letter)
    @encoding.push(letter)
    @myenigma["wheels"].size.reverse_each_with_ind{ |wheel,idx|
      letter=wheel.encode(letter)
      @encoding.push(letter)
    }

    @myenigma["wheels"].size.each_with_index{ |wheel,idx|
      #
      # skip UKW: only used once for the encoding process
      #
      next if idx == 0
      letter=wheel.encode(letter)
      @encoding.push(letter)
    }
  
    letter=steckerboard.encode(letter)
    @encoding.push(letter)

    @encoding.each{ |letter| printf( "%c ->",  letter ) }
    print "\n"
    @encoding.clear
  end


end # Machine

# ...OPTIONS......................................................................
#
# creates an Object representing all settings
# made by the user via options on the commandline
#
class Options

  attr_reader :useroptions
  
  def initialize

    #
    # set default options
    #
    @useroptions=Hash.new
    @useroptions["help"]=false
    @useroptions["version"]=false
    @useroptions["model"]="M4"

    @useroptions["grund"]=Array.new
    @useroptions["rings"]=Array.new
    @useroptions["ukwheel"]=Array.new
    @useroptions["steckers"]=Array.new
    #
    #  initialise commandline options
    #
    opts = GetoptLong.new(
                          [ "--help",         "-h",  GetoptLong::NO_ARGUMENT ],
                          [ "--version",      "-v",  GetoptLong::NO_ARGUMENT ],
                          [ "--model",        "-m",  GetoptLong::REQUIRED_ARGUMENT ],
                          [ "--wheels",       "-w",  GetoptLong::REQUIRED_ARGUMENT ],
                          [ "--rings",        "-r",  GetoptLong::REQUIRED_ARGUMENT ],
                          [ "--grund",        "-g",  GetoptLong::REQUIRED_ARGUMENT ],
                          [ "--ukw",          "-u",  GetoptLong::REQUIRED_ARGUMENT ],
                          [ "--steckers",     "-s",  GetoptLong::REQUIRED_ARGUMENT ],
                          [ "--input",        "-i",  GetoptLong::REQUIRED_ARGUMENT ],
                          [ "--output",       "-o",  GetoptLong::REQUIRED_ARGUMENT ],
                          [ "--config",       "-c",  GetoptLong::REQUIRED_ARGUMENT ]
                          )

    #
    # read options given by the user
    #
    opts.each do |opt, arg|
      case opt
      when "--help","-h"
        @useroptions["help"]=true
      when "--version","-v"
        @useroptions["version"]=true
      when "--model","-m"
        @useroptions["model"]=arg
      when "--wheels","-w"
        @useroptions["wheels"]=*tr_wheels(arg)
      when "--rings","-r"
        @useroptions["rings"]=*tr_rings(arg)
      when "--grund","-g"
        @useroptions["grund"]=*tr_grund(arg)
      when "--ukw","-u"
        @useroptions["ukwheel"]=*tr_ukw(arg)
      when "--steckers","-s"
        @useroptions["steckers"]=*tr_steckers(arg)
      when "--input","-i"
        @useroptions["ichannel"]=set_ichannel(arg)
      when "--output","-o"
        @useroptions["ochannel"]=set_ochannel(arg)
      when "--config","-c"
        @useroptions=set_config(arg)
      end # case
    end # opts.each


  end #initialize

  private

  def tr_wheels( wheels )
    wheels.split(//).each{ |val|
      val = "9" if( val == "B" )       # first Griechenwalze of the M4, nineth wheel
      val = "10" if( val == "C" )      # second Griechenwalze of the M4, tenth wheel
      val = "10" if( val == "G" )      # second Griechenwalze of the M4, tenth wheel, other naming
      val.to_i
    }
  end

  def tr_rings( rings )
    rings.split(//).collect{ |ring| ring[0] - ?A }
  end
  
  def tr_grund( grund )
    grund.split(//).collect{ |ring| ring[0] - ?A }    
  end

  def tr_ukw( ukwalze )
    ukwalze.split(//).collect{ |walze| walze[0] - ?A }
  end

  def tr_steckers( steckers )
    quseroptions["steckers"]=Hash.new[*stecker.split(//)]
  end

  def tr_ichannel(arg)
    #
    # if not STDIN
    #
    if arg.tty? == false
      File.new( arg,"r" )
    else
      arg
    end
  end

  def tr_ochannel(arg)
    #
    # if not STDOUT
    #
    if arg.tty? == false
      File.new( arg,"w" )
    else
      arg
    end
  end

end # Options

# ...ENIGMA.......................................................................
#
# Entry point
#
class Enigma


  def initialize
    

    #
    # get user settings from commandline
    #
    opts=Options.new
    @useroptions=opts.useroptions

    #
    # help is wanted
    #
    if @useroptions["help"] 
      help
    end

    #
    # print version of script
    #
    if @useroptions["version"]
      version
    end
    
    #
    # options which do not really use
    # the script terminate script
    # gracefully
    #
    if @version || @help
      exit 0
    end

  end

  def encode
    machine.encode
  end


  def reset
    machine.reset
  end

  def help
  end

  def version
  end


end # Enigma

# ................................................................................
#
# Let's rock!
#
Enigma.new

# Local Variables:
# mode:ruby
# comment-colum:40
# comment-start: "#"
