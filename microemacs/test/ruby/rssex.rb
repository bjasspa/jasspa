require 'rss'
require 'uri'

include TwentyFourHours

module RSS
    class Channel
	CACHE_EXT='.24hdb'

        def ref_name
   	    sprintf("%s_%04x",
	    @extra_info['category'],
	    @about.to_s.sum(1024))
        end

	def Channel.cachefile(uri)
	    unless(uri.kind_of?(URI))
		uri=URI.parse(uri)
	    end
            file=uri.host+uri.path
            unless(uri.query.nil?)
                file+='?'+uri.query.to_s
                file.gsub!('?','%3f')
            end
	    file.gsub!('/','%2f')
	    file+=CACHE_EXT
	end
    end
end
