# opml.rb 
#  -- opml module for 24hours suit
#
# NISHI Takao <zophos@koka-in.org>
# $Id: opml.rb,v 1.1 2005-02-18 22:57:30 jon Exp $

require 'iconv'
require 'rexml/document'
require '24hconf.rb'

module TwentyFourHours
    module Opml
        def Opml.conv2conf(str)
            xml=REXML::Document.new(str)
            if(xml.root.name!='opml')
                raise
            end
            
            body=xml.root.elements.to_a('body').first
            cfg=Config.new
            cat=nil
            body.each_element{|el|
                Opml.parse_outline(el,cat,cfg)
            }

            cfg
        end

        def Opml.parse_outline(el,cat,cfg)
            if(el.attributes.has_key?('xmlUrl'))
                cfg.start_section('rss')
                cfg.set_item('link',el.attributes['xmlUrl'])
                if(el.attributes.has_key?('title'))
                    cfg.set_item('title',el.attributes['title'])
                end 
                if(el.attributes.has_key?('htmlUrl'))
                    cfg.set_item('target',el.attributes['htmlUrl'])
                end
                cfg.set_item('category',cat) unless cat.nil?
            elsif(el.has_elements?)
                if(cat.nil?)
                    cat=el.attributes['title']
                else
                    cat+=':'+el.attributes['title'].to_s
                end
                el.each_element{|e|
                    Opml.parse_outline(e,cat,cfg)
                }
            end

            cfg
        end
    end
end
