#!/usr/bin/ruby -w

$:.unshift( ".." )

require 'xml/xpath'

def node_type( t ) 
  print "Node type = "

  if t == XML::XPATH::XML_ELEMENT_NODE
	print "ELEMENT\n"
  elsif t == XML::XPATH::XML_ATTRIBUTE_NODE
	print "ATTRIBUTE\n"
  else
	print "UNKNOW\n"
  end
end

xpath = XML::XPATH.new()
xpath.xmlfile = "config.xml"

node_type( 1 );
node_type( 2 );
node_type( 3 );
  
print "Ruby/XPath v#{XML::XPATH::RUBY_XPATH_VERSION}\n"

x = "/config/class[@name='adresse']/@table"
puts x
i = xpath.execute( x )
print i, " nodes !\n"
p xpath.to_a
print "---------------------------------------------------\n"

x = "/config/class[@name='adresse']"
puts x
i = xpath.execute( x )
print i, " nodes !\n"
p xpath.to_a
print "---------------------------------------------------\n"

x = "/config/general/*"
print i, " nodes !\n"
p xpath.to_a
puts x
i = xpath.execute( x )
print "---------------------------------------------------\n"

x = "/config/ldap/bind"
puts x
i = xpath.execute( x )
print i, " nodes !\n"
p xpath.to_a
print "---------------------------------------------------\n"

x = "/config/ldap/bind/@dn" 
puts x
i = xpath.execute( x )
print i, " nodes !\n"
p xpath.to_a
print "---------------------------------------------------\n"

x = "/config/class" 
puts x
i = xpath.execute( x )
print i, " nodes !\n"
xpath.each { |n|
  p n
}
print "---------------------------------------------------\n"

ii = xpath.first
while( ii != 0 ) 
  print "node #{ii} : #{xpath.path}\n"
  ii = xpath.next
end

ii = xpath.last
while( ii != 0 ) 
  print "node #{ii} : #{xpath.path}\n"
  ii = xpath.prev
end
