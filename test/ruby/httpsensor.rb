# httpsensor.rb
#  -- sensing module for 24hours suit
#
# NISHI Takao <zophos@koka-in.org>
# $Id: httpsensor.rb,v 1.1 2005-02-18 22:57:30 jon Exp $

require 'thread'
require 'timeout'
require 'kconv'
require 'net/http'
require 'rssex'

include TwentyFourHours

class RssSensor
    TIMEOUT_SEC=30

    CACHE_EXPIRE=900
    CACHE_EXT='.24hdb'
    
    STAT_ERROR=-1
    STAT_UNKNOWN=0
    STAT_SENSING=1
    STAT_CACHE=2
    STAT_GOT=3

    @@cache_dir=nil

    def RssSensor.cache_dir=(x)
	@@cache_dir=x
    end

    def initialize(url,category,no_rss_link=false)
	@rss=nil
	@uri=URI::parse(url)
	@category=category.to_s.downcase
	@no_rss_link=no_rss_link

	@stat=STAT_UNKNOWN
	@http_response=nil
	@http_body=nil
	@open_timeout=TIMEOUT_SEC
	@read_timeout=TIMEOUT_SEC
	@now=Time.now
    end
    
    attr_reader :stat,:rss,:category
    
    def get(open_timeout=TIMEOUT_SEC,
	      read_timeout=TIMEOUT_SEC,
	      cache_expire=CACHE_EXPIRE)
	
	@stat=STAT_UNKNOWN
	@http_response=nil
	@http_body=nil

	begin
	    @rss=_read_cache
	    @stat=STAT_CACHE

	    if(@rss.extra_info['last-http-stat']=='ok' &&
	       @rss.extra_info['last-parse-stat']!='ng' &&
	       @now-@rss.extra_info['last_sense']<cache_expire)
		return self
	    end
	rescue StandardError,NameError
	end


	@open_timeout=open_timeout
	@read_timeout=read_timeout

	begin
	    (@http_response,@http_body)=_get
	rescue StandardError,NameError
	    STDERR<<"#{@uri.to_s}:get:#{$!}\n"
	end
	
	return self
    end

    def parse
	if(@stat==STAT_CACHE)
	    return @rss
	end

	begin
	    if(@http_response.nil?||@http_body.nil?)
		if(@rss.kind_of?(RSS::Channel))
		    @rss.extra_info['last_sense']=@now
		    @rss.extra_info['last-http-stat']='ng'
		end
	    else
		@rss=_parse(@now,@http_response,@http_body,&@block)
	    end
	rescue StandardError,NameError
	    @stat=STAT_ERROR
	    STDERR<<"#{@uri.to_s}:parse:#{$!}\n"
	    if(@rss.kind_of?(RSS::Channel))
		@rss.extra_info['last-parse-stat']='ng'
	    else
		return @rss
	    end
	end

	if(@rss.items.empty?)
	    STDERR<<"#{@uri.to_s}:parse:empty body\n"
	else
	    @rss.extra_info['no-rss-link']=@no_rss_link
	    begin
		_write_cache
	    rescue
	    end
	end

	return @rss
    end

    private
    def _get
	response=nil
	body=nil

	@stat=STAT_SENSING

	http=Net::HTTP.new(@uri.host,@uri.port)
	http.open_timeout=@open_timeout
	http.read_timeout=@read_timeout

	path=@uri.path
	unless(@uri.query.nil?)
	    path+='?'+@uri.query
	end

	3.times{|i|
	    begin
		(response,body)=http.get(path)
		@stat=STAT_GOT
		break
	    rescue Net::ProtoFatalError
		@stat=STAT_ERROR
		raise
	    rescue TimeoutError,StandardError
		if(i<2)
		    sleep(60)
		else
		    @stat=STAT_ERROR
		    raise
		end
	    rescue Exception
		@stat=STAT_ERROR
		raise
	    end
	}
	return [response,body]
    end

    def _parse(now,response,body,&block)
	begin
	    ch=RSS.parse(body)
	    ch.extra_info['last-parse-stat']='ok'
	    ch.extra_info['category']=@category
	    ch.extra_info['rss_src']=@uri.to_s
	    ch.extra_info['no-rss-link']=@no_rss_link
	    if(@rss.nil?)
		@rss=ch
	    else
		if(ch.items.empty?)
		    @rss.extra_info['last-parse-stat']='ng'
		else
		    @rss.update(ch)
		    @rss.extra_info['last-parse-stat']='ok'
		end
	    end
	rescue StandardError,NameError,ScriptError
	    @rss=RSS::Channel.new('','','','','')
	    @rss.extra_info['last-parse-stat']='ng'
	end
	
	@rss.extra_info['last-http-stat']='ok'
	@rss.extra_info['last_sense']=now

	@rss
    end
    
    def _read_cache
	buf=nil
	File::open(_cache_file){|f|
	    buf=Marshal::load(f)
	}
	
	return buf
    end

    def _write_cache
	unless(@rss.items.empty?)
	    File::open(_cache_file,"w"){|f|
		f.flock(File::LOCK_EX)
		Marshal::dump(@rss,f)
		f.flock(File::LOCK_UN)
	    }
	end
    end

    def _cache_file
	if(@@cache_dir.nil?)
	    return nil
	end
	
	cachefile=RSS::Channel.cachefile(@uri)

	return @@cache_dir.to_s+cachefile
    end
