# rss.rb 
#  -- rss module for 24hours suit
#
# NISHI Takao <zophos@koka-in.org>
# $Id: rss.rb,v 1.1 2005-02-18 22:57:31 jon Exp $

require 'iconv'
require 'rexml/document'
require 'parsedate'

module TwentyFourHours
    module RSS
	def RSS.parse(str)
	    xml=REXML::Document.new(str)
	    root=xml.root
            case root.name
	    when 'RDF'
                Channel.parse(root,xml.encoding)
            when 'rss'
                Channel.parse09(root,xml.encoding)
            else
                raise
	    end
	    
	end
	
	def RSS.parsedate(date)
	    d=ParseDate.parsedate(date)
	    d.pop
	    tz=d.pop
	    if(tz=~/([+\-])(\d\d)?:?(\d\d)?/)
		tz=$2.to_i*3600+$3.to_i*60
		if($1=='+')
		    tz=0-tz
		end
	    else
		tz=0
	    end
	    Time.utc(*d)+tz
	end
	
	class Item
	    def initialize(about,title,link)
		@about=about
		@title=title
		@link=link
		@about=@link if @about.nil?

		@description=''
		@dc_subject=''
		@dc_author=''
		@dc_date=Time.now
		
		@extra_info={}
	    end
	    attr_reader :about,:title,:link,:extra_info
	    attr_accessor :description,:dc_subject,:dc_date,:dc_author

	    def iconv(to,from='utf-8')
		(@title,@description,@dc_subject,@dc_author)=
		    Iconv.iconv(to,from,
				@title,@description,@dc_subject,@dc_author)
		self
	    end
	    
	    def to_s(encode='utf-8')
		buf=self.to_xml.to_s
		
		if(encode.downcase!='utf-8')
		    Iconv.iconv(encode,'utf-8',buf).first
		else
		    buf
		end
	    end
	    
	    def to_xml
		item=REXML::Element.new('item')
		item.attributes['rdf:about']=@about
		item.add_element('title').text=@title
		item.add_element('link').text=@link
		unless(@description.to_s.empty?)
		    item.add_element('description').text=@description
		end
		unless(@dc_subject.to_s.empty?)
		    item.add_element('dc:subject').text=@dc_subject
		end
		unless(@dc_author.to_s.empty?)
		    item.add_element('dc:author').text=@dc_author
		end
		item.add_element('dc:date').text=
		    @dc_date.utc.strftime("%Y-%m-%dT%H:%M:%S+00:00")
		
		item
	    end
	    
	    def Item.parse(xml,encoding='utf-8')
		about=xml.attributes['about']
		title=xml.elements.to_a('title').first.text
		link=xml.elements.to_a('link').first.text
		
		ret=Item.new(about,title,link)
		
		description=xml.elements.to_a('description').first
		dc_subject=xml.elements.to_a('dc:subject').first
		dc_author=xml.elements.to_a('dc:author').first
		dc_date=xml.elements.to_a('dc:date').first
		
		unless(description.nil?)
		    ret.description=description.text
		end
		unless(dc_subject.nil?)
		    ret.dc_subject=dc_subject.text
		end
		unless(dc_author.nil?)
		    ret.dc_author=dc_author.text
		end
		unless(dc_date.nil?)
		    begin
			ret.dc_date=RSS::parsedate(dc_date.text)
		    rescue
		    end
		end
		
		ret
	    end

            def Item.parse09(xml,encoding='utf-8')
		title=xml.elements.to_a('title').first.text
		link=xml.elements.to_a('link').first.text
		about=link

		ret=Item.new(about,title,link)
		
		description=xml.elements.to_a('description').first
		unless(description.nil?)
		    ret.description=description.text
		end
		
		ret
	    end
	end
	
	class Image
	    def initialize(about,title,url,link)
		@about=about
		@url=url
		@title=title
		@link=link
	    end
	    attr_reader :about,:url,:title,:link
	    
	    def iconv(to,from='utf-8')
		@title=Iconv.iconv(to,from,@title).first
		self
	    end
	    
	    def to_xml
		img=REXML::Element.new('image')
		img.attributes['rdf:about']=@about
		img.add_element('title').text=@title
		img.add_element('url').text=@url
		img.add_element('link').text=@link
		
		img
	    end
	    
	    def to_s(encode='utf-8')
		buf=self.to_xml.to_s
		
		if(encode.downcase!='utf-8')
		    Iconv.iconv(encode,'utf-8',buf).first
		else
		    buf
		end
	    end
	    
	    
	    def Image.parse(xml,encoding='utf-8')
		about=xml.attributes['about']
		title=xml.elements.to_a('title').first.text
		url=xml.elements.to_a('url').first.text
		link=xml.elements.to_a('link').first.text
		
		ret=Image.new(about,title,url,link)
		
		ret
	    end
	end
	
	class Channel
	    def initialize(lang,about,title,link,description)
		@lang=lang
		@about=about
		@title=title
		@link=link
		@description=description
		@about=@link if @about.nil?

		@image=nil
		@dc_language=@lang
		@dc_date=Time.now
		@dc_subject=''
		@dc_rights=''
		@dc_publisher=''
		@dc_creator=''
		@syn_update_period='daily'
		@syn_update_frequency=1
		@syn_update_base=@dc_date
		@seq=[]
		@items=[]
		
		@extra_info={}
	    end
	    attr_reader :lang,:about,:title,:link,:description,:image,
		:dc_date,:extra_info
	    attr_accessor :dc_language,:dc_subject,:dc_rights,
		:dc_publisher,:dc_creator,
		:syn_update_period,:syn_update_frequency,:syn_update_base
	    
	    def image=(img)
		unless(img.kind_of?(Image))
		    raise TypeError,"Invalid image set"
		end
		
		@image=img
	    end
	    
	    def dc_date=(date)
		if(date.class==Time)
		    @dc_date=date
		@syn_update_base=date
		end
	    end
	    
	    def syn_update_frequency=(i)
		if(i>0)
		    @syn_update_frequency=i
		end
	    end
	    
	    def push(x)
		unless(x.kind_of?(Item))
		    raise TypeError,"Invalid item pushed"
		end
		
		if(@seq.index(x.about).nil?)
		    @seq.push(x.about)
		    @items.push(x)
		end
	    end
	    
	    def each_item
		@items.each{|i|
		    yield i
		}
	    end
	    
	    def items
		@items.dup
	    end
	    
	    def update(channel)
		@lang=channel.lang
		@title=channel.title
		@description=channel.description
		@link=channel.link
		@image=channel.image
		@dc_language=channel.dc_language
		@dc_date=channel.dc_date
		@dc_subject=channel.dc_subject
		@syn_update_period=channel.syn_update_period
		@syn_update_frequency=channel.syn_update_frequency
		@syn_update_base=channel.syn_update_base
		
		@extra_info.update(channel.extra_info)
		
		index={}
		@items.each{|i|
		    index[i.about]=i
		}
		
		@seq.clear
		@items.clear
		channel.each_item{|i|
		    if(index.has_key?(i.about))
			self.push(index[i.about])
		    else
			self.push(i)
		    end
		}
		
		self
	    end
	    
	    def iconv(to,from='utf-8')
		(@title,@description,@dc_subject,
		 @dc_rights,@dc_publisher,@dc_creator)=
		    Iconv.iconv(to,from,@title,@description,@dc_subject,
				@dc_rights,@dc_publisher,@dc_creator)
		
		unless(@image.nil?)
		    @image=@image.iconv(to,from)
		end

		@items.each{|i|
		    i.iconv(to,from)
		}
		
		self
	    end
	    
	    def to_s(encode='utf-8')
		buf=''
		self.to_xml(encode).write(buf,0)
		
		buf
	    end
	    
	    def to_xml(encode='utf-8')
		xml=REXML::Document.new
		xml.add(REXML::XMLDecl.new('1.0',encode))
		xml.add(REXML::DocType.new('rdf:RDF',
					   "PUBLIC '-//W3C//ENTITIES Latin 1 for XHTML//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml-lat1.ent'")
			)
		
		rdf=REXML::Element.new('rdf:RDF')
		rdf.attributes['xml:lang']=@lang
		rdf.attributes['xmlns']='http://purl.org/rss/1.0/'
		rdf.attributes['xmlns:rdf']=
		    'http://www.w3.org/1999/02/22-rdf-syntax-ns#'
		rdf.attributes['xmlns:dc']=
		    'http://purl.org/dc/elements/1.1/'
		rdf.attributes['xmlns:syn']=
		    'http://purl.org/rss/1.0/modules/syndication/'
		
		ch=REXML::Element.new('channel')
		ch.attributes['rdf:about']=@about
		ch.add_element('title').text=@title
		ch.add_element('link').text=@link
		ch.add_element('description').text=@description
		ch.add_element('dc:language').text=@dc_language
		ch.add_element('dc:date').text=
		    @dc_date.utc.strftime("%Y-%m-%dT%H:%M:%S+00:00")
		unless(@dc_subject.to_s.empty?)
		    ch.add_element('dc:subject').text=@dc_subject
		end
		unless(@dc_rights.to_s.empty?)
		    ch.add_element('dc:rights').text=@dc_rights
		end
		unless(@dc_publisher.to_s.empty?)
		    ch.add_element('dc:publisher').text=@dc_publisher
		end
		unless(@dc_creator.to_s.empty?)
		    ch.add_element('dc:creator').text=@dc_creator
		end
		ch.add_element('syn:updatePeriod').text=@syn_update_period
		ch.add_element('syn:updateFrequency').text=
		    @syn_update_frequency.to_s
		ch.add_element('syn:updateBase').text=
		    @syn_update_base.utc.strftime("%Y-%m-%dT%H:%M:%S+00:00")
		
		unless(@image.nil?)
		    ch.add_element('image').attributes['rdf:resource']=
			@image.about
		end
		
		seq=ch.add_element('rdf:Seq')
		@seq.each{|s|
		    seq.add_element('rdf:li').attributes['rdf:resource']=s
		}
		ch.add_element('items').add_element(seq)
		
		rdf.add(ch)
		unless(@image.nil?)
		    rdf.add(@image.to_xml)
		end
		
		@items.each{|i|
		    rdf.add(i.to_xml)
		}
		
		xml.add(rdf)
		
		xml
	    end
	    
	    def Channel.parse(xml,encoding='utf-8')
		lang=xml.attributes['lang']
		ch=xml.elements.to_a('channel').first
		about=ch.attributes['about']
		title=ch.elements.to_a('title').first.text
		link=ch.elements.to_a('link').first.text
		description=ch.elements.to_a('description').first.text
		if(lang.nil?)
		    begin
			lang=ch.elements.to_a('language').first.text
		    rescue NameError
		    end
		    begin
			lang=ch.elements.to_a('dc:language').first.text
		    rescue NameError
		    end
		end
		if(lang.nil?)
		    lang='en'
		end
		
		ret=Channel.new(lang,about,title,link,description)
		
		dc_date=ch.elements.to_a('dc:date').first
		dc_subject=ch.elements.to_a('dc:subject').first
		dc_rights=ch.elements.to_a('dc:rights').first
		dc_publisher=ch.elements.to_a('dc:publisher').first
		dc_creator=ch.elements.to_a('dc:creator').first
		
		syn_update_period=ch.elements.to_a('syn:updatePeriod').first
		syn_update_frequency=
		    ch.elements.to_a('syn:updateFrequency').first
		syn_update_base=ch.elements.to_a('syn:updateBase').first
		
		unless(dc_date.nil?)
		    begin
			ret.dc_date=RSS.parsedate(dc_date.text)
		    rescue
		    end
		end
		unless(dc_subject.nil?)
		    ret.dc_subject=dc_subject.text
		end
		unless(dc_rights.nil?)
		    ret.dc_rights=dc_rights.text
		end
		unless(dc_publisher.nil?)
		    ret.dc_publisher=dc_publisher.text
		end
		unless(dc_creator.nil?)
		    ret.dc_creator=dc_creator.text
		end
		
		unless(syn_update_period.nil?)
		    ret.syn_update_period=syn_update_period.text
		end
		unless(syn_update_frequency.nil?)
		    ret.syn_update_frequency=syn_update_frequency.text.to_i
		end
		unless(syn_update_base.nil?)
		    begin
			ret.syn_update_base=RSS.parsedate(syn_update_base.text)
		    rescue
		    end
		end
		
		xml.elements.each('item'){|item|
		    ret.push(Item.parse(item,encoding))
		}
		
		image=xml.elements.to_a('image').first
		unless(image.nil?)
		    ret.image=Image.parse(image,encoding)
		end
		
		ret
	    end

            def Channel.parse09(xml,encoding='utf-8')
		lang=xml.attributes['lang']
		ch=xml.elements.to_a('channel').first
		title=ch.elements.to_a('title').first.text
		link=ch.elements.to_a('link').first.text
                about=link
		description=ch.elements.to_a('description').first.text
		if(lang.nil?)
		    begin
			lang=ch.elements.to_a('language').first.text
		    rescue NameError
		    end
		    begin
			lang=ch.elements.to_a('dc:language').first.text
		    rescue NameError
		    end
		end
		if(lang.nil?)
		    lang='en'
		end
		
		ret=Channel.new(lang,about,title,link,description)
		
		ch.elements.each('item'){|item|
		    ret.push(Item.parse09(item,encoding))
		}
		
		ret
	    end

	end
    end
end

