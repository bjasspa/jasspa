# 24hconf.rb
#   -- config parser for 24hours suit
#
# NISHI Takao <zophos@koka-in.org>
# $Id: 24hconf.rb,v 1.1 2005-02-18 22:57:29 jon Exp $

module TwentyFourHours

    class Config<Array
	class Section<Hash
	    def initialize(name)
		super(nil)
		@name=name
	    end
	    attr_reader :name

            def to_s
                buf="=begin #{@name}\n"
                self.each{|k,v|
                    buf+="==#{k}"
                    if(v.include?("\n"))
                        buf+="<<_EOB_\n#{v}_EOB_\n"
                    else
                        buf+=" #{v}\n"
                    end
                }
                buf+="=end\n"

                buf
            end
	end
	
	def initialize
	    super
	end
	
	def start_section(name)
	    self.push(Section.new(name.strip.downcase))
	end
	
	def set_item(key,value)
	    self.last[key.strip.downcase]=value
	end
	
        def to_s
            self.join("\n")
        end

        def section(name)
            self.map{|sec|
                if(sec.name==name)
                    sec
                else
                    nil
                end
            }.compact
        end

	def Config.parse(file)
	    conf=Config.new
	    
	    section=nil
	    item=nil
	    
	    isheredoc=false
	    _endofheredoc=nil
	    
	    File.foreach(file){|line|
		if(isheredoc)
		    if(line.chomp==_endofheredoc)
			isheredoc=false
		    else
			conf.last[item]+=line.to_s
		    end
		else
		    if(line.strip.empty?||line=~/^#.*$/)
			#continue
		    elsif(line=~/^=begin\s+(.+)$/)
			section=$1.chomp
			conf.start_section(section)
		    elsif(line=~/^==(.+)/)
			(item,body)=$1.split(/\s+/,2)
			
			(item,_endofheredoc)=item.split('<<',2)
			
			conf.set_item(item,body.to_s)
			unless(_endofheredoc.nil?)
			    isheredoc=true
			end
		    end
		end
	    }
	    
	    return conf
	end
    end
end