end

class HtmlSensor<RssSensor
    def initialize(url,category,title,
		   kcode,block_regexp,line_regexp,
		   no_rss_link=false,&block)
	super(url,category,no_rss_link)
	
	@title=title
	@kcode=kcode
	@block_regexp=block_regexp
	@line_regexp=line_regexp
	@block=block
    end

    private
    def _parse(now,response,body,&block)
	
	last_modified=now
	if(response.key?('Last-Modified'))
	    last_modified=ParseDate::parsedate(response['Last-Modified'])
	    if(last_modified[6]=='GMT')
		last_modified=Time::utc(*last_modified[0..5]).localtime
	    else
		last_modified=Time::local(*last_modified[0..5])
	    end
	end
	
	if(!@rss.nil? && last_modified<=@rss.syn_update_base)
	    @rss.extra_info['last_sense']=now
	    @rss.extra_info['last-http-stat']='ok'
	    @rss.extra_info['last-parse-stat']='cache'
	    return @rss
	end

	lang='ja'
	if(@kcode==Kconv::NOCONV)
	    lang='en'
	end
	ch=RSS::Channel.new(lang,
			    @uri.to_s,
			    @title,
			    @uri.to_s,
			    "summary of #{@title}, by 24hours")
	ch.dc_date=last_modified
	ch.dc_subject=@category
	#ch.syn_update_period='hourly'
	#ch.syn_update_base=now
	ch.extra_info['category']=@category
	ch.extra_info['last_sense']=now

	ch.extra_info['last-http-stat']='ok'

	
	body=Kconv.kconv(body,Kconv::EUC,@kcode).gsub(/[\r\n]/,'').strip
	unless(@block_regexp.nil?)
	    body=~@block_regexp
	    body=$&.to_s
	end
	if(body.empty?)
	    @rss.extra_info['last-parse-stat']='ng'
	    return @rss
	end

	body.scan(@line_regexp){|line|
	    src=nil
	    title=nil
	    
	    if(block.nil?)
		src=$1
		title=$2
	    else
		(src,title)=yield(line)
	    end
	    
	    unless(src.nil?)
		src=(@uri+URI.parse(src)).to_s
		title=title.to_s.strip
		title.gsub!(/<.+?>/,'')
		if(title.empty?)
		    title=src
		end

		i=RSS::Item.new(src,title,src)
		i.dc_date=last_modified

		ch.push(i)
	    end
	}

	ch.iconv('utf-8','euc-jp')

	if(@rss.nil?)
	    @rss=ch
	else
	    if(ch.items.empty?)
		@rss.extra_info['last-parse-stat']='ng'
	    else
		@rss.update(ch)
		@rss.extra_info['last-parse-stat']='ok'
	    end
	end

	last_modified=Time.at(0)
	@rss.each_item{|i|
	    if(last_modified<i.dc_date)
		last_modified=i.dc_date
	    end
	}
	@rss.dc_date=last_modified
	#@rss.syn_update_period='hourly'
	#@rss.syn_update_base=now

	@rss
    end
end

class Sensor
    PARSE_TIMEOUT=300

    @@max_thread_num=5
    def Sensor.max_thread_num=(x)
	@@max_thread_num=x
    end

    def initialize
	@jobs=Queue.new
    end

    def push(x)
	@jobs.push(x)
    end

    def start
	job_num=@jobs.size
	parse_queue=Queue.new
	
	th=[]
	@@max_thread_num.times{|i|
	    th[i]=Thread.new{
		loop{
		    begin
			src=@jobs.pop(true)
			parse_queue.push(src.get)
		    rescue ThreadError
			break
		    rescue Exception
			parse_queue.push(nil)
		    end
		}
	    }
	}

	buf=[]
	job_num.times{|x|
	    begin
		timeout(PARSE_TIMEOUT){
		    buf.push(parse_queue.pop.parse)
		}
	    rescue TimeoutError
		STDERR<<"sensor:parse:#{$!}\n"
		break
	    rescue NameError
	    end
	}
	buf

    end
end
